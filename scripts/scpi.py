#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Minimal SCPI/VISA helper for Rigol DS1000Z-series oscilloscopes.

Based on SCPI-1999 conventions and common Rigol DS1000Z programming commands.
Provides connection management, basic IEEE 488.2/SCPI I/O, and helpers for
channel, trigger, timebase, acquisition, and protocol-decoder setup.
"""

from __future__ import annotations

import time
from typing import Any, Iterable, Optional


class SCPIError(RuntimeError):
    pass


class RigolSCPI:
    def __init__(self, resource: str, timeout_ms: int = 15000, chunk_size: int = 102400, retries: int = 4):
        self.resource = resource
        self.timeout_ms = timeout_ms
        self.chunk_size = chunk_size
        self.retries = retries
        self.rm = None
        self.inst = None

    def open(self):
        try:
            import pyvisa  # type: ignore
        except Exception as e:  # pragma: no cover
            raise SCPIError(f"pyvisa unavailable: {e}")
        last = None
        for i in range(self.retries):
            rm = None
            try:
                rm = pyvisa.ResourceManager()
                inst = rm.open_resource(self.resource)
                inst.timeout = self.timeout_ms
                inst.chunk_size = self.chunk_size
                # clear stale state
                try:
                    inst.clear()
                except Exception:
                    pass
                self.rm, self.inst = rm, inst
                return self
            except Exception as e:
                last = e
                try:
                    if rm:
                        rm.close()
                except Exception:
                    pass
                time.sleep(0.4 * (i + 1))
        raise SCPIError(f"open failed for {self.resource}: {last}")

    def close(self):
        for obj in (self.inst, self.rm):
            try:
                if obj is not None:
                    obj.close()
            except Exception:
                pass
        self.inst = None
        self.rm = None

    def __enter__(self):
        return self.open()

    def __exit__(self, exc_type, exc, tb):
        self.close()
        return False

    # ---- low-level I/O ----
    def write(self, cmd: str) -> bool:
        if not self.inst:
            raise SCPIError("not connected")
        try:
            self.inst.write(cmd)
            return True
        except Exception:
            return False

    def query(self, cmd: str) -> Optional[str]:
        if not self.inst:
            raise SCPIError("not connected")
        try:
            return self.inst.query(cmd)
        except Exception:
            return None

    def query_raw(self, cmd: str) -> Optional[bytes]:
        if not self.inst:
            raise SCPIError("not connected")
        try:
            self.inst.write(cmd)
            return self.inst.read_raw()
        except Exception:
            return None

    def read_raw(self) -> Optional[bytes]:
        if not self.inst:
            raise SCPIError("not connected")
        try:
            return self.inst.read_raw()
        except Exception:
            return None

    def query_binary_values(self, cmd: str, datatype: str = "B", container=list):
        if not self.inst:
            raise SCPIError("not connected")
        return self.inst.query_binary_values(cmd, datatype=datatype, container=container)

    # ---- common SCPI ----
    def idn(self) -> str:
        return (self.query("*IDN?") or "").strip()

    def opc(self) -> bool:
        r = self.query("*OPC?")
        return (r or "").strip() in ("1", "1\n", "+1")

    def cls(self):
        self.write("*CLS")

    def stop(self):
        self.write(":STOP")

    def run(self):
        self.write(":RUN")

    def single(self):
        self.write(":SINGle")

    def tforce(self):
        self.write(":TFORce")

    def trig_status(self) -> str:
        return (self.query(":TRIGger:STATus?") or "").strip()

    def wait_stop(self, timeout_s: float = 10.0, poll_s: float = 0.2) -> bool:
        deadline = time.time() + timeout_s
        while time.time() < deadline:
            st = self.trig_status()
            if st in ("STOP", "TD", "T'D"):
                return True
            time.sleep(poll_s)
        return False

    # ---- scope setup helpers ----
    @staticmethod
    def _ch(ch: Any) -> str:
        s = str(ch).upper().replace("CHANNEL", "CHAN")
        return ("CHAN" + s) if s.isdigit() else s

    def setup_channel(self, ch: Any, probe: str = "1", scale: float = 1.0, offset: float = 0.0, display: bool = True):
        c = self._ch(ch)
        self.write(f":{c}:DISPlay {'ON' if display else 'OFF'}")
        self.write(f":{c}:PROBe {probe}")
        self.write(f":{c}:SCALe {scale}")
        self.write(f":{c}:OFFSet {offset}")

    def setup_timebase(self, scale_s: float, offset_s: float = 0.0):
        self.write(f":TIMebase:MAIN:SCALe {scale_s}")
        self.write(f":TIMebase:MAIN:OFFSet {offset_s}")

    def setup_acquire(self, acq_type: str = "NORM", mdepth: str = "AUTO"):
        self.write(f":ACQuire:TYPE {acq_type}")
        self.write(f":ACQuire:MDEPth {mdepth}")

    def setup_edge_trigger(self, source: Any, slope: str = "NEGative", level_v: float = 1.0, sweep: str = "SING"):
        src = self._ch(source)
        self.write(":TRIGger:MODE EDGE")
        self.write(f":TRIGger:EDGE:SOURce {src}")
        self.write(f":TRIGger:EDGE:SLOPe {slope}")
        self.write(f":TRIGger:EDGE:LEVel {level_v}")
        self.write(f":TRIGger:SWEep {sweep}")

    def setup_spi_decoder(
        self,
        decoder: int = 1,
        cs: Any = "CHAN1",
        sclk: Any = "CHAN3",
        mosi: Any = "CHAN4",
        miso: Any = "CHAN2",
        threshold_v: float = 1.0,
        width: int = 8,
        endian: str = "MSB",
        edge: str = "RISE",
        ss_polarity: str = "NEG",
    ):
        d = int(decoder)
        chs = {k: self._ch(v) for k, v in {"cs": cs, "sclk": sclk, "mosi": mosi, "miso": miso}.items()}
        cmds = [
            f":DECoder{d}:MODE SPI",
            f":DECoder{d}:DISPlay ON",
            f":DECoder{d}:FORMat HEX",
            f":DECoder{d}:THReshold:{chs['cs']} {threshold_v}",
            f":DECoder{d}:THReshold:{chs['sclk']} {threshold_v}",
            f":DECoder{d}:THReshold:{chs['mosi']} {threshold_v}",
            f":DECoder{d}:THReshold:{chs['miso']} {threshold_v}",
            f":DECoder{d}:THReshold:AUTO OFF",
            f":DECoder{d}:SPI:SS {chs['cs']}",
            f":DECoder{d}:SPI:SEL {chs['cs']}",
            f":DECoder{d}:SPI:NSEL {chs['cs']}",
            f":DECoder{d}:SPI:SCLK {chs['sclk']}",
            f":DECoder{d}:SPI:CLK {chs['sclk']}",
            f":DECoder{d}:SPI:MOSI {chs['mosi']}",
            f":DECoder{d}:SPI:SDI {chs['mosi']}",
            f":DECoder{d}:SPI:MISO {chs['miso']}",
            f":DECoder{d}:SPI:SDO {chs['miso']}",
            f":DECoder{d}:SPI:MODE TIM",
            f":DECoder{d}:SPI:TIMing ON",
            f":DECoder{d}:SPI:EDGE {edge}",
            f":DECoder{d}:SPI:POLarity {ss_polarity}",
            f":DECoder{d}:SPI:SS:POLarity {ss_polarity}",
            f":DECoder{d}:SPI:SSPolarity {ss_polarity}",
            f":DECoder{d}:SPI:ENDian {endian}",
            f":DECoder{d}:SPI:ORDer {endian}",
            f":DECoder{d}:SPI:WIDTh {int(width)}",
            f":DECoder{d}:SPI:WORD {int(width)}",
            f":DECoder{d}:SPI:BITs {int(width)}",
        ]
        for cmd in cmds:
            self.write(cmd)

    # ---- waveform helpers ----
    def waveform_preamble(self, source: Any):
        self.write(f":WAVeform:SOURce {self._ch(source)}")
        self.write(":WAVeform:MODE NORM")
        self.write(":WAVeform:FORMat BYTE")
        pre = (self.query(":WAVeform:PREamble?") or "").strip()
        return pre

    def waveform_data(self, start: int = 1, stop: int = 1200):
        self.write(f":WAVeform:STARt {int(start)}")
        self.write(f":WAVeform:STOP {int(stop)}")
        try:
            return self.query_binary_values(":WAVeform:DATA?", datatype="B", container=list)
        except Exception:
            raw = self.query_raw(":WAVeform:DATA?") or b""
            if raw[:1] == b"#" and len(raw) > 2:
                nlen = int(raw[1:2])
                hdr = 2 + nlen
                cnt = int(raw[2:hdr] or b"0")
                return list(raw[hdr:hdr + cnt])
            return list(raw)

    def read_waveform(self, source: Any, chunk: int = 600):
        pre = self.waveform_preamble(source)
        parts = [p.strip() for p in pre.split(",")] if pre else []
        try:
            points = int(float(parts[2])) if len(parts) > 2 else 1200
        except Exception:
            points = 1200
        data = []
        start = 1
        while start <= points:
            stop = min(points, start + chunk - 1)
            part = self.waveform_data(start, stop)
            if not part:
                break
            data.extend(part)
            start = stop + 1
        return pre, data[:points]
