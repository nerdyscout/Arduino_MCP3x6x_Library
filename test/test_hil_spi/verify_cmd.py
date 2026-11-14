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
    rc = verify(build_env=build_env, do_reset=do_reset, debug=debug)
    sys.exit(rc)
