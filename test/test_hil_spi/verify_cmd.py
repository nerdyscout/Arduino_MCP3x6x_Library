#!/usr/bin/env python3
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))
from spi_verify import verify  # noqa: E402


if __name__ == "__main__":
    build_env = os.environ.get("PIOENV", "hil_spi")
    debug = os.environ.get("VERBOSE", "") != ""
    do_reset = os.environ.get("HIL_SPI_RESET", "0") == "1"
    try:
        repeat = int(os.environ.get("HIL_SPI_REPEAT", "1"))
    except Exception:
        repeat = 1
    rc = 0
    for i in range(max(1, repeat)):
        if repeat > 1:
            print(f"--- verify run {i+1}/{repeat} ---")
        rc = verify(build_env=build_env, do_reset=do_reset if i == 0 else False, debug=debug, repeat=1)
        if rc != 0:
            sys.exit(rc)
    sys.exit(0)
