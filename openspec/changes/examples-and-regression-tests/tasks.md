# Tasks

## 0. Library change

- [x] T-000.1 Add `double getTemperature()` method to `MCP3x6x` class in `src/Arduino_MCP3x6x_Library.hpp` — saves current MUX, switches to temperature sensor, reads via `readTemperature()`, restores MUX, returns temperature in Celsius. (Priority: high — est. ~15 min.)

## 1. Example sketches

- [x] T-001.1 Create `examples/MCP3462_Mux_SingleEnded/` — single-ended MuxMode sketch on MCP3462 (16-bit 4ch), `setup()` calls `begin()` + `mux()`, `loop()` reads CH0 via `analogRead()`, Serial diagnostics at 115200. Verify compiles for ATmega328P via `examples/build.sh`. (Priority: high — est. ~30 min. Verifies REQ-001, REQ-002, REQ-003, REQ-005, REQ-006, REQ-008.)
- [x] T-001.2 Create `examples/MCP3464_Scan_Continuous/` — ScanMode continuous sketch on MCP3464 (16-bit 8ch), enables CH0+CH1 scan, reads via `analogReadContinuous()`, Serial diagnostics. Verify compiles for ATmega328P. (Priority: high — est. ~30 min. Verifies REQ-001, REQ-002, REQ-003, REQ-005, REQ-007.)
- [x] T-001.3 Create `examples/MCP3562_Differential/` — Differential input sketch on MCP3562 (24-bit 4ch), configures MUX differential pair, reads via `analogRead()`, Serial diagnostics. Verify compiles for ATmega328P. (Priority: high — est. ~30 min. Verifies REQ-001, REQ-002, REQ-003, REQ-005, REQ-008.)
- [x] T-001.4 Create `examples/MCP3564_Temperature/` — Temperature reading sketch on MCP3564 (24-bit 8ch), uses `getTemperature()` method, Serial diagnostics. Verify compiles for ATmega328P. (Priority: medium — est. ~30 min. Verifies REQ-001, REQ-002, REQ-003, REQ-005, REQ-007.)
- [x] T-001.5 Create `examples/MCP3462_Polling/` — Polling mode sketch (no IRQ/MCLK) on MCP3462, reads via `analogRead()` in `loop()`, Serial diagnostics. Verify compiles for ATmega328P. (Priority: medium — est. ~20 min. Verifies REQ-001, REQ-002, REQ-003, REQ-005, REQ-008.)
- [x] T-001.6 Create `examples/MCP3564_DMA/` — DMA continuous read sketch on MCP3564 (24-bit 8ch, SAMD board), uses DMA descriptors or `analogReadContinuous()`. Verify compiles for SAMD21 env. (Priority: medium — est. ~45 min. Verifies REQ-001, REQ-002, REQ-003, REQ-005, REQ-009.)
- [x] T-001.7 Create `examples/MCP3564_GainOSR/` — Gain/OSR configuration sketch on MCP3564, calls `config1()` with non-default OSR and `config2()` with non-default gain, reads via `analogRead()`, Serial diagnostics. Verify compiles for ATmega328P. (Priority: medium — est. ~30 min. Verifies REQ-001, REQ-002, REQ-003, REQ-005, REQ-010.)
- [x] T-001.8 Verify all examples build in `examples/build.sh` matrix for ATmega328P env without modification to `build.sh`. (Priority: high — est. ~15 min. Verifies REQ-006.)

## 2. Regression tests

- [x] T-002.1 Create `test/test_MCP3x6x/test_spi_bytes.cpp` — verifies SPI command/address bytes for CMD_RESET (0x78), IWRITE/SREAD register transactions, MOSI byte sequences. Verify compiles and passes in mock mode (ArduinoFake). (Priority: high — est. ~45 min. Verifies REQ-004, SCN-007, SCN-008.)
- [x] T-002.2 Create `test/test_MCP3x6x/test_chipid.cpp` — verifies chip-ID validation: correct HW_ID nibble accepted (begin() returns true), incorrect nibble rejected (begin() returns false) for each MCP variant class. Verify compiles and passes in mock mode. (Priority: high — est. ~45 min. Verifies REQ-007, SCN-012, SCN-013.)
- [x] T-002.3 Create `test/test_MCP3x6x/test_analog_read.cpp` — verifies analogRead returns int32_t, toVoltage() converts raw value correctly (raw * reference / maxValue). Verify compiles and passes in mock mode. (Priority: high — est. ~45 min. Verifies REQ-006, SCN-010, SCN-011.)
- [x] T-002.4 Create `test/test_MCP3x6x/test_begin_paths.cpp` — verifies begin() happy path (POR=1 + correct HW_ID → true) and failure paths (POR=0 or wrong HW_ID → false). Verify compiles and passes in mock mode. (Priority: high — est. ~45 min. Verifies REQ-008, SCN-014, SCN-015.)
- [x] T-002.5 Register new tests in `test_runner.cpp` / `register_*_tests()` so they are discovered and executed. Verify all new tests appear in test output. (Priority: high — est. ~15 min. Verifies REQ-001, REQ-009.)
- [x] T-002.6 Verify all 8 existing test files still compile and pass unchanged. (Priority: high — est. ~15 min. Verifies REQ-010.)

## 3. CI pipeline (mock tests)

- [x] T-003.1 Add native test environment to `platformio.ini` modeled after `platformio_ci.ini` native_ci, using ArduinoFake lib dep. Verify mock tests run locally via `pio test -e native`. (Priority: medium — est. ~60 min. Verifies REQ-002, SCN-003, SCN-016.)
- [x] T-003.2 Verify mock tests do not require hardware (no oscilloscope, no MCP3x6x device needed). (Priority: medium — est. ~15 min. Verifies REQ-002, SCN-004.)

## 4. Verification and handoff

- [x] T-004.1 Correlate example Serial output with expected behavior; record pass/fail for every example (compiles, runs, prints diagnostics). (Priority: high — est. ~30 min. Verifies REQ-001…REQ-005.)
- [x] T-004.2 Record pass/fail for every test (mock + hardware): REQ-001…REQ-010, list any deviations. (Priority: high — est. ~30 min. Verifies REQ-001…REQ-010.)
- [x] T-004.3 Write short results note in change folder for review and commit. (Priority: high — est. ~15 min.)
- [x] T-004.4 [engineer follow-up] Commit only the results note and new example/test files — no library code changes. (Priority: low — est. ~10 min.)

## Workflow follow-up

- Review results with the user; any library behavior deviating from the spec becomes a separate change through grilling.
- Archive this change with `/opsx-archive` once review is complete.
