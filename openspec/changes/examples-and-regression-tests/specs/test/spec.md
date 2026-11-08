# Spec Delta

## Purpose

New OpenSpec capability tracking for `test/`: a regression test suite sharing a single codebase between mock-based unit tests (ArduinoFake, GitLab pipeline) and real hardware tests (locally, board-independent), ensuring code quality of the `Arduino_MCP3x6x_Library` across changes.

## ADDED Requirements

### Requirement: REQ-001 Shared codebase for mock and real hardware tests
The `test/` directory SHALL use a single set of test source files that compile and run both as mock tests (ArduinoFake) and as real hardware tests, without duplicating test logic between the two modes.

#### Scenario: SCN-001 Same test files for both modes
- **GIVEN** the `test/test_MCP3x6x/` directory contains test source files
- **WHEN** tests are built for mock (ArduinoFake) and for real hardware
- **THEN** the same `.cpp` files are used in both builds

#### Scenario: SCN-002 Conditional compilation for mock vs hardware
- **GIVEN** a test file contains both mock-dependent and hardware-dependent code
- **WHEN** compiled with `PIO_NATIVE_TESTING` defined
- **THEN** mock code paths are active; compiled without it, hardware code paths are active

### Requirement: REQ-002 Mock tests run via ArduinoFake in GitLab pipeline
Mock-based unit tests SHALL be executable in the GitLab CI pipeline using ArduinoFake as the SPI/Arduino framework mock, without physical hardware.

#### Scenario: SCN-003 GitLab pipeline runs mock tests
- **GIVEN** a GitLab CI pipeline is triggered
- **WHEN** the mock test job runs
- **THEN** all mock tests execute and report pass/fail without requiring an oscilloscope or MCP3x6x device

#### Scenario: SCN-004 ArduinoFake mock defines SPI behavior
- **GIVEN** a test verifies `begin()` chip-ID validation
- **WHEN** ArduinoFake provides mock SPI responses
- **THEN** the test injects a status byte and HW_ID nibble and asserts `begin()` returns true on match, false on mismatch

### Requirement: REQ-003 Real hardware tests run independently from env board
Real hardware tests SHALL run on physical Arduino boards independent of the PlatformIO environment, verifying actual SPI transactions and conversion results.

#### Scenario: SCN-005 Hardware test on ATmega328P Nano
- **GIVEN** an Arduino Nano (ATmega328P) wired to an MCP3x6x on default SPI pins
- **WHEN** a real hardware test is uploaded and executed
- **THEN** the test reads actual chip ID, performs conversions, and reports pass/fail via Serial

#### Scenario: SCN-006 Hardware test board-independent
- **GIVEN** a real hardware test sketch
- **WHEN** uploaded to a different board env (e.g., SAMD21)
- **THEN** the test adapts to the board's default SPI pins via the library constructor

### Requirement: REQ-004 Tests verify SPI byte sequences
Tests SHALL verify the exact SPI command/address bytes sent on MOSI and the status/data bytes received on MISO for library operations.

#### Scenario: SCN-007 Reset command byte verified
- **GIVEN** mock SPI records MOSI bytes
- **WHEN** `mcp.reset()` is called
- **THEN** the first MOSI byte equals `0x78` (CMD_RESET)

#### Scenario: SCN-008 Register write bytes verified
- **GIVEN** mock SPI records MOSI bytes
- **WHEN** `mcp.config0()` writes CONFIG0
- **THEN** MOSI byte 1 equals `0x86` (IWRITE|ADR_CONFIG0) and byte 2 equals the CONFIG0 raw value

### Requirement: REQ-005 Tests verify register read/write behavior
Tests SHALL verify that register writes are applied and register reads return expected values.

#### Scenario: SCN-009 CONFIG0 write then read round-trip
- **GIVEN** mock SPI allows write-then-read
- **WHEN** `mcp.config0()` writes a value, then `mcp.read(Config0)` reads it back
- **THEN** the read value equals the written value

### Requirement: REQ-006 Tests verify analogRead conversion results
Tests SHALL verify that `analogRead()` returns an int32 raw value and that `toVoltage()` converts it correctly.

#### Scenario: SCN-010 analogRead returns int32
- **GIVEN** mock SPI returns a known ADCDATA value
- **WHEN** `mcp.analogRead(CH0)` is called
- **THEN** the return type is `int32_t`

#### Scenario: SCN-011 toVoltage converts raw correctly
- **GIVEN** a known raw ADC value and reference voltage
- **WHEN** `mcp.toVoltage(raw)` is called
- **THEN** the returned microvolt value equals `raw * reference / maxValue`

### Requirement: REQ-007 Tests verify chip ID validation
Tests SHALL verify that `begin()` accepts the correct chip-ID nibble and rejects incorrect ones.

#### Scenario: SCN-012 Correct chip ID accepted
- **GIVEN** mock SPI returns HW_ID nibble 0xF for MCP3564
- **WHEN** `mcp.begin()` is called on `MCP3564`
- **THEN** `begin()` returns true

#### Scenario: SCN-013 Incorrect chip ID rejected
- **GIVEN** mock SPI returns HW_ID nibble 0x0 (wrong)
- **WHEN** `mcp.begin()` is called on `MCP3564`
- **THEN** `begin()` returns false

### Requirement: REQ-008 Tests verify begin() success/failure paths
Tests SHALL cover both the happy path (reset succeeds, chip ID matches) and error paths (reset fails, chip ID mismatches).

#### Scenario: SCN-014 begin() happy path
- **GIVEN** mock SPI returns POR=1 status and correct HW_ID
- **WHEN** `mcp.begin()` is called
- **THEN** `begin()` returns true

#### Scenario: SCN-015 begin() failure path
- **GIVEN** mock SPI returns POR=0 status or wrong HW_ID
- **WHEN** `mcp.begin()` is called
- **THEN** `begin()` returns false

### Requirement: REQ-009 All tests pass for code quality acceptance
All regression tests SHALL pass in both mock (CI) and real hardware (local) modes for code quality acceptance.

#### Scenario: SCN-016 Mock test suite passes in CI
- **GIVEN** GitLab pipeline runs the mock test job
- **WHEN** all mock tests complete
- **THEN** zero tests fail and exit code is 0

#### Scenario: SCN-017 Real hardware test suite passes locally
- **GIVEN** physical hardware wired and test sketch uploaded
- **WHEN** all real hardware tests complete
- **THEN** zero tests fail and Serial reports pass for every test

## ADDED Requirements

### Requirement: REQ-010 Existing test files remain functional
All 8 existing test files under `test/test_MCP3x6x/` (test_analog.cpp, test_basic.cpp, test_config.cpp, test_dma.cpp, test_fastcmd_status.cpp, test_misc.cpp, test_runner.cpp, test_scan_mode.cpp) SHALL continue to compile and pass without modification.

#### Scenario: SCN-018 Existing tests pass
- **GIVEN** the existing 8 test files are unchanged
- **WHEN** the test suite is built and run
- **THEN** all existing tests pass
