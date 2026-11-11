# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout
#
# hil_spi.py — PlatformIO custom target: test_hil_spi
# Build+upload+reset+SPI decoder readback+byte comparison on real hardware.

Import("env")
import json
import os
import subprocess
import sys
sys.path.insert(0, 'test')
from _shared import _run, _env_name, _adc_type, _reset_board_via_dtr, _detect_scope_resource  # noqa: E402
from _shared import CHAN_CS, CHAN_SCLK, CHAN_MISO, CHAN_MOSI  # noqa: E402


def target_test_hil_spi(target, source, env):
    env_name = _env_name(env)
    ci_env = env_name + "_ci"
    adc_type = _adc_type(env)
    debug = env.get("VERBOSE_BUILD", False) or "VERBOSE" in os.environ
    print(f"=== Test HIL+SPI: {ci_env} / {adc_type} ===")

    # 1. Build+upload the test firmware (skip monitor, we use decoder.py instead)
    rc = _run(["pio", "test", "-c", "platformio.ini", "-e", ci_env, "--without-testing"])
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
                             "${workspaceFolder}/.config/opencode/skills/scpi/scripts/decoder.py")
    if not os.path.isfile(decoder):
        print(f"ERROR: decoder.py not found at {decoder} (set DECODER_PATH)", file=sys.stderr)
        sys.exit(1)

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


env.AddCustomTarget(
    name="test_hil_spi",
    dependencies=None,
    actions=target_test_hil_spi,
    title="Test HIL+SPI",
    description="Build+upload+reset+SPI decoder readback+byte comparison on real hardware"
)
