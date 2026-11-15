#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout
"""HIL SPI verification: host-paced capture + sigrok-cli decode (datasheet gate)."""

from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import time
import traceback
from pathlib import Path
from typing import Any

sys.path.insert(0, "test")
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import serial  # type: ignore

from _shared import _detect_scope_resource, _reset_board_via_dtr  # noqa: E402

DEFAULT_CONFIG = {
    "resource": "",
    "board_port": "/dev/ttyUSB0",
    "channels": {"cs": "CHAN1", "sclk": "CHAN3", "miso": "CHAN2", "mosi": "CHAN4"},
    "trigger": {"source": "CHAN1", "slope": "NEGative", "level_volts": 2.0},
    "timebase": {"scale_s": 0.000002, "offset_s": 0.000008},
    "acquire": {"type": "NORM", "mdepth": "12000000"},
    "volts_per_div": 2.0,
    "threshold_volts": 1.6,
}

EXPECTED = [
    ("reset", 0x78, 0x17),
    ("conversion", 0x68, 0x17),
    ("standby", 0x6C, 0x17),
    ("shutdown", 0x70, 0x17),
    ("full_shutdown", 0x74, 0x17),
]


def _deep_merge(base: dict[str, Any], override: dict[str, Any]) -> dict[str, Any]:
    out = dict(base)
    for k, v in (override or {}).items():
        if isinstance(v, dict) and isinstance(out.get(k), dict):
            out[k] = _deep_merge(out[k], v)
        else:
            out[k] = v
    return out


def load_config() -> dict[str, Any]:
    cfg = _deep_merge({}, DEFAULT_CONFIG)
    for p in [os.environ.get("HIL_SPI_CONFIG", ""), os.path.join("test", "hil_spi_config.json"), "hil_spi_config.json"]:
        if p and os.path.isfile(p):
            try:
                with open(p, "r", encoding="utf-8") as f:
                    cfg = _deep_merge(cfg, json.load(f))
                break
            except Exception as exc:
                print(f"WARNING: failed to read config {p}: {exc}", file=sys.stderr)
    return cfg


def _tmc_block(raw: bytes) -> bytes:
    if raw[:1] == b"#" and len(raw) > 2 and raw[1:2].isdigit():
        nd = int(raw[1:2])
        n = int(raw[2:2 + nd] or b"0")
        return raw[2 + nd:2 + nd + n]
    return raw or b""


def _wait_status(inst: Any, want: set[str], timeout_s: float) -> str:
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        st = str(inst.query(":TRIGger:STATus?")).strip()
        if st in want or any(w in st.upper() for w in want):
            return st
        time.sleep(0.03)
    return str(inst.query(":TRIGger:STATus?")).strip()


def _configure_scope(inst: Any, cfg: dict[str, Any]) -> None:
    ch = cfg["channels"]
    trig = cfg.get("trigger", {})
    tb = cfg.get("timebase", {})
    acq = cfg.get("acquire", {})
    vdiv = cfg.get("volts_per_div", 2.0)
    inst.timeout = 6000
    inst.write(":STOP")
    for c in [ch["cs"], ch["sclk"], ch["mosi"], ch["miso"]]:
        inst.write(f":{c}:DISPlay ON")
        inst.write(f":{c}:PROBe 1")
        inst.write(f":{c}:SCALe {vdiv}")
        inst.write(f":{c}:OFFSet 0")
        inst.write(f":{c}:COUPling DC")
    inst.write(f":TIMebase:MAIN:SCALe {tb.get('scale_s', 0.000002)}")
    inst.write(f":TIMebase:MAIN:OFFSet {tb.get('offset_s', 0.000008)}")
    inst.write(f":ACQuire:TYPE {acq.get('type', 'NORM')}")
    inst.write(f":ACQuire:MDEPth {acq.get('mdepth', '12000000')}")
    inst.write(":TRIGger:MODE EDGE")
    inst.write(f":TRIGger:EDGE:SOURce {trig.get('source', ch['cs'])}")
    inst.write(f":TRIGger:EDGE:SLOPe {trig.get('slope', 'NEGative')}")
    inst.write(f":TRIGger:EDGE:LEVel {trig.get('level_volts', 2.0)}")
    inst.query("*OPC?")


def _read_wave(inst: Any, ch: str) -> tuple[list[float], float]:
    inst.timeout = 5000
    inst.write(f":WAVeform:SOURce {ch}")
    inst.write(":WAVeform:MODE NORM")
    inst.write(":WAVeform:FORMat BYTE")
    pre = str(inst.query(":WAVeform:PREamble?")).strip().split(",")
    if len(pre) < 10:
        raise RuntimeError(f"bad preamble for {ch}: {pre!r}")
    pts = int(float(pre[2]))
    xinc = float(pre[4])
    yinc = float(pre[7])
    yorg = float(pre[8])
    yref = float(pre[9])
    data: list[int] = []
    start = 1
    while start <= pts:
        stop = min(pts, start + 399)
        inst.write(f":WAVeform:STARt {start}")
        inst.write(f":WAVeform:STOP {stop}")
        inst.write(":WAVeform:DATA?")
        raw = inst.read_raw() or b""
        part = list(_tmc_block(raw))
        if len(part) != (stop - start + 1):
            raise RuntimeError(f"{ch} chunk {start}-{stop} got {len(part)} bytes")
        data.extend(part)
        start = stop + 1
    return [(d - yorg - yref) * yinc for d in data], xinc


def _bits(vals: list[float], thr: float) -> list[int]:
    return [1 if v >= thr else 0 for v in vals]


def _write_vcd(path: str, clk: list[int], mosi: list[int], miso: list[int], cs: list[int], xinc: float) -> None:
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    scale = max(1, int(round(xinc / 1e-9)))
    with open(path, "w", encoding="utf-8") as f:
        f.write(f"$timescale {scale} ns $end\n$scope module logic $end\n$var wire 1 ! clk $end\n$var wire 1 \" mosi $end\n$var wire 1 # miso $end\n$var wire 1 $ cs $end\n$upscope $end\n$enddefinitions $end\n#0\n")
        prev = [clk[0], mosi[0], miso[0], cs[0]]
        syms = ["!", '"', "#", "$"]
        for s, v in zip(syms, prev):
            f.write(f"{v}{s}\n")
        for i in range(1, len(clk)):
            cur = [clk[i], mosi[i], miso[i], cs[i]]
            if cur == prev:
                continue
            f.write(f"#{i}\n")
            for j, s in enumerate(syms):
                if cur[j] != prev[j]:
                    f.write(f"{cur[j]}{s}\n")
            prev = cur


def _sigrok_decode(vcd: str, debug: bool = False) -> tuple[list[int], list[int]]:
    env = os.environ.copy()
    decdir = Path.cwd() / "scripts" / "decoders"
    if decdir.is_dir():
        env["SIGROKDECODE_DIR"] = str(decdir)
    base = [
        "timeout", "8", "sigrok-cli", "-I", "vcd", "-i", vcd,
        "-P", "spi:clk=clk:mosi=mosi:miso=miso:cs=cs:cpol=0:cpha=0:bitorder=msb-first:wordsize=8:cs_polarity=active-low",
    ]

    def get(ann: str) -> list[int]:
        cmd = base + ["-A", f"spi={ann}"]
        p = subprocess.run(cmd, capture_output=True, text=True, env=env)
        if p.returncode != 0:
            raise RuntimeError(f"sigrok-cli {ann} failed (rc={p.returncode}): {(p.stderr or p.stdout).strip()[:300]}")
        out: list[int] = []
        for line in p.stdout.splitlines():
            m = re.search(r"\b([0-9A-Fa-f]{2})\b", line.split(":", 1)[-1])
            if m:
                out.append(int(m.group(1), 16))
        return out

    mosi = get("mosi-data")
    miso = get("miso-data")
    if debug:
        print(f"[debug] sigrok mosi={mosi} miso={miso}", file=sys.stderr)
    return mosi, miso


def _step_once(idx: int, ser: Any, rm: Any, resource: str, cfg: dict[str, Any], debug: bool) -> tuple[bool, str]:
    ch = cfg["channels"]
    thr = float(cfg.get("threshold_volts", 1.6))
    inst = rm.open_resource(resource)
    inst.timeout = 6000
    try:
        try:
            inst.clear()
        except Exception:
            pass
        _configure_scope(inst, cfg)
        inst.write(":STOP")
        try:
            inst.write(":CLEar")
        except Exception:
            pass
        inst.write(":TRIGger:SWEep SINGle")
        inst.write(":SINGle")
        inst.query("*OPC?")
        rdy = _wait_status(inst, {"WAIT"}, 2.5)
        try:
            ser.reset_input_buffer()
        except Exception:
            pass
        ser.write(b"n")
        ser.flush()
        label = None
        ser_mosi = None
        t_end = time.time() + 4.0
        while time.time() < t_end:
            line = ser.readline().decode("utf-8", "ignore").strip()
            if not line:
                continue
            if line.startswith("{"):
                try:
                    obj = json.loads(line)
                    label = obj.get("name")
                    ser_mosi = obj.get("mosi")
                except Exception:
                    pass
            if line == "SCOPE_END":
                break
        st = _wait_status(inst, {"STOP", "TD"}, 3.0)
        inst.write(":STOP")
        time.sleep(0.12)
        inst.query("*OPC?")
        if not label:
            raise RuntimeError(f"no step label (rdy={rdy} stop={st})")
        waves: dict[str, list[float]] = {}
        xinc = 0.0
        for name, chan in [("cs", ch["cs"]), ("sclk", ch["sclk"]), ("mosi", ch["mosi"]), ("miso", ch["miso"])]:
            waves[name], xinc = _read_wave(inst, chan)
        logs = Path.cwd() / "logs" / "hil_spi"
        logs.mkdir(parents=True, exist_ok=True)
        vcd = str(logs / f"step_{idx}_{label}.vcd")
        _write_vcd(vcd, _bits(waves["sclk"], thr), _bits(waves["mosi"], thr), _bits(waves["miso"], thr), _bits(waves["cs"], thr), xinc)
        mosi, miso = _sigrok_decode(vcd, debug=debug)
        exp = next(((n, em, ei) for n, em, ei in EXPECTED if n == label), None)
        if not exp:
            raise RuntimeError(f"unexpected label {label}")
        ok = (mosi == [exp[1]] and miso == [exp[2]])
        print(f"[{idx}] {'OK' if ok else 'BAD'} {label} ser={ser_mosi} mosi={[hex(x) for x in mosi]} miso={[hex(x) for x in miso]} exp={[hex(exp[1]),hex(exp[2])]}")
        if not ok and not os.environ.get("HIL_SPI_KEEP_ARTIFACTS"):
            try:
                os.remove(vcd)
            except Exception:
                pass
        return ok, str(label)
    finally:
        try:
            inst.close()
        except Exception:
            pass


def verify(build_env: str = "hil_spi", do_reset: bool = True, debug: bool = False, repeat: int = 1) -> int:
    cfg = load_config()
    board_port = os.environ.get("BOARD_PORT") or cfg.get("board_port") or "/dev/ttyUSB0"
    os.environ["BOARD_PORT"] = str(board_port)

    try:
        import pyvisa  # type: ignore
    except Exception as e:
        print(f"ERROR: PyVISA not available: {e}", file=sys.stderr)
        return 1

    if subprocess.run(["timeout", "2", "sigrok-cli", "--version"], capture_output=True, text=True).returncode != 0:
        print("ERROR: sigrok-cli not available", file=sys.stderr)
        return 1

    resource = cfg.get("resource") or os.environ.get("OSCI_RESOURCE") or _detect_scope_resource(debug=debug)
    if not resource:
        return 1
    print(f"Scope resource: {resource}")
    print(f"Board port: {board_port}")

    total = len(EXPECTED) * max(1, repeat)
    passed = 0
    rm = pyvisa.ResourceManager()
    ser = serial.Serial(board_port, 115200, timeout=0.4)
    try:
        if do_reset:
            _reset_board_via_dtr(debug=debug)
            time.sleep(2.5)
        try:
            ser.reset_input_buffer()
        except Exception:
            pass
        for i in range(1, total + 1):
            try:
                ok, _ = _step_once(i, ser, rm, resource, cfg, debug)
                if ok:
                    passed += 1
            except Exception as e:
                print(f"[{i}] FAIL {e}", file=sys.stderr)
                if debug:
                    traceback.print_exc(limit=2)
            time.sleep(0.2)
        print(f"Sigrok gate: {passed}/{total} steps matched datasheet")
        if passed == total:
            print("=== SPI verification passed (sigrok-cli, datasheet). ===")
            return 0
        return 1
    finally:
        try:
            ser.close()
        except Exception:
            pass
        try:
            rm.close()
        except Exception:
            pass
