# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout
#
# PlatformIO custom target: test_hil_spi (full build+upload+verify)

Import("env")

import os
import sys

sys.path.insert(0, "test")
sys.path.insert(0, "test/test_hil_spi")
from _shared import _env_name, _reset_board_via_dtr, _run  # noqa: E402
from spi_verify import verify  # noqa: E402


def target_test_hil_spi(target, source, env):
    env_name = _env_name(env)
    build_env = env_name if env_name in ("hil_spi", "ATMEGA328P_ci") else env_name + "_ci"
    debug = bool(env.get("VERBOSE_BUILD", False) or os.environ.get("VERBOSE"))
    print(f"=== Test HIL+SPI: {build_env} ===")

    rc = _run(["pio", "test", "-c", "platformio.ini", "-e", build_env, "-f", "test_hil_spi", "--without-testing"], debug=debug)
    if rc != 0:
        print("ERROR: build/upload failed", file=sys.stderr)
        sys.exit(rc)

    _reset_board_via_dtr(debug=debug)
    sys.exit(verify(build_env=build_env, do_reset=False, debug=debug))


env.AddCustomTarget(
    name="test_hil_spi",
    dependencies=None,
    actions=target_test_hil_spi,
    title="Test HIL+SPI",
    description="Build, upload, and verify SPI fast-command sequence on real hardware",
)
