# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout

import os
import subprocess
import sys
import time


def _run(cmd, debug=False):
    if debug:
        print(f"[debug] $ {' '.join(cmd)}", file=sys.stderr)
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=False)
    except FileNotFoundError as exc:
        print(f"ERROR: command not found: {cmd[0]} ({exc})", file=sys.stderr)
        return 127
    if result.stdout:
        print(result.stdout, end="")
    if result.stderr:
        print(result.stderr, end="", file=sys.stderr)
    if debug:
        print(f"[debug] rc={result.returncode}", file=sys.stderr)
    return result.returncode


def _env_name(env):
    return env.get("PIOENV", "ATMEGA328P")


def _reset_board_via_dtr(debug=False):
    env_port = os.environ.get("BOARD_PORT") or os.environ.get("OSCI_PORT")
    if env_port:
        ports = [env_port]
    else:
        import serial.tools.list_ports
        all_ports = [p.device for p in serial.tools.list_ports.comports()]
        ports = [p for p in all_ports if "USB" in p or "ACM" in p or "usb" in p.lower()] or all_ports
    if not ports:
        print("ERROR: no serial ports found for board reset", file=sys.stderr)
        return False
    if len(ports) > 1:
        print(f"WARNING: multiple serial ports ({ports}), using first: {ports[0]}", file=sys.stderr)
    port = ports[0]
    try:
        import serial
        ser = serial.Serial(port, 115200, timeout=1)
        try:
            ser.setDTR(False)
            time.sleep(0.2)
            ser.setDTR(True)
            time.sleep(0.5)
        finally:
            ser.close()
        if debug:
            print(f"[debug] DTR reset on {port}", file=sys.stderr)
    except Exception as exc:
        print(f"ERROR: failed to reset board on {port}: {exc}", file=sys.stderr)
        return False
    return True


def _detect_scope_resource(debug=False):
    import pyvisa
    env_resource = os.environ.get("OSCI_RESOURCE")
    if env_resource:
        if debug:
            print(f"[debug] using OSCI_RESOURCE={env_resource}", file=sys.stderr)
        return env_resource
    rm = None
    try:
        rm = pyvisa.ResourceManager()
        resources = rm.list_resources()
    except Exception as exc:
        print(f"ERROR: cannot enumerate VISA resources: {exc}", file=sys.stderr)
        return None
    finally:
        if rm is not None:
            try:
                rm.close()
            except Exception:
                pass
    if len(resources) == 0:
        print("ERROR: no VISA resources found; connect oscilloscope", file=sys.stderr)
        return None
    if len(resources) > 1:
        print(f"ERROR: multiple VISA resources ({resources}); set OSCI_RESOURCE", file=sys.stderr)
        return None
    return resources[0]
