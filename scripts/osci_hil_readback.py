#!/usr/bin/env python3
"""
osci_hil_readback.py — Hardware-in-the-loop SPI readback for Arduino_MCP3x6x_Library.

Builds+uploads the hardware test build via PlatformIO, then configures and reads
back decoded SPI events from a Rigol oscilloscope over USBTMC, comparing decoded
MOSI bytes against the library's expected command bytes. Exits 0 iff every step
succeeds and all bytes match.

Usage:
    scripts/osci_hil_readback.py --env ATMEGA328P [--resource VISA] [--port /dev/ttyS0] [--debug]

All repository paths are relative to this script's location. The oscilloscope VISA
resource and Nano serial port are auto-detected when exactly one candidate exists;
otherwise --resource / --port (or OSCI_RESOURCE / OSCI_PORT env vars) are required.
"""

import argparse
import configparser
import json
import os
import subprocess
import sys

import serial.tools.list_ports
import pyvisa

# ---------------------------------------------------------------------------
# Paths (all relative to this script's directory = repo root / scripts)
# ---------------------------------------------------------------------------
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
PLATFORMIO_INI = os.path.join(REPO_ROOT, "platformio.ini")

# decoder.py lives outside the repo; allow override via env var or argument.
DEFAULT_DECODER = "/home/stefan/.config/opencode/skills/scpi/scripts/decoder.py"

# Scope channel mapping (confirmed by user, see openspec/changes/mcp3x6x-nano-osci-hil-readback/design.md)
CHAN_CS = "CHAN1"   # Nano pin 10
CHAN_SCLK = "CHAN3"  # Nano pin 13
CHAN_MISO = "CHAN2"  # Nano pin 12
CHAN_MOSI = "CHAN4"  # Nano pin 11

# Expected decoded MOSI bytes from the test_config.cpp hardware smoke sequence.
# Each entry: (label, list of expected MOSI bytes in order)
EXPECTED_TRANSACTIONS = [
    ("reset()",              [0x78]),
    ("chip-ID read",         [0x79]),
    ("config0() write",      [0x86, 0xC0]),
    ("config1() write",      [0x88]),
    ("config2() write",      [0x8A]),
    ("config3() write",      [0x8C]),
    ("irq() write",          [0x8E]),
    ("mux() write",          [0x9A, 0x08]),
    ("conversion()",         [0x68]),
    ("readADCDATA()",        [0x81]),  # followed by status + 2 data bytes (MISO)
]


def die(message, code=1):
    print(f"ERROR: {message}", file=sys.stderr)
    sys.exit(code)


def run(cmd, cwd=None, debug=False):
    """Run a command, streaming stdout/stderr. Return (returncode, stdout)."""
    if debug:
        print(f"[debug] $ {' '.join(cmd)}", file=sys.stderr)
    try:
        result = subprocess.run(
            cmd,
            cwd=cwd,
            capture_output=True,
            text=True,
            check=False,
        )
    except FileNotFoundError as exc:
        die(f"command not found: {cmd[0]} ({exc})", code=127)
    if debug and result.stdout:
        print(result.stdout, end="")
    if debug and result.stderr:
        print(result.stderr, end="", file=sys.stderr)
    return result.returncode, result.stdout


def discover_envs():
    """Return list of [env:] section names from platformio.ini."""
    if not os.path.isfile(PLATFORMIO_INI):
        die(f"platformio.ini not found at {PLATFORMIO_INI}")
    parser = configparser.ConfigParser()
    parser.read(PLATFORMIO_INI)
    envs = []
    for section in parser.sections():
        if section.startswith("env:"):
            envs.append(section[len("env:"):])
    return envs


def auto_detect_resource(preferred=None, env_var="OSCI_RESOURCE"):
    """Auto-detect a single VISA resource; fall back to preferred/env."""
    if preferred:
        return preferred
    env_val = os.environ.get(env_var)
    if env_val:
        return env_val
    try:
        rm = pyvisa.ResourceManager()
        resources = rm.list_resources()
    except Exception as exc:  # pyvisa may not be installed or backend missing
        die(f"cannot enumerate VISA resources: {exc}")
    if len(resources) == 0:
        die("no VISA resources found; specify --resource or set OSCI_RESOURCE")
    if len(resources) == 1:
        return resources[0]
    die(
        f"multiple VISA resources found ({resources}); "
        f"specify --resource or set OSCI_RESOURCE"
    )


def auto_detect_port(preferred=None, env_var="OSCI_PORT"):
    """Auto-detect a single serial port; fall back to preferred/env."""
    if preferred:
        return preferred
    env_val = os.environ.get(env_var)
    if env_val:
        return env_val
    ports = [p.device for p in serial.tools.list_ports.comports()]
    if len(ports) == 0:
        die("no serial ports found; specify --port or set OSCI_PORT")
    if len(ports) == 1:
        return ports[0]
    die(
        f"multiple serial ports found ({ports}); "
        f"specify --port or set OSCI_PORT"
    )


def reset_nano(port, debug=False):
    """Toggle DTR to reset the Nano so it re-runs setup()."""
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        ser.setDTR(False)
        if debug:
            print(f"[debug] DTR False on {port}", file=sys.stderr)
        ser.close()
        import time
        time.sleep(0.2)
        ser = serial.Serial(port, 115200, timeout=1)
        ser.setDTR(True)
        ser.close()
        if debug:
            print(f"[debug] DTR True on {port} (Nano reset)", file=sys.stderr)
        time.sleep(0.5)
    except Exception as exc:
        die(f"failed to reset Nano on {port}: {exc}")


def call_decoder(args, debug=False):
    """Invoke decoder.py with the given extra args; return parsed JSON or die."""
    decoder_path = os.environ.get("DECODER_PATH", DEFAULT_DECODER)
    if not os.path.isfile(decoder_path):
        die(f"decoder.py not found at {decoder_path} (set DECODER_PATH or pass --decoder)")
    cmd = [sys.executable, decoder_path] + args
    rc, stdout = run(cmd, debug=debug)
    if rc != 0:
        die(f"decoder.py failed (exit {rc})")
    try:
        return json.loads(stdout)
    except json.JSONDecodeError as exc:
        die(f"decoder.py output is not valid JSON: {exc}\noutput: {stdout[:500]}")


def extract_mosi_bytes(events):
    """Extract MOSI byte lists from decoded SPI events."""
    transactions = []
    if isinstance(events, dict):
        events = events.get("events", events)
    if not isinstance(events, list):
        return transactions
    for ev in events:
        if not isinstance(ev, dict):
            continue
        mosi = ev.get("mosi")
        if mosi is None:
            continue
        # Normalize to list of ints
        bytes_list = []
        for b in mosi:
            if isinstance(b, str):
                bytes_list.append(int(b, 16))
            elif isinstance(b, int):
                bytes_list.append(b)
        if bytes_list:
            transactions.append(bytes_list)
    return transactions


def compare_bytes(actual, expected, label):
    """Compare actual MOSI bytes against expected. Returns True on match."""
    if len(actual) < len(expected):
        print(f"  MISMATCH {label}: expected {len(expected)} byte(s), got {len(actual)}: {actual}", file=sys.stderr)
        return False
    for i, exp in enumerate(expected):
        if actual[i] != exp:
            print(f"  MISMATCH {label}: byte[{i}] expected 0x{exp:02X}, got 0x{actual[i]:02X}", file=sys.stderr)
            return False
    print(f"  OK {label}: {actual}")
    return True


def main():
    parser = argparse.ArgumentParser(
        description="HIL SPI readback: pio build/upload + Rigol decoder readback + byte comparison."
    )
    parser.add_argument("--env", required=True, help="PlatformIO environment name (must exist in platformio.ini)")
    parser.add_argument("--resource", default=None, help="VISA resource string (auto-detected if omitted)")
    parser.add_argument("--port", default=None, help="Nano serial port for DTR reset (auto-detected if omitted)")
    parser.add_argument("--decoder", default=None, help="Path to decoder.py (default: DECODER_PATH env or built-in default)")
    parser.add_argument("--debug", action="store_true", help="Pass --debug to decoder.py and print debug info")
    args = parser.parse_args()

    # Resolve decoder path (CLI arg wins over env over default)
    if args.decoder:
        os.environ["DECODER_PATH"] = args.decoder

    # Validate env exists in platformio.ini
    envs = discover_envs()
    if args.env not in envs:
        die(f"environment '{args.env}' not found in platformio.ini. Available: {envs}")

    print(f"=== OSCI HIL Readback ===")
    print(f"Environment : {args.env}")
    print(f"Repo root   : {REPO_ROOT}")
    print()

    # Step 1: pio run
    print("[1/6] pio run -e", args.env)
    rc, _ = run(["pio", "run", "-e", args.env], cwd=REPO_ROOT, debug=args.debug)
    if rc != 0:
        die(f"pio run -e {args.env} failed (exit {rc})")

    # Step 2: pio upload
    print("[2/6] pio upload -e", args.env)
    rc, _ = run(["pio", "upload", "-e", args.env], cwd=REPO_ROOT, debug=args.debug)
    if rc != 0:
        die(f"pio upload -e {args.env} failed (exit {rc})")

    # Step 3: Detect scope resource
    resource = auto_detect_resource(args.resource)
    print(f"[3/6] Scope resource: {resource}")

    # Step 4: Detect serial port + reset Nano
    port = auto_detect_port(args.port)
    print(f"[4/6] Nano serial port: {port} — resetting")
    reset_nano(port, debug=args.debug)

    # Step 5: decoder configure
    print("[5/6] decoder.py configure")
    configure_args = [
        "--resource", resource,
        "--protocol", "spi",
        "--decoder", "1",
        "--mode", "configure",
        "--json",
        "--sclk", CHAN_SCLK,
        "--mosi", CHAN_MOSI,
        "--miso", CHAN_MISO,
        "--cs", CHAN_CS,
        "--width", "8",
        "--edge", "rise",
        "--polarity", "pos",
        "--endian", "msb",
        "--select", "cs",
    ]
    if args.debug:
        configure_args.append("--debug")
    call_decoder(configure_args, debug=args.debug)

    # Step 6: decoder readback + byte comparison
    print("[6/6] decoder.py readback + byte comparison")
    readback_args = [
        "--resource", resource,
        "--protocol", "spi",
        "--decoder", "1",
        "--mode", "readback",
        "--json",
        "--etable", "on",
    ]
    if args.debug:
        readback_args.append("--debug")
    events = call_decoder(readback_args, debug=args.debug)

    actual_txs = extract_mosi_bytes(events)
    print(f"  Decoded {len(actual_txs)} SPI transaction(s) from scope")

    if len(actual_txs) != len(EXPECTED_TRANSACTIONS):
        die(
            f"transaction count mismatch: expected {len(EXPECTED_TRANSACTIONS)}, "
            f"got {len(actual_txs)}"
        )

    all_ok = True
    for (label, expected), actual in zip(EXPECTED_TRANSACTIONS, actual_txs):
        if not compare_bytes(actual, expected, label):
            all_ok = False

    if not all_ok:
        die("byte comparison failed — decoded MOSI bytes do not match expected")

    print()
    print("=== All steps passed. Exit 0. ===")
    sys.exit(0)


if __name__ == "__main__":
    main()
