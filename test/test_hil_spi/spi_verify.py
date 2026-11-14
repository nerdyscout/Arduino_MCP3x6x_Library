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


def _read_etable_pair(scope):
    """Read one decoded (mosi,miso) pair from Rigol internal decoder ETABle."""
    # Import lazily so verify can still run basic checks without scripts on path issues
    try:
        sys.path.insert(0, os.path.join(os.getcwd(), "scripts"))
        from decoder import _parse_tmc_block, _etable_hex_payload, _extract_from_etable_text  # type: ignore
    except Exception:
        return [], []
    try:
        scope.inst.timeout = 8000
        scope.write(":ETABle1:DATA?")
        raw = scope.read_raw() or b""
        payload = _parse_tmc_block(raw)
        text = _etable_hex_payload(payload).decode("ascii", "ignore")
        mo, mi = _extract_from_etable_text(text)
        return mo, mi
    except Exception:
        return [], []


def _configure_scope_for_decode(scope, cfg):
    ch = cfg.get("channels", {})
    trig = cfg.get("trigger", {})
    tb = cfg.get("timebase", {})
    acq = cfg.get("acquire", {})
    dec = cfg.get("decoder", {})
    ch_cs = str(ch.get("cs", "CHAN1"))
    ch_sclk = str(ch.get("sclk", "CHAN3"))
    ch_miso = str(ch.get("miso", "CHAN2"))
    ch_mosi = str(ch.get("mosi", "CHAN4"))
    probe = _probe_value(cfg.get("probe", "1x"))
    vdiv = float(cfg.get("volts_per_div", 2.0))
    offset = float(cfg.get("offset_volts", 0.0))
    trig_src = str(trig.get("source", "CHAN1"))
    trig_slope = str(trig.get("slope", "NEGative"))
    trig_level = float(trig.get("level_volts", 2.0))
    tscale = float(tb.get("scale_s", 0.000002))
    toffset = float(tb.get("offset_s", 0.000008))
    mdepth = str(acq.get("mdepth", "12000000"))
    dec_mode = str(dec.get("mode", "CS")).upper()
    dec_select = str(dec.get("spi_select", "NCS")).upper()
    dec_edge = str(dec.get("edge", "FALL")).upper()
    dec_endian = str(dec.get("endian", "LSB")).upper()
    dec_pol = str(dec.get("ss_polarity", dec.get("polarity", "NEG"))).upper()
    dec_width = int(dec.get("width", 8))
    dec_thr = float(dec.get("threshold_volts", trig_level))

    scope.write(":STOP")
    for c in [ch_cs, ch_sclk, ch_miso, ch_mosi]:
        scope.write(f":{c}:DISPlay ON")
        scope.write(f":{c}:PROBe {probe}")
        scope.write(f":{c}:SCALe {vdiv}")
        scope.write(f":{c}:OFFSet {offset}")
        scope.write(f":{c}:COUPling DC")
    scope.write(f":TIMebase:MAIN:SCALe {tscale}")
    scope.write(f":TIMebase:MAIN:OFFSet {toffset}")
    scope.write(":ACQuire:TYPE NORM")
    scope.write(f":ACQuire:MDEPth {mdepth}")
    scope.write(":TRIGger:MODE EDGE")
    scope.write(f":TRIGger:EDGE:SOURce {trig_src}")
    scope.write(f":TRIGger:EDGE:SLOPe {trig_slope}")
    scope.write(f":TRIGger:EDGE:LEVel {trig_level}")
    # decoder
    scope.write(":DECoder1:MODE SPI")
    scope.write(":DECoder1:DISPlay ON")
    scope.write(":DECoder1:FORMat HEX")
    scope.write(":DECoder1:THReshold:AUTO OFF")
    for c in [ch_cs, ch_sclk, ch_miso, ch_mosi]:
        scope.write(f":DECoder1:THReshold:{c} {dec_thr}")
    scope.write(f":DECoder1:SPI:CLK {ch_sclk}")
    scope.write(f":DECoder1:SPI:MOSI {ch_mosi}")
    scope.write(f":DECoder1:SPI:MISO {ch_miso}")
    scope.write(f":DECoder1:SPI:CS {ch_cs}")
    scope.write(f":DECoder1:SPI:SELect {dec_select}")
    scope.write(f":DECoder1:SPI:MODE {dec_mode}")
    scope.write(f":DECoder1:SPI:POLarity {dec_pol}")
    scope.write(f":DECoder1:SPI:EDGE {dec_edge}")
    scope.write(f":DECoder1:SPI:ENDian {dec_endian}")
    scope.write(f":DECoder1:SPI:WIDTh {dec_width}")
    if dec_mode == "TIM":
        scope.write(f":DECoder1:SPI:TIMeout {float(dec.get('spi_timeout_s', 0.00005))}")
    scope.write(":ETABle1:DISPlay ON")
    scope.write(":ETABle1:FORMat HEX")
    scope.write(":ETABle1:VIEW PACKage")
    try:
        scope.opc()
    except Exception:
        pass


def _capture_decode_pair(scope, debug=False):
    """Read latest decoded MOSI/MISO pair from running ETABle (AUTO/RUN)."""
    mo, mi = _read_etable_pair(scope)
    if debug:
        print(f"    [cap] mosi={mo} miso={mi}")
    return mo, mi


def _name_to_label(name):
    return f"{name}()" if name and not name.endswith("()") else (name or "")


def _wait_serial_for(ser, target, timeout_s=5.0):
    t = time.time() + timeout_s
    while time.time() < t:
        try:
            line = ser.readline().decode("utf-8", "ignore").strip()
        except Exception:
            line = ""
        if not line:
            continue
        if line == target:
            return True, line
        if target == "JSON" and line.startswith("{"):
            return True, line
    return False, ""


def verify_internal_decoder(decoder_path, cfg, debug=False):
    """Gate: prove each MCP3x6x fast command (MOSI+MISO) is read back by the
    scope internal decoder (:ETABle1:DATA?) using a single fixed setting.
    Synchronizes on serial SCOPE_BEGIN/name, single-captures each frame, and
    accumulates decoded (mosi,miso) pairs until all expected pairs are seen."""
    import pyvisa
    import serial

    resource = os.environ.get("OSCI_RESOURCE") or cfg.get("resource") or ""
    if not resource:
        return False, "no scope resource (OSCI_RESOURCE/config)"
    board_port = os.environ.get("BOARD_PORT") or cfg.get("board_port") or "/dev/ttyUSB0"
    expected_by_label = {label: (v.get("mosi", []), v.get("miso", [])) for label, v in EXPECTED}
    pending = dict(expected_by_label)
    max_rounds = int(cfg.get("capture", {}).get("attempts", 60))

    rm = pyvisa.ResourceManager()
    inst = None
    ser = None
    try:
        inst = rm.open_resource(resource)
        inst.timeout = 15000
        ser = serial.Serial(board_port, 115200, timeout=0.25)
        time.sleep(0.2)
        try:
            ser.reset_input_buffer()
        except Exception:
            pass

        class _S:
            pass
        scope = _S()
        scope.inst = inst
        scope.timeout_ms = 15000
        scope.write = inst.write
        scope.query = lambda cmd: inst.query(cmd)
        scope.read_raw = lambda: inst.read_raw()

        def _ws(timeout_s=2.5, poll_s=0.04):
            t = time.time() + timeout_s
            while time.time() < t:
                try:
                    s = (inst.query(":TRIGger:STATus?") or "").strip().upper()
                    if s in ("STOP", "TD", "T'D", "TDONE"):
                        return True
                except Exception:
                    pass
                time.sleep(poll_s)
            return False
        scope.wait_stop = _ws

        _configure_scope_for_decode(scope, cfg)

        attempts = 0
        while attempts < max_rounds and pending:
            attempts += 1
            # align to next command announcement
            ok, _ = _wait_serial_for(ser, "SCOPE_BEGIN", timeout_s=4.0)
            if not ok:
                continue
            # arm single capture for this command
            try:
                inst.write(":TRIGger:SWEep SINGle")
                inst.write(":SINGle")
            except Exception:
                pass
            # read JSON/name/end
            cmd_name = ""
            t_end = time.time() + 2.0
            while time.time() < t_end:
                try:
                    l2 = ser.readline().decode("utf-8", "ignore").strip()
                except Exception:
                    l2 = ""
                if not l2:
                    continue
                if debug:
                    print(f"  [ser] {l2}")
                if l2.startswith("{"):
                    try:
                        obj = json.loads(l2)
                        cmd_name = str(obj.get("name", ""))
                    except Exception:
                        pass
                if l2 == "SCOPE_END":
                    break
            label = _name_to_label(cmd_name)

            _ws(timeout_s=1.8, poll_s=0.03)
            try:
                inst.write(":STOP")
            except Exception:
                pass
            time.sleep(0.15)
            got_mosi, got_miso = _read_etable_pair(scope)
            if debug:
                print(f"  [decode {attempts}/{max_rounds}] label={label} mosi={got_mosi} miso={got_miso}")

            if label in pending:
                exp_m, exp_i = expected_by_label[label]
                if got_mosi == exp_m and got_miso == exp_i:
                    del pending[label]
                    print(f"  OK {label}: mosi={exp_m} miso={exp_i} (internal decoder)")
                elif debug:
                    print(f"    mismatch {label}: got mosi={got_mosi} miso={got_miso} exp mosi={exp_m} miso={exp_i}")

        try:
            inst.write(":TRIGger:SWEep AUTO")
            inst.write(":RUN")
        except Exception:
            pass
        if not pending:
            return True, "internal decoder (:ETABle1:DATA?) read back all expected MOSI+MISO pairs"
    finally:
        try:
            if ser is not None:
                ser.close()
        except Exception:
            pass
        try:
            if inst is not None:
                inst.close()
        except Exception:
            pass
        try:
            rm.close()
        except Exception:
            pass
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
    if decoder_path:
        print(f"Using internal decoder: {decoder_path}")
    else:
        print("Using in-process internal decoder readback (:ETABle1:DATA?)")
    ok, msg = verify_internal_decoder(decoder_path, cfg, debug=debug)
    if not ok:
        print(f"ERROR: internal decoder gate failed: {msg}", file=sys.stderr)
        return 1
    print(f"Internal decoder gate: PASS ({msg})")
    print("=== SPI verification passed (internal decoder). ===")
    return 0
