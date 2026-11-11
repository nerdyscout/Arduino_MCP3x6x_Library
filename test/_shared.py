# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout
#
# _shared.py — Shared helpers for PlatformIO HIL extra scripts.
# Imported by test/hil.py and test/hil_spi.py.

import os
import subprocess
import sys
import time


def _run(cmd, debug=False):
    """Run a command, streaming output. Return returncode."""
    if debug:
        print(f"[debug] $ {' '.join(cmd)}", file=sys.stderr)
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=False)
    except FileNotFoundError as exc:
        print(f"ERROR: command not found: {cmd[0]} ({exc})", file=sys.stderr)
        return 127
    if debug:
        if result.stdout:
            print(result.stdout, end="")
        if result.stderr:
            print(result.stderr, end="", file=sys.stderr)
    return result.returncode


def _env_name(env):
    return env.get("PIOENV", "ATMEGA328P")


def _adc_type(env):
    """Extract TEST_ADC_TYPE from build flags, default MCP3564."""
    bflags = env.get("BUILD_FLAGS", [])
    if isinstance(bflags, str):
        bflags = bflags.split()
    for f in bflags:
        if f.startswith("-DTEST_ADC_TYPE="):
            return f.split("=", 1)[1]
    return "MCP3564"


def _reset_board_via_dtr(debug=False):
    """Toggle DTR on the single detected serial port to reset the board."""
    import serial.tools.list_ports
    ports = [p.device for p in serial.tools.list_ports.comports()]
    if len(ports) == 0:
        print("ERROR: no serial ports found for board reset", file=sys.stderr)
        return False
    if len(ports) > 1:
        print(f"WARNING: multiple serial ports ({ports}), using first: {ports[0]}", file=sys.stderr)
    port = ports[0]
    try:
        import serial
        ser = serial.Serial(port, 115200, timeout=1)
        ser.setDTR(False)
        ser.close()
        time.sleep(0.2)
        ser = serial.Serial(port, 115200, timeout=1)
        ser.setDTR(True)
        ser.close()
        time.sleep(0.5)
        if debug:
            print(f"[debug] DTR reset on {port}", file=sys.stderr)
    except Exception as exc:
        print(f"ERROR: failed to reset board on {port}: {exc}", file=sys.stderr)
        return False
    return True


def _detect_scope_resource(debug=False):
    """Auto-detect exactly one VISA resource for the oscilloscope."""
    import pyvisa
    try:
        rm = pyvisa.ResourceManager()
        resources = rm.list_resources()
    except Exception as exc:
        print(f"ERROR: cannot enumerate VISA resources: {exc}", file=sys.stderr)
        return None
    if len(resources) == 0:
        print("ERROR: no VISA resources found; connect oscilloscope", file=sys.stderr)
        return None
    if len(resources) > 1:
        print(f"ERROR: multiple VISA resources ({resources}); specify one", file=sys.stderr)
        return None
    return resources[0]


# Scope channel mapping (Nano: CS=10→CHAN1, SCK=13→CHAN3, MISO=12→CHAN2, MOSI=11→CHAN4)
CHAN_CS = "CHAN1"
CHAN_SCLK = "CHAN3"
CHAN_MISO = "CHAN2"
CHAN_MOSI = "CHAN4"
