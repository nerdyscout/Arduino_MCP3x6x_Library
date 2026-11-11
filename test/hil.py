# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout
#
# hil.py — PlatformIO custom target: test_hil
# Build+upload+run Unity test on real hardware via Serial readback.

Import("env")
import sys
sys.path.insert(0, 'test')
from _shared import _run, _env_name  # noqa: E402


def target_test_hil(target, source, env):
    env_name = _env_name(env)
    ci_env = env_name + "_ci"
    print(f"=== Test HIL: {ci_env} ===")

    # pio test handles build+upload+monitor for hardware, parses Unity output
    rc = _run(["pio", "test", "-c", "platformio.ini", "-e", ci_env])
    sys.exit(rc)


env.AddCustomTarget(
    name="test_hil",
    dependencies=None,
    actions=target_test_hil,
    title="Test HIL",
    description="Build+upload+run Unity test on real hardware via Serial readback"
)
