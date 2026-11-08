# Proposal: examples-and-regression-tests

## Objective
Add simple, fully functional Arduino example sketches under `examples/` and regression tests under `test/` to verify the `Arduino_MCP3x6x_Library` usage and code quality.

## Why (stakeholder needs)
Users need copy-pasteable example sketches that demonstrate how to use the library for common MCP3x6x operating modes (MuxMode, ScanMode, Single Ended, Differential, Temperature, polling, DMA, different Gain/OSR settings) without requiring hardware-specific board configuration. Contributors need regression tests that run both on mock hardware (CI/GitLab pipeline via ArduinoFake) and on real hardware (locally, board-independent) to prevent regressions as the library evolves.

## What changes
- New capability `examples`: a set of Arduino `.ino` sketches under `examples/`, each demonstrating one operating mode of the MCP3x6x library. Each example must compile and run independently of the target board environment.
- New capability `test`: regression tests under `test/` sharing a single codebase between mock-based unit tests (ArduinoFake, GitLab pipeline) and real hardware tests (locally, board-independent).
- Capabilities modified: `examples`, `test`.

## Impact
- `examples/` gains multiple new `.ino` sketches; existing examples remain unchanged.
- `test/` gains new regression test files; existing tests remain unchanged.
- No changes to `src/Arduino_MCP3x6x_Library.hpp` or library API.
- CI GitLab pipeline gains mock-test execution for `test/` (ArduinoFake).
- Local hardware test runs remain independent of board env.

## Non-goals
- No library source code changes.
- No new hardware required beyond what contributors already have.
- Examples are not required to pass automated CI on hardware (they are hardware-only smoke sketches).
- Mock tests do not require an oscilloscope or physical MCP3x6x device.

## Open questions
- OQ-1: Exact list of example sketches (names, MCP variant per sketch, which reading each demonstrates).
- OQ-2: Which test files are added to `test/` beyond the existing 8 files (test_analog.cpp, test_basic.cpp, etc.).
- OQ-3: Whether ArduinoFake mock defines are shared in a common header or per-test-file.
- OQ-4: GitLab pipeline job name and trigger for mock tests.
