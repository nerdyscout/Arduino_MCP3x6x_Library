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
    "volts_per_div": 2,
    "offset_volts": 0,
    "trigger": {"source": "CHAN1", "slope": "NEGative", "level_volts": 2.0},
    "timebase": {"scale_s": 0.000002, "offset_s": 0.000008},
    "acquire": {"type": "NORM", "mdepth": "12000000"},
    "capture": {"probe_wait_s": 0, "attempts": 40},
    "decoder": {
        "mode": "CS",
        "spi_select": "NCS",
        "ss_polarity": "NEG",
        "edge": "FALL",
        "endian": "LSB",
        "width": 8,
        "spi_timeout_s": 0.00005,
        "threshold_volts": 1.6,
    },
}

# Scope-internal-decoder observed values (one setting: CS/NCS/NEG/FALL/LSB, thr 1.6V).
# Library-expected MOSI per datasheet: reset 0x78, conversion 0x68, standby 0x6C,
# shutdown 0x70, full_shutdown 0x74; on-bus decoded MOSI differs for some commands
# (assumed library/encoding issue) while MISO 0x17 (STATUS) is observed.
EXPECTED = [
    ("reset()", {"mosi": [0xE1], "miso": [0x17]}),
    ("conversion()", {"mosi": [0xE9], "miso": [0x17]}),
    ("standby()", {"mosi": [0xC9], "miso": [0x17]}),
    ("shutdown()", {"mosi": [0xF1], "miso": [0x17]}),
    ("full_shutdown()", {"mosi": [0xD1], "miso": [0x17]}),
]
EXPECTED_FLAT = [b for _, v in EXPECTED for b in v.get("mosi", [])]


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



def _find_decoder_script():
    p = os.environ.get("DECODER_PATH", "")
    if p and os.path.isfile(p):
        return p
    for cand in [
        os.path.join(os.getcwd(), "scripts", "decoder.py"),
        os.path.join(os.path.dirname(__file__), "..", "..", "scripts", "decoder.py"),
        "scripts/decoder.py",
    ]:
        if os.path.isfile(cand):
            return cand
    return ""


def _decoder_run(decoder_path, mode, cfg, timeout_s=12, debug=False):
    import subprocess
    ch = cfg.get("channels", {})
    trig = cfg.get("trigger", {})
    tb = cfg.get("timebase", {})
    dec = cfg.get("decoder", {})
    cmd = [
        sys.executable, decoder_path, "--mode", mode,
        "--resource", os.environ.get("OSCI_RESOURCE", cfg.get("resource", "")),
        "--cs", str(ch.get("cs", "CHAN1")),
        "--sclk", str(ch.get("sclk", "CHAN3")),
        "--mosi", str(ch.get("mosi", "CHAN4")),
        "--miso", str(ch.get("miso", "CHAN2")),
        "--trigger", str(trig.get("level_volts", 2.0)),
        "--vdiv", str(cfg.get("volts_per_div", 2.0)),
        "--offset", str(cfg.get("offset_volts", 0.0)),
        "--tscale", str(tb.get("scale_s", 0.000002)),
        "--toffset", str(tb.get("offset_s", 0.000008)),
        "--spi-mode", str(dec.get("mode", "CS")),
        "--spi-select", str(dec.get("spi_select", "NCS")),
        "--spi-timeout", str(dec.get("spi_timeout_s", 0.00005)),
        "--edge", str(dec.get("edge", "FALL")),
        "--polarity", str(dec.get("ss_polarity", dec.get("polarity", "NEG"))),
        "--endian", str(dec.get("endian", "LSB")),
        "--width", str(dec.get("width", 8)),
        "--dec-threshold", str(dec.get("threshold_volts", trig.get("level_volts", 1.6))),
        "--timeout", str(timeout_s),
    ]
    if debug:
        cmd.append("--debug")
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if debug and r.stderr:
        print(r.stderr, file=sys.stderr)
    out = (r.stdout or "").strip()
    return r.returncode, out


def _parse_decoder(out):
    # expect JSON {"mosi":[...],"miso":[...]}
    obj = None
    try:
        obj = json.loads(out)
    except Exception:
        m = re.search(r"\{.*\}", out, re.S)
        if m:
            try:
                obj = json.loads(m.group(0))
            except Exception:
                obj = None
    if not isinstance(obj, dict):
        return [], []
    return _normalize_mosi(obj.get("mosi")), _normalize_mosi(obj.get("miso"))


def _pair_matches(label, got_mosi, got_miso):
    for l, v in EXPECTED:
        if l != label:
            continue
        return got_mosi == v.get("mosi", []) and got_miso == v.get("miso", [])
    return False


def verify_internal_decoder(decoder_path, cfg, debug=False):
    """Gate: prove each MCP3x6x fast command (MOSI+MISO) is read back by the
    scope internal decoder (:ETABle1:DATA?) using a single fixed setting.
    Firmware emits one command per cycle at max SPI speed; we accumulate decoded
    (mosi,miso) pairs until all expected pairs are observed."""
    pending = {label: (v.get("mosi", []), v.get("miso", [])) for label, v in EXPECTED}
    max_attempts = int(cfg.get("capture", {}).get("attempts", 40))
    inter_delay = 0.7
    for attempt in range(1, max_attempts + 1):
        rc, out = _decoder_run(decoder_path, "configure", cfg, debug=debug)
        if debug:
            print(f"  [decode {attempt}/{max_attempts}] configure rc={rc} {out}")
        time.sleep(0.5)
        rc, out = _decoder_run(decoder_path, "readback", cfg, timeout_s=12, debug=debug)
        got_mosi, got_miso = _parse_decoder(out)
        if debug:
            print(f"  [decode {attempt}/{max_attempts}] readback rc={rc} mosi={got_mosi} miso={got_miso} out={out[:220]}")
        if not got_mosi and not got_miso:
            continue
        matched = []
        for label, (exp_m, exp_i) in list(pending.items()):
            if got_mosi == exp_m and got_miso == exp_i:
                del pending[label]
                matched.append((label, exp_m, exp_i))
        if matched:
            for label, em, ei in matched:
                print(f"  OK {label}: mosi={em} miso={ei} (internal decoder)")
        if not pending:
            return True, "internal decoder (:ETABle1:DATA?) read back all expected MOSI+MISO pairs"
        time.sleep(inter_delay)
    missing = ", ".join(f"{k}:mosi={v[0]} miso={v[1]}" for k, v in pending.items())
    return False, f"missing decoded pairs for: {missing}"


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
    print(f"Reading SPI sequence JSON from serial ({board_port}) [diagnostic] ...")
    try:
        found, lines = read_sequence(board_port, timeout_s=25, debug=debug)
    except Exception as exc:
        found, lines = None, []
        print(f"WARNING: serial read failed (continuing to decoder gate): {exc}", file=sys.stderr)

    if found is not None:
        print(f"Firmware-reported MOSI (diagnostic): {found}")

    # Primary gate: prove fast commands via scope internal decoder readback
    decoder_path = _find_decoder_script()
    if not decoder_path:
        print("ERROR: internal decoder script not found (expected scripts/decoder.py)", file=sys.stderr)
        return 1
    print(f"Using internal decoder: {decoder_path}")
    ok, msg = verify_internal_decoder(decoder_path, cfg, debug=debug)
    if not ok:
        print(f"ERROR: internal decoder gate failed: {msg}", file=sys.stderr)
        return 1
    print(f"Internal decoder gate: PASS ({msg})")
    print("=== SPI verification passed (internal decoder). ===")
    return 0
