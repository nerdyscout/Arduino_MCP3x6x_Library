# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout

import json
import os
import re
import sys
import time

sys.path.insert(0, "test")
from _shared import _detect_scope_resource, _reset_board_via_dtr  # noqa: E402

DEFAULT_CONFIG = {
    "resource": "",
    "board_port": "",
    "channels": {"cs": "CHAN1", "sclk": "CHAN3", "miso": "CHAN2", "mosi": "CHAN4"},
    "probe": "1x",
    "volts_per_div": 1,
    "offset_volts": -2,
    "trigger": {"source": "CHAN1", "slope": "NEGative", "level_volts": 1.0},
    "timebase": {"scale_s": 0.002, "offset_s": 0.008},
    "acquire": {"type": "NORM", "mdepth": "AUTO"},
    "capture": {"probe_wait_s": 12},
}

EXPECTED = [
    ("reset()", [0x78]),
    ("conversion()", [0x68]),
    ("standby()", [0x6C]),
    ("shutdown()", [0x70]),
    ("full_shutdown()", [0x74]),
]
EXPECTED_FLAT = [b for _, bs in EXPECTED for b in bs]


def _deep_merge(base, override):
    out = dict(base)
    for k, v in (override or {}).items():
        if isinstance(v, dict) and isinstance(out.get(k), dict):
            out[k] = _deep_merge(out[k], v)
        else:
            out[k] = v
    return out


def load_config():
    candidates = [
        os.environ.get("HIL_SPI_CONFIG", ""),
        os.path.join("test", "hil_spi_config.json"),
        "hil_spi_config.json",
        os.path.join(os.getcwd(), "test", "hil_spi_config.json"),
    ]
    for p in candidates:
        if not p:
            continue
        try:
            if os.path.isfile(p):
                with open(p, "r", encoding="utf-8") as f:
                    return _deep_merge(DEFAULT_CONFIG, json.load(f))
        except Exception as exc:
            print(f"WARNING: failed to read config {p}: {exc}", file=sys.stderr)
    return dict(DEFAULT_CONFIG)


def _probe_value(probe):
    s = str(probe).strip().lower().replace("x", "")
    try:
        v = float(s)
        return str(int(v)) if v == int(v) else str(v)
    except Exception:
        return "1"


def _normalize_mosi(m):
    out = []
    for x in m or []:
        if isinstance(x, int):
            out.append(x)
        elif isinstance(x, str):
            try:
                out.append(int(x, 16) if x.lower().startswith("0x") else int(x))
            except Exception:
                pass
    return out


def apply_scope_settings(resource, cfg):
    try:
        import pyvisa
    except Exception as exc:
        print(f"WARNING: pyvisa unavailable, skipping scope setup ({exc})", file=sys.stderr)
        return

    ch = cfg.get("channels", {})
    ch_cs = str(ch.get("cs", "CHAN1"))
    ch_sclk = str(ch.get("sclk", "CHAN3"))
    ch_miso = str(ch.get("miso", "CHAN2"))
    ch_mosi = str(ch.get("mosi", "CHAN4"))
    probe = _probe_value(cfg.get("probe", "1x"))
    vdiv = float(cfg.get("volts_per_div", 1))
    offset = float(cfg.get("offset_volts", -2))
    trig = cfg.get("trigger", {})
    tb = cfg.get("timebase", {})
    acq = cfg.get("acquire", {})

    rm = pyvisa.ResourceManager()
    inst = rm.open_resource(resource)
    try:
        inst.timeout = 15000

        def w(c):
            inst.write(c)

        w(":STOP")
        for c in [ch_cs, ch_sclk, ch_miso, ch_mosi]:
            w(f":{c}:DISPlay ON")
            w(f":{c}:PROBe {probe}")
            w(f":{c}:SCALe {vdiv}")
            w(f":{c}:OFFSet {offset}")
        w(f":TIMebase:MAIN:SCALe {float(tb.get('scale_s', 0.002))}")
        w(f":TIMebase:MAIN:OFFSet {float(tb.get('offset_s', 0.008))}")
        w(f":ACQuire:TYPE {acq.get('type', 'NORM')}")
        w(f":ACQuire:MDEPth {acq.get('mdepth', 'AUTO')}")
        w(":TRIGger:MODE EDGE")
        w(f":TRIGger:EDGE:SOURce {trig.get('source', ch_cs)}")
        w(f":TRIGger:EDGE:SLOPe {trig.get('slope', 'NEGative')}")
        w(f":TRIGger:EDGE:LEVel {float(trig.get('level_volts', 1.0))}")
        w(":TRIGger:SWEep AUTO")
        w(":RUN")
        print("Scope settings applied.")
    finally:
        try:
            inst.close()
        except Exception:
            pass
        try:
            rm.close()
        except Exception:
            pass


def read_sequence(board_port, timeout_s=20, debug=False):
    import serial as pyserial

    ser = pyserial.Serial(board_port, 115200, timeout=1)
    try:
        deadline = time.time() + timeout_s
        lines = []
        while time.time() < deadline:
            raw = ser.readline().decode("ascii", errors="replace").strip()
            if not raw:
                continue
            lines.append(raw)
            if debug:
                print(f"  SERIAL: {raw}")
            if raw.startswith("{") and "\"mosi\"" in raw:
                try:
                    obj = json.loads(raw)
                    if isinstance(obj.get("mosi"), list):
                        return _normalize_mosi(obj["mosi"]), lines
                except Exception:
                    pass
        return None, lines
    finally:
        try:
            ser.close()
        except Exception:
            pass



def _run_decoder_compare(decoder_path, expected, cfg, debug=False):
    if not decoder_path or not os.path.isfile(decoder_path):
        return None, "decoder not found"
    import subprocess
    channels = cfg.get("channels", {})
    ch = {
        "cs": str(channels.get("cs", "CHAN1")).replace("CHAN", ""),
        "sclk": str(channels.get("sclk", "CHAN3")).replace("CHAN", ""),
        "mosi": str(channels.get("mosi", "CHAN4")).replace("CHAN", ""),
        "miso": str(channels.get("miso", "CHAN2")).replace("CHAN", ""),
    }
    base = [sys.executable, decoder_path]
    try:
        # Best-effort configure/readback (interface varies by decoder implementation)
        cfg_cmd = base + ["--mode", "configure"]
        rb_cmd = base + ["--mode", "readback"]
        if debug:
            print(f"  decoder configure: {' '.join(cfg_cmd)}")
        subprocess.run(cfg_cmd, check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if debug:
            print(f"  decoder readback: {' '.join(rb_cmd)}")
        r = subprocess.run(rb_cmd, check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        out = (r.stdout or "") + "\n" + (r.stderr or "")
        # try parse JSON array/obj from output
        m = re.search(r"\{.*\"mosi\".*\}", out, re.S)
        if m:
            try:
                obj = json.loads(m.group(0))
                got = _normalize_mosi(obj.get("mosi"))
                if got:
                    return (got == expected), f"decoder readback mosi={got}"
            except Exception:
                pass
        m = re.search(r"\[[^\]]+\]", out)
        if m:
            try:
                arr = json.loads(m.group(0))
                if isinstance(arr, list):
                    got = _normalize_mosi(arr)
                    if got:
                        return (got == expected), f"decoder readback bytes={got}"
            except Exception:
                pass
        return None, f"decoder readback not parseable (rc={r.returncode})"
    except Exception as exc:
        return None, f"decoder error: {exc}"

def verify(build_env="hil_spi", do_reset=False, debug=False, repeat=1):
    cfg = load_config()
    if cfg.get("board_port"):
        os.environ.setdefault("BOARD_PORT", str(cfg["board_port"]))
    if cfg.get("resource"):
        os.environ.setdefault("OSCI_RESOURCE", str(cfg["resource"]))

    resource = _detect_scope_resource(debug=debug)
    if resource is None:
        return 1
    print(f"Scope resource: {resource}")
    apply_scope_settings(resource, cfg)

    if do_reset:
        _reset_board_via_dtr(debug=debug)

    skip_probe = build_env == "hil_spi" or os.environ.get("SKIP_PIN_PROBE") == "1"
    probe_wait = 0.0 if skip_probe else float(cfg.get("capture", {}).get("probe_wait_s", 12))
    if probe_wait > 0:
        print(f"Waiting {probe_wait:.1f}s for pin-probe to complete ...")
        time.sleep(probe_wait)
    else:
        time.sleep(1.0)

    board_port = os.environ.get("BOARD_PORT") or cfg.get("board_port") or "/dev/ttyUSB0"
    print(f"Reading SPI sequence JSON from serial ({board_port}) ...")
    try:
        found, lines = read_sequence(board_port, timeout_s=25, debug=debug)
    except Exception as exc:
        print(f"ERROR: serial read failed: {exc}", file=sys.stderr)
        return 1

    if found is None:
        print(f"ERROR: no SPI sequence JSON received (got {len(lines)} lines)", file=sys.stderr)
        for l in lines[-10:]:
            print(f"  last: {l}", file=sys.stderr)
        return 1

    print(f"Received MOSI sequence: {found}")
    if found != EXPECTED_FLAT:
        print(f"ERROR: MOSI mismatch: expected {EXPECTED_FLAT}, got {found}", file=sys.stderr)
        return 1

    idx = 0
    for label, exp in EXPECTED:
        got = found[idx:idx + len(exp)]
        idx += len(exp)
        print(f"  OK {label}: {got}")

    decoder_path = os.environ.get("DECODER_PATH", "")
    dec_ok, dec_msg = _run_decoder_compare(decoder_path, EXPECTED_FLAT, cfg, debug=debug)
    if dec_ok is True:
        print(f"Decoder readback gate: PASS ({dec_msg})")
    elif dec_ok is False:
        print(f"ERROR: decoder readback gate failed: {dec_msg}", file=sys.stderr)
        return 1
    elif decoder_path:
        print(f"NOTE: decoder readback gate skipped/unavailable: {dec_msg}")

    print("=== SPI verification passed. ===")
    return 0
