# SPDX-License-Identifier: MIT
# Copyright (c) 2026 nerdyscout

Import("env")

import sys

sys.path.insert(0, "test")
from _shared import _env_name, _run  # noqa: E402


def target_test_hil(target, source, env):
    ci_env = _env_name(env) + "_ci"
    print(f"=== Test HIL: {ci_env} ===")
    rc = _run(["pio", "test", "-c", "platformio.ini", "-e", ci_env, "-f", "test_hil"])
    sys.exit(rc)


env.AddCustomTarget(
    name="test_hil",
    dependencies=None,
    actions=target_test_hil,
    title="Test HIL",
    description="Build, upload, and run HIL Unity tests on real hardware",
)
