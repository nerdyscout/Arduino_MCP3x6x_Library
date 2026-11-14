#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Rigol DS1000Z (e.g. DS1104Z) SPI decoder readback for HIL verification.

Reads true SPI-bus-decoded MOSI bytes using the oscilloscope's internal
protocol decoder via SCPI (SCPI-1999 / Rigol DS1000Z programming interface).

Modes:
  configure : apply analog+decoder config and arm single capture
  readback  : wait for capture, query internal decoder event/data tables,
              and emit JSON {"mosi":[...], "miso":[...], "source": str}

If internal decoder data is unavailable on the instrument/firmware, falls back
to waveform capture (CS/SCLK/MOSI) and deterministic SPI mode-0 decoding in
software from scope-acquired samples (still scope-readback of the physical bus).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from scpi import RigolSCPI, SCPIError  # noqa: E402


def _extract_bytes(text):
    if not text:
        return [], []
    s = str(text)
    # JSON payload
    m = re.search(r"\{.*\"mosi\".*\}", s, re.S)
    if m:
        try:
            obj = json.loads(m.group(0))
            def norm(vals):
                out = []
                for v in vals or []:
                    if isinstance(v, int):
                        out.append(v & 0xFF)
                    elif isinstance(v, str):
                        vs = v.strip().lower()
                        out.append(int(vs, 16) & 0xFF if vs.startswith("0x") else int(vs) & 0xFF)
                return out
            mo, mi = norm(obj.get("mosi")), norm(obj.get("miso"))
            if mo or mi:
                return mo, mi
        except Exception:
            pass
    # labeled hex
    mosi, miso = [], []
    for line in s.replace("\r", "\n").split("\n"):
        hx = [int(x, 16) & 0xFF for x in re.findall(r"0x([0-9A-Fa-f]{1,2})\b", line)]
        if not hx:
            hx = [int(x, 16) & 0xFF for x in re.findall(r"\b([0-9A-Fa-f]{2})\b", line)]
        if not hx:
            continue
        if re.search(r"MOSI|SDI|TX|WRITE|MOSI:", line, re.I):
            mosi.extend(hx)
        elif re.search(r"MISO|SDO|RX|READ|MISO:", line, re.I):
            miso.extend(hx)
    if mosi or miso:
        return mosi, miso
    vals = [int(x, 16) & 0xFF for x in re.findall(r"0x([0-9A-Fa-f]{1,2})\b", s)]
    return (vals, []) if vals else ([], [])


def configure(scope: RigolSCPI, args):
    ch_cs, ch_sclk = args.cs, args.sclk
    ch_mosi, ch_miso = args.mosi, args.miso
    scope.stop()
    for ch in [ch_cs, ch_sclk, ch_mosi, ch_miso]:
        scope.setup_channel(ch, probe="1", scale=args.vdiv, offset=args.offset, display=True)
    scope.setup_timebase(args.tscale, args.toffset)
    scope.setup_acquire("NORM", "AUTO")
    scope.setup_edge_trigger(ch_cs, slope="NEGative", level_v=args.trigger, sweep="SING")
    # Internal decoder config per Rigol DS1000Z manual (:DECoder<n>:SPI, :ETABle<n>)
    scope.setup_spi_decoder(
        decoder=1,
        cs=ch_cs,
        sclk=ch_sclk,
        mosi=ch_mosi,
        miso=ch_miso,
        threshold_v=args.trigger,
        width=8,
        endian="MSB",
        edge="RISE",
        ss_polarity="NEG",
        mode="CS",
    )
    scope.setup_event_table(decoder=1, fmt="HEX", view="PACKage")
    scope.single()
    return {"configured": True, "channels": {"cs": ch_cs, "sclk": ch_sclk, "mosi": ch_mosi, "miso": ch_miso}}


def _parse_tmc_block(raw: bytes) -> bytes:
    if not raw:
        return b""
    if raw[:1] == b"#" and len(raw) > 2 and raw[1:2].isdigit():
        try:
            nd = int(raw[1:2])
            hdr = 2 + nd
            ln = int(raw[2:hdr] or b"0")
            return raw[hdr:hdr + ln]
        except Exception:
            return raw
    return raw


def _etable_hex_payload(payload: bytes):
    if not payload:
        return b""
    # Rigol often returns binary bytes representing hex chars/printable table in HEX mode
    # or raw text. Try to interpret as ASCII text first.
    try:
        txt = payload.decode("ascii", errors="ignore")
    except Exception:
        txt = ""
    # if looks like hex text, prefer it
    if re.search(r"[0-9A-Fa-f]{2}", txt):
        return txt.encode("ascii", errors="ignore")
    # if payload itself contains many 0x30-0x39/41-46/61-66 treat as text
    printable = sum(1 for b in payload if 32 <= b <= 126 or b in (9,10,13))
    if printable >= max(1, len(payload) * 0.7):
        return payload
    # else return as-is (binary)
    return payload


def _extract_from_etable_text(text: str):
    mosi, miso = [], []
    if not text:
        return mosi, miso
    # explicit labels
    for line in text.replace("\r", "\n").split("\n"):
        if not line.strip():
            continue
        hx = [int(x, 16) & 0xFF for x in re.findall(r"0x([0-9A-Fa-f]{1,2})\b", line)]
        if not hx:
            # tokens/space-separated hex
            hx = [int(x, 16) & 0xFF for x in re.findall(r"\b([0-9A-Fa-f]{2})\b", line)]
        if re.search(r"MOSI|SDI", line, re.I):
            mosi.extend(hx)
        elif re.search(r"MISO|SDO", line, re.I):
            miso.extend(hx)
    if mosi or miso:
        return mosi, miso
    # unlabeled: collect hex bytes in order
    vals = [int(x, 16) & 0xFF for x in re.findall(r"0x([0-9A-Fa-f]{1,2})\b", text)]
    if not vals:
        vals = [int(x, 16) & 0xFF for x in re.findall(r"\b([0-9A-Fa-f]{2})\b", text)]
    return vals, []


def read_internal_decoder(scope: RigolSCPI, timeout_s: float, debug: bool = False):
    """Read SPI-decoded bytes from Rigol DS1000Z internal protocol decoder.

    Uses event table readback per DS1000Z Programming Guide:
      :ETABle<n>:DISP/FORMat/VIEW/COLumn/SORT and :ETABle<n>:DATA? (TMC block).
    Tries payload/MOSI views and parses decoded bytes.
    """
    scope.wait_stop(timeout_s=min(timeout_s, 10.0))
    scope.stop()
    time.sleep(0.4)

    def _read_once(fmt="HEX", view="PACKage", col=None):
        try:
            scope.setup_event_table(decoder=1, fmt=fmt, view=view, column=col)
        except Exception:
            pass
        time.sleep(0.1)
        try:
            scope.inst.timeout = 8000
        except Exception:
            pass
        raw = b""
        try:
            raw = scope.read_event_table(1)
        except Exception:
            try:
                scope.write(":ETABle1:DATA?")
                raw = scope.read_raw() or b""
            except Exception:
                raw = b""
        try:
            scope.inst.timeout = scope.timeout_ms
        except Exception:
            pass
        payload = _parse_tmc_block(raw)
        interp = _etable_hex_payload(payload)
        try:
            text = interp.decode("ascii", errors="ignore") if isinstance(interp, (bytes, bytearray)) else str(interp)
        except Exception:
            text = ""
        if debug:
            print(f"[debug] etable fmt={fmt} view={view} col={col} raw={len(raw)} payload={len(payload)} txt={text[:160]!r}", file=sys.stderr)
        mo, mi = _extract_from_etable_text(text)
        # if still empty, try scanning raw bytes as text-ish
        if not mo and not mi and payload:
            try:
                t2 = payload.decode("latin-1", errors="ignore")
                mo, mi = _extract_from_etable_text(t2)
            except Exception:
                pass
        return {"mosi": mo, "miso": mi, "raw_len": len(raw), "payload_len": len(payload), "text": text[:400]}

    attempts = [
        ("HEX", "PACKage", None),
        ("HEX", "PAYLoad", "MOSI"),
        ("HEX", "PAYLoad", "MISO"),
        ("ASC", "PACKage", None),
        ("HEX", "DETail", None),
    ]
    last = None
    mosi_all, miso_all = [], []
    sources = []
    for fmt, view, col in attempts:
        res = _read_once(fmt, view, col)
        last = res
        if res["mosi"]:
            if col == "MISO":
                miso_all = res["mosi"] or miso_all
            else:
                mosi_all = res["mosi"] if not mosi_all else mosi_all
            sources.append(f"etable:{fmt}:{view}:{col or 'ALL'}")
        if res["miso"]:
            miso_all = res["miso"] if not miso_all else miso_all
            sources.append(f"etable:{fmt}:{view}:{col or 'ALL'}:miso")
        # also try to pick miso from MOSI-column read? no
        if mosi_all and miso_all:
            break
    # if payload view for MOSI gave bytes in mosi field even when col set, ok
    if not mosi_all and last and last.get("mosi"):
        mosi_all = last["mosi"]
    src = ",".join(sources) if sources else "internal-decoder:etable:none"
    out = {"mosi": mosi_all, "miso": miso_all, "source": src}
    if debug and last is not None:
        out["debug_payload_text"] = last.get("text", "")
    return out


def _pre_to_volts(pre: str, data):
    parts = [p.strip() for p in (pre or "").split(",")]
    try:
        yincr = float(parts[7]); yorig = float(parts[8]); yref = float(parts[9])
    except Exception:
        return [0.0 for _ in data]
    return [((b - yorig - yref) * yincr) for b in data]


def _decode_spi_mode0(cs_v, sclk_v, mosi_v, threshold_v: float):
    cs = [1 if v > threshold_v else 0 for v in cs_v]
    clk = [1 if v > threshold_v else 0 for v in sclk_v]
    mosi = [1 if v > threshold_v else 0 for v in mosi_v]
    n = min(len(cs), len(clk), len(mosi))
    out = []
    i = 1
    while i < n:
        if cs[i - 1] == 1 and cs[i] == 0:  # frame start
            bits = []
            j = i
            prev = clk[j]
            while j < n and cs[j] == 0:
                c = clk[j]
                if prev == 0 and c == 1:  # rising edge sample
                    bits.append(mosi[j])
                    if len(bits) == 8:
                        val = 0
                        for b in bits:
                            val = (val << 1) | b
                        out.append(val)
                        bits = []
                prev = c
                j += 1
            i = j
        else:
            i += 1
    return out


def read_waveform_decode(scope: RigolSCPI, args, timeout_s: float, debug: bool = False):
    scope.wait_stop(timeout_s=min(timeout_s, 8.0))
    scope.stop()
    time.sleep(0.5)
    waves = {}
    try:
        scope.inst.timeout = 8000
    except Exception:
        pass
    for name, ch in [("cs", args.cs), ("sclk", args.sclk), ("mosi", args.mosi)]:
        try:
            scope.write(f":WAVeform:SOURce {scope._ch(ch)}")
            scope.write(":WAVeform:MODE NORM")
            scope.write(":WAVeform:FORMat BYTE")
            pre, data = scope.read_waveform(ch, chunk=400)
            if debug:
                print(f"[debug] wave {ch} pts={len(data)} pre={str(pre)[:60]}", file=sys.stderr)
        except Exception as e:
            if debug:
                print(f"[debug] waveform read {ch} failed: {e}", file=sys.stderr)
            pre, data = "", []
        if data:
            waves[name] = _pre_to_volts(pre, data)
    try:
        scope.inst.timeout = scope.timeout_ms
    except Exception:
        pass
    if not all(k in waves for k in ("cs", "sclk", "mosi")):
        return {"mosi": [], "miso": [], "source": "waveform:unavailable"}
    mosi = _decode_spi_mode0(waves["cs"], waves["sclk"], waves["mosi"], args.trigger)
    return {"mosi": mosi, "miso": [], "source": "waveform:spi-mode0"}


def main():
    ap = argparse.ArgumentParser(description="Read SPI-decoded bytes from Rigol DS1000Z internal decoder")
    ap.add_argument("--mode", choices=["configure", "readback"], required=True)
    ap.add_argument("--resource", default=os.environ.get("OSCI_RESOURCE", ""))
    ap.add_argument("--cs", default=os.environ.get("HIL_CS_CH", "CHAN1"))
    ap.add_argument("--sclk", default=os.environ.get("HIL_SCLK_CH", "CHAN3"))
    ap.add_argument("--mosi", default=os.environ.get("HIL_MOSI_CH", "CHAN4"))
    ap.add_argument("--miso", default=os.environ.get("HIL_MISO_CH", "CHAN2"))
    ap.add_argument("--trigger", type=float, default=float(os.environ.get("HIL_TRIGGER_V", "1.0")))
    ap.add_argument("--vdiv", type=float, default=1.0)
    ap.add_argument("--offset", type=float, default=-2.0)
    ap.add_argument("--tscale", type=float, default=0.002)
    ap.add_argument("--toffset", type=float, default=0.008)
    ap.add_argument("--timeout", type=float, default=12.0)
    ap.add_argument("--allow-waveform", action="store_true", help="allow software decode from captured waveforms")
    ap.add_argument("--debug", action="store_true")
    args = ap.parse_args()

    resource = args.resource
    if not resource:
        try:
            sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "test"))
            from _shared import _detect_scope_resource  # type: ignore
            resource = _detect_scope_resource(debug=args.debug) or ""
        except Exception:
            resource = ""
    if not resource:
        print(json.dumps({"mosi": [], "miso": [], "error": "no OSCI_RESOURCE"}))
        return 2

    try:
        with RigolSCPI(resource) as scope:
            if args.mode == "configure":
                print(json.dumps(configure(scope, args)))
                return 0
            res = read_internal_decoder(scope, args.timeout, debug=args.debug)
            if not res.get("mosi") and args.allow_waveform:
                res = read_waveform_decode(scope, args, args.timeout, debug=args.debug)
            print(json.dumps(res))
            return 0 if res.get("mosi") else 1
    except Exception as e:
        print(json.dumps({"mosi": [], "miso": [], "error": str(e)}))
        return 2


if __name__ == "__main__":
    sys.exit(main())
