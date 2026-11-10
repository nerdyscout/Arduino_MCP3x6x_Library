# Spec Delta

## Purpose

Hardware-in-the-loop (HIL) readback capability: automated PlatformIO upload of `test_config.cpp` on an ATmega328P/Arduino Nano, SPI decode readback from an auto-detected Rigol oscilloscope over USBTMC via `decoder.py`, and byte-for-byte comparison of decoded instrument-side SPI events against `test_config.cpp`'s expected MOSI bytes. Extends the `hardware` capability created by `mcp3464-spi-debug-arduino-nano/` (REQ-001…REQ-010) with orchestration requirements REQ-011…REQ-017.

This is a delta specification: requirements marked ADDED are new; requirements that restate prior capability behavior are referenced, not duplicated.

## ADDED Requirements

### Requirement: REQ-011 test_config.cpp hardware build and upload on ATMEGA328P
`test_config.cpp` SHALL compile and upload via PlatformIO `ATMEGA328P` env (`board = nanoatmega328`) using its hardware-only path (`#ifndef PIO_NATIVE_TESTING`) without modifying library source, keeping default SPI pins CS=10, MOSI=11, MISO=12, SCK=13 at 4 MHz MODE0.

Traces to: stakeholder need "test_config.cpp hardware path builds and uploads via PlatformIO ATMEGA328P env".

#### Scenario: SCN-011 Hardware build and upload succeed
- **GIVEN** PlatformIO `ATMEGA328P` env selected and Nano connected
- **WHEN** `pio run -e ATMEGA328P` and `pio upload -e ATMEGA328P` run against the `test_config.cpp` hardware build
- **THEN** the build exits 0, upload completes, and the sketch runs on the Nano

#### Scenario: SCN-012 Native path is not affected by the hardware build target
- **GIVEN** the `ATMEGA328P` build target is selected
- **WHEN** `pio run -e ATMEGA328P` compiles `test_config.cpp`
- **THEN** the `#ifdef PIO_NATIVE_TESTING` mock tests are excluded and the `#ifndef PIO_NATIVE_TESTING` hardware path is compiled for AVR

### Requirement: REQ-012 Hardware smoke sequence executes on real Nano
The `test_config.cpp` hardware `setup()` SHALL run `begin()`, `unlock()`, register writes (config0/config1/config2/config3/irq/mux), then poll `analogRead(CH0..CH3)`, and SHALL print diagnostics to Serial at 115200 baud; `loop()` SHALL keep the sketch alive.

Traces to: stakeholder need "Hardware smoke sequence executes on the Nano ... with Serial output at 115200 baud".

#### Scenario: SCN-013 Full smoke sequence runs and prints diagnostics
- **GIVEN** Nano running the uploaded `test_config.cpp` hardware sketch, Serial monitor at 115200 baud
- **WHEN** `setup()` executes `begin()`, `unlock()`, config0/1/2/3/irq/mux writes, then `analogRead(CH0..CH3)` polling
- **THEN** Serial prints begin() result, per-register-write status, and per-channel raw ADC values at 115200 baud, and `loop()` continues running without resetting

#### Scenario: SCN-014 begin() failure halts and reports
- **GIVEN** `begin()` returns false on the Nano (e.g. chip-ID mismatch)
- **WHEN** `setup()` detects the failure
- **THEN** Serial prints a failure message and the sketch halts (does not proceed to register writes or analogRead)

### Requirement: REQ-013 Oscilloscope auto-detection over USBTMC
The orchestration SHALL auto-detect the connected Rigol when exactly one VISA resource is present (decoder.py resource auto-detect), and SHALL honor a `--resource` override supplied via environment variable when multiple or no resources are auto-detected.

Traces to: stakeholder need "Oscilloscope auto-detected over USBTMC ... with --resource override via env var".

#### Scenario: SCN-015 Single Rigol resource auto-detected
- **GIVEN** exactly one Rigol VISA resource is connected over USBTMC
- **WHEN** the orchestration runs decoder.py without `--resource`
- **THEN** decoder.py selects the single resource and configure/readback succeed

#### Scenario: SCN-016 Resource override when auto-detect is ambiguous
- **GIVEN** zero or multiple VISA resources are present and `OSCI_RESOURCE` is set
- **WHEN** the orchestration passes `--resource $OSCI_RESOURCE` to decoder.py
- **THEN** decoder.py targets the specified resource instead of auto-detecting

#### Scenario: SCN-017 No usable VISA resource fails fast
- **GIVEN** no VISA resource is present and no `--resource` override is set
- **WHEN** decoder.py runs
- **THEN** decoder.py exits non-zero and the orchestration propagates the failure (no silent fallback)

### Requirement: REQ-014 decoder.py configure mode programs SPI decoder on correct channels
`decoder.py --protocol spi --decoder 1 --mode configure --json` SHALL program the Rigol's SPI decoder 1 on the confirmed channel mapping (CS=CHAN1, SCLK=CHAN3, MISO=CHAN2, MOSI=CHAN4, width 8, edge rise, polarity pos, endian msb, select cs) and SHALL verify each setting by readback.

Traces to: stakeholder need "decoder.py configure mode programs the scope's SPI decoder on the confirmed channel mapping".

#### Scenario: SCN-018 SPI decoder configured on confirmed channel map
- **GIVEN** Rigol reachable over USBTMC, decoder 1 unused
- **WHEN** `decoder.py --protocol spi --decoder 1 --mode configure --json --sclk CHAN3 --mosi CHAN4 --miso CHAN2 --cs CHAN1 --width 8 --edge rise --polarity pos --endian msb --select cs` runs
- **THEN** decoder 1 is programmed with SCLK=CHAN3, MOSI=CHAN4, MISO=CHAN2, CS=CHAN1, width 8, edge rise, polarity pos, endian msb, select cs, and each setting readback-verified; JSON output reports success

#### Scenario: SCN-019 Configure failure aborts before readback
- **GIVEN** decoder.py configure mode cannot program decoder 1 (e.g. instrument busy or invalid channel)
- **WHEN** configure mode runs
- **THEN** decoder.py exits non-zero and the orchestration does not proceed to readback mode

### Requirement: REQ-015 decoder.py readback mode returns decoded SPI events as JSON
`decoder.py --protocol spi --decoder 1 --mode readback --json --etable on` SHALL query `:ETABle<1>:DATA?` and return decoded SPI events as JSON; the orchestration SHALL compare the decoded MOSI command bytes against the expected bytes emitted by `test_config.cpp` (enumerated in scenario SCN-020).

Traces to: stakeholder need "correlation of instrument-side decoded SPI events with test_config.cpp's expected MOSI bytes".

#### Scenario: SCN-020 Decoded MOSI bytes match expected sequence
- **GIVEN** `test_config.cpp` hardware sketch running on the Nano, SPI decoder 1 configured on the Nano's SPI lines
- **WHEN** `decoder.py --mode readback --json --etable on` queries `:ETABle<1>:DATA?`
- **THEN** the JSON contains decoded SPI events whose MOSI bytes include 0x78, 0x79, 0x86+0xC0, 0x88, 0x8A, 0x8C, 0x8E, 0x9A+0x08, 0x68, 0x81+status+2 data bytes in the order emitted by the smoke sequence

#### Scenario: SCN-021 Unexpected or missing MOSI byte is a mismatch
- **GIVEN** readback JSON is parsed
- **WHEN** an expected MOSI byte is absent or an unexpected command byte appears
- **THEN** the comparison reports a mismatch and the orchestration exits non-zero

#### Scenario: SCN-022 Empty event table is treated as failure
- **GIVEN** `:ETABle<1>:DATA?` returns no decoded events
- **WHEN** readback mode runs
- **THEN** decoder.py/orchestration reports no events and exits non-zero (no false pass)

### Requirement: REQ-016 Orchestration script exits 0 end-to-end with exit-code propagation
`scripts/osci_hil_readback.sh` SHALL run `pio run -e ATMEGA328P && pio upload -e ATMEGA328P`, auto-detect the Rigol, run configure then readback, compare decoded events to expected bytes, propagate every command's exit code, exit 0 only when all steps succeed, and pass `--debug` through to decoder.py.

Traces to: stakeholder need "a scripted go/no-go command that exits 0 end-to-end and propagates failures".

#### Scenario: SCN-023 Happy path exits 0
- **GIVEN** Nano connected, exactly one Rigol over USBTMC, decoder venv present
- **WHEN** `scripts/osci_hil_readback.sh` runs
- **THEN** build+upload, configure, readback, and byte comparison all succeed and the script exits 0

#### Scenario: SCN-024 Step failure propagates non-zero exit
- **GIVEN** any orchestration step fails (build, upload, configure, readback, or byte mismatch)
- **WHEN** the script runs
- **THEN** the script exits non-zero at the first failing step and does not mask the exit code

#### Scenario: SCN-025 Debug passthrough
- **GIVEN** `--debug` is passed to the script
- **WHEN** the script runs decoder.py
- **THEN** `--debug` is forwarded to decoder.py so SCPI traffic is logged to stderr

### Requirement: REQ-017 Native tests still pass (no regression)
The `test_config.cpp` native mock tests under `#ifdef PIO_NATIVE_TESTING` SHALL remain unchanged and SHALL still pass under the `native` env; adding the hardware path SHALL not alter native test behavior.

Traces to: stakeholder need "Native tests still pass (no regression): test_config.cpp mock tests under #ifdef PIO_NATIVE_TESTING unchanged".

#### Scenario: SCN-026 Native mock tests pass unchanged
- **GIVEN** `pio test -e native` runs `test_config.cpp`
- **WHEN** the hardware path (`#ifndef PIO_NATIVE_TESTING`) has been added
- **THEN** all native mock tests under `#ifdef PIO_NATIVE_TESTING` still pass and their registration/code is unmodified

#### Scenario: SCN-027 Hardware path is excluded from native build
- **GIVEN** `PIO_NATIVE_TESTING` is defined (native env)
- **WHEN** `test_config.cpp` is compiled
- **THEN** the `setup()`/`loop()` hardware function is not compiled and no AVR-only symbols are referenced in native mode
