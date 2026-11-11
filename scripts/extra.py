# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout
#
# platformio_extra_script.py — Custom PlatformIO targets for HIL testing.
#
# Adds two custom targets runnable on any environment:
#   pio run -t test_hil -e <env>          # Build+upload+run Unity test on real hardware (Serial readback)
#   pio run -t test_hil_spi -e <env>      # Same + SPI bus readback via decoder.py byte comparison
#
# Wire into platformio.ini [env] section:
#   extra_scripts = platformio_extra_script.py

Import("env")
import os
import subprocess
import sys
import time

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

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


# ---------------------------------------------------------------------------
# Target: test_hil
#   Build + upload + run Unity test on real hardware via Serial monitor.
#   Uses pio test which handles build+upload+monitor for hardware envs.
# ---------------------------------------------------------------------------

def target_test_hil(target, source, env):
    env_name = _env_name(env)
    ci_env = env_name + "_ci"
    print(f"=== Test HIL: {ci_env} ===")

    # pio test handles build+upload+monitor for hardware, parses Unity output
    rc = _run(["pio", "test", "-c", "platformio_ci.ini", "-e", ci_env])
    sys.exit(rc)


# ---------------------------------------------------------------------------
# Target: test_hil_spi
#   Build + upload + reset + decoder.py configure/readback + byte comparison.
# ---------------------------------------------------------------------------

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


def target_test_hil_spi(target, source, env):
    env_name = _env_name(env)
    ci_env = env_name + "_ci"
    adc_type = _adc_type(env)
    debug = env.get("VERBOSE_BUILD", False) or "VERBOSE" in os.environ
    print(f"=== Test HIL+SPI: {ci_env} / {adc_type} ===")

    # 1. Build+upload the test firmware (skip monitor, we use decoder.py instead)
    rc = _run(["pio", "test", "-c", "platformio_ci.ini", "-e", ci_env, "--without-testing"])
    if rc != 0:
        print("ERROR: build/upload failed", file=sys.stderr)
        sys.exit(rc)

    # 2. Reset board via DTR to re-run setup() SPI traffic
    if not _reset_board_via_dtr(debug=debug):
        sys.exit(1)

    # 3. Detect scope VISA resource
    resource = _detect_scope_resource(debug=debug)
    if resource is None:
        sys.exit(1)
    print(f"Scope resource: {resource}")

    decoder = os.environ.get("DECODER_PATH",
                             "/home/stefan/.config/opencode/skills/scpi/scripts/decoder.py")
    if not os.path.isfile(decoder):
        print(f"ERROR: decoder.py not found at {decoder} (set DECODER_PATH)", file=sys.stderr)
        sys.exit(1)

    # Scope channel mapping (Nano: CS=10→CHAN1, SCK=13→CHAN3, MISO=12→CHAN2, MOSI=11→CHAN4)
    CHAN_CS = "CHAN1"
    CHAN_SCLK = "CHAN3"
    CHAN_MISO = "CHAN2"
    CHAN_MOSI = "CHAN4"

    # 4. decoder.py configure
    print("Configuring SPI decoder on scope ...")
    cfg_args = [sys.executable, decoder, "--resource", resource,
                "--protocol", "spi", "--decoder", "1", "--mode", "configure", "--json",
                "--sclk", CHAN_SCLK, "--mosi", CHAN_MOSI, "--miso", CHAN_MISO, "--cs", CHAN_CS,
                "--width", "8", "--edge", "rise", "--polarity", "pos",
                "--endian", "msb", "--select", "cs"]
    if debug:
        cfg_args.append("--debug")
    rc = _run(cfg_args)
    if rc != 0:
        print("ERROR: decoder configure failed", file=sys.stderr)
        sys.exit(rc)

    # 5. decoder.py readback
    print("Reading back decoded SPI events ...")
    rb_args = [sys.executable, decoder, "--resource", resource,
               "--protocol", "spi", "--decoder", "1", "--mode", "readback",
               "--json", "--etable", "on"]
    if debug:
        rb_args.append("--debug")
    rc, stdout = subprocess.run(rb_args, capture_output=True, text=True, check=False)
    if debug and stdout:
        print(stdout, end="")
    if rc != 0:
        print("ERROR: decoder readback failed", file=sys.stderr)
        sys.exit(rc)

    # 6. Parse and compare decoded MOSI bytes
    import json
    try:
        events = json.loads(stdout)
    except json.JSONDecodeError as exc:
        print(f"ERROR: decoder output is not valid JSON: {exc}", file=sys.stderr)
        sys.exit(1)

    if isinstance(events, dict):
        events = events.get("events", events)
    if not isinstance(events, list):
        print("ERROR: unexpected decoder output format", file=sys.stderr)
        sys.exit(1)

    expected = [
        ("reset()",              [0x78]),
        ("chip-ID read",         [0x79]),
        ("config0() write",      [0x86, 0xC0]),
        ("config1() write",      [0x88]),
        ("config2() write",      [0x8A]),
        ("config3() write",      [0x8C]),
        ("irq() write",          [0x8E]),
        ("mux() write",          [0x9A, 0x08]),
        ("conversion()",         [0x68]),
        ("readADCDATA()",        [0x81]),
    ]

    transactions = []
    for ev in events:
        if not isinstance(ev, dict):
            continue
        mosi = ev.get("mosi")
        if mosi is None:
            continue
        bytes_list = []
        for b in mosi:
            if isinstance(b, str):
                bytes_list.append(int(b, 16))
            elif isinstance(b, int):
                bytes_list.append(b)
        if bytes_list:
            transactions.append(bytes_list)

    print(f"Decoded {len(transactions)} SPI transaction(s) from scope")
    if len(transactions) != len(expected):
        print(f"ERROR: transaction count mismatch: expected {len(expected)}, got {len(transactions)}",
              file=sys.stderr)
        sys.exit(1)

    all_ok = True
    for (label, exp_bytes), actual in zip(expected, transactions):
        if len(actual) < len(exp_bytes):
            print(f"  MISMATCH {label}: expected {len(exp_bytes)} byte(s), got {len(actual)}: {actual}",
                  file=sys.stderr)
            all_ok = False
            continue
        ok = True
        for i, exp in enumerate(exp_bytes):
            if actual[i] != exp:
                print(f"  MISMATCH {label}: byte[{i}] expected 0x{exp:02X}, got 0x{actual[i]:02X}",
                      file=sys.stderr)
                ok = False
        if ok:
            print(f"  OK {label}: {actual}")
        else:
            all_ok = False

    if not all_ok:
        print("ERROR: SPI byte comparison failed", file=sys.stderr)
        sys.exit(1)

    print("=== All steps passed. Exit 0. ===")
    sys.exit(0)


# ---------------------------------------------------------------------------
# Register custom targets
# ---------------------------------------------------------------------------

env.AddCustomTarget(
    name="test_hil",
    dependencies=None,
    actions=target_test_hil,
    title="Test HIL",
    description="Build+upload+run Unity test on real hardware via Serial readback"
)

env.AddCustomTarget(
    name="test_hil_spi",
    dependencies=None,
    actions=target_test_hil_spi,
    title="Test HIL+SPI",
    description="Build+upload+reset+SPI decoder readback+byte comparison on real hardware"
)
