# Design: examples-and-regression-tests

## Context

The `Arduino_MCP3x6x_Library` supports MCP346x/MCP356x family 16/24-bit ADCs with multiple operating modes (MuxMode, ScanMode, Single Ended, Differential, Temperature, polling, DMA, Gain/OSR settings). Users need copy-pasteable example sketches to understand usage, and contributors need regression tests that run on mock hardware (CI) and real hardware (local) to prevent regressions.

## Goals and non-goals

### Goals
- Add example sketches under `examples/` covering MuxMode, ScanMode, Single Ended, Differential, Temperature, polling, DMA, and Gain/OSR settings.
- Each example must compile and run independently of the target board env.
- Add regression tests under `test/` sharing one codebase between mock (ArduinoFake) and real hardware modes.
- Mock tests must run in GitLab CI pipeline; real hardware tests run locally board-independently.
- No changes to library source code (`src/`, `test/` existing files unchanged).

### Non-goals
- No library API changes.
- Examples are not automated in CI on hardware (they are hardware-only smoke sketches).
- Mock tests do not require an oscilloscope or physical MCP3x6x device.
- No new hardware required.

## Technical approach

### Examples
Each example is a standalone Arduino `.ino` sketch under `examples/<name>/`:
- Uses `#include "Arduino.h"` and `#include "Arduino_MCP3x6x_Library.hpp"`.
- Instantiates an MCP variant class (e.g., `MCP3462 mcp;`, `MCP3564 mcp;`) using default constructor (default SPI pins).
- Calls `Serial.begin(115200)` in `setup()` for diagnostics.
- Demonstrates one operating mode per sketch (MuxMode, ScanMode, Single Ended, Differential, Temperature, polling, DMA, Gain/OSR).
- No board-specific `#ifdef` guards for core functionality.
- Discovered and built automatically by `examples/build.sh`.

### Tests
Test files under `test/test_MCP3x6x/`:
- Single codebase shared between mock and hardware modes.
- Mock mode: compiled with `PIO_NATIVE_TESTING` defined; uses ArduinoFake to mock SPI and Arduino core functions.
- Hardware mode: compiled without `PIO_NATIVE_TESTING`; runs on physical boards.
- Test registration via `register_*_tests()` functions called from `test_runner.cpp`.
- Mock SPI records MOSI bytes and injects MISO responses for byte-level verification.

### ArduinoFake mock
- `ArduinoFake` provides mock `SPI`, `Serial`, `pinMode`, `digitalWrite`, etc.
- Mock SPI records `transfer()` calls (address + data bytes) and returns programmable status/data bytes.
- Tests assert on recorded MOSI bytes and injected MISO bytes.

## Decisions

| Decision | Rationale |
|----------|-----------|
| One change for both `examples/` and `test/` | User requested both folders be sorted in one commit |
| Examples use default constructor (no pin args) | Ensures board independence; default SPI pins work on all supported boards |
| Examples print Serial at 115200 baud | Enables correlation with oscilloscope captures |
| Tests use `PIO_NATIVE_TESTING` for mock mode | Matches existing pattern in `test_basic.cpp` |
| Mock SPI records bytes for byte-level verification | Enables precise verification of command/address bytes without hardware |
| `examples/build.sh` auto-discovers new examples | No script changes needed when adding examples |

## Risks and trade-offs

| Risk | Mitigation |
|------|------------|
| DMA example only works on SAMD boards (DMA not available on ATmega328P) | DMA example clearly documented as SAMD-only; other examples cover ATmega328P |
| Mock tests may not catch hardware-specific timing issues | Real hardware tests run locally to cover timing |
| Examples may not compile for all board envs | Examples tested on ATmega328P (Nano); board independence verified by construction |
| ArduinoFake mock may diverge from real SPI behavior | Real hardware tests validate actual behavior locally |

## File changes

### New files under `examples/`
- `examples/MCP3462_Mux_SingleEnded/` — Single-ended MuxMode reading on MCP3462 (16-bit 4ch)
- `examples/MCP3464_Scan_Continuous/` — Continuous scan mode on MCP3464 (16-bit 8ch)
- `examples/MCP3562_Differential/` — Differential input on MCP3562 (24-bit 4ch)
- `examples/MCP3564_Temperature/` — Temperature sensor reading on MCP3564 (24-bit 8ch)
- `examples/MCP3462_Polling/` — Polling mode (no IRQ/MCLK) on MCP3462
- `examples/MCP3564_DMA/` — DMA continuous read on MCP3564 (SAMD board)
- `examples/MCP3564_GainOSR/` — Gain and OSR configuration on MCP3564

### New files under `test/test_MCP3x6x/`
- `test_spi_bytes.cpp` — Verifies SPI command/address bytes for reset, register writes, reads
- `test_chipid.cpp` — Verifies chip-ID validation accept/reject paths
- `test_analog_read.cpp` — Verifies analogRead return type and toVoltage conversion
- `test_begin_paths.cpp` — Verifies begin() happy path and failure paths

### Modified files under `src/`
- `src/Arduino_MCP3x6x_Library.hpp` — Added `double getTemperature()` method that saves the current MUX channel, switches to the temperature sensor, reads the temperature in centi-Celsius via `readTemperature()`, restores the previous MUX channel, and returns temperature in degrees Celsius. Uses existing `_tempOffset` and `_tempSlope` calibration.

### OpenSpec artifacts
- `openspec/changes/examples-and-regression-tests/.openspec.yaml`
- `openspec/changes/examples-and-regression-tests/proposal.md`
- `openspec/changes/examples-and-regression-tests/specs/examples/spec.md`
- `openspec/changes/examples-and-regression-tests/specs/test/spec.md`
- `openspec/changes/examples-and-regression-tests/design.md`
- `openspec/changes/examples-and-regression-tests/tasks.md`
