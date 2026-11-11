# Spec Delta

## Purpose

New OpenSpec capability tracking for `examples/`: a collection of simple, fully functional Arduino `.ino` sketches that demonstrate how to use the `Arduino_MCP3x6x_Library` for common MCP3x6x operating modes. Each example is intended to be copied, adapted, and run by users to understand library usage without requiring board-specific configuration.

## ADDED Requirements

### Requirement: REQ-001 Each example is a fully functional sketch
Every example under `examples/` SHALL be a complete Arduino sketch (`.ino` file with `setup()` and `loop()`) that compiles for at least the ATmega328P/Arduino Nano board env and runs without hardware-specific preprocessor conditions.

#### Scenario: SCN-001 Example compiles for ATmega328P
- **GIVEN** PlatformIO `ATMEGA328P` env (`board = nanoatmega328`) is selected
- **WHEN** `pio run -e ATMEGA328P` is executed on an example directory
- **THEN** the build exits 0 with no errors or warnings treated as errors

#### Scenario: SCN-002 Example has setup() and loop()
- **GIVEN** an example `.ino` file under `examples/`
- **WHEN** the file is inspected
- **THEN** it contains both `void setup()` and `void loop()` functions

### Requirement: REQ-002 Each example demonstrates a distinct operating mode
Each example SHALL focus on one distinct MCP3x6x operating mode: MuxMode, ScanMode, Single Ended, Differential, Temperature, polling, DMA, or different Gain/OSR settings.

#### Scenario: SCN-003 MuxMode example
- **GIVEN** an example named for MuxMode
- **WHEN** the example source is inspected
- **THEN** it calls `mcp.mux()` or configures MUX register for single-channel selection

#### Scenario: SCN-004 ScanMode example
- **GIVEN** an example named for ScanMode
- **WHEN** the example source is inspected
- **THEN** it calls `mcp.enableScanChannel()`, `mcp.startScan()`, or `mcp.startContinuous()`

#### Scenario: SCN-005 Single Ended example
- **GIVEN** an example named for Single Ended input
- **WHEN** the example source is inspected
- **THEN** it configures MUX with a single-ended channel (e.g., `MCP3x6x_CH0` vs AGND)

#### Scenario: SCN-006 Differential example
- **GIVEN** an example named for Differential input
- **WHEN** the example source is inspected
- **THEN** it configures MUX with a differential channel pair (e.g., `MCP3x6x_DIFFA`)

#### Scenario: SCN-007 Temperature example
- **GIVEN** an example named for Temperature reading
- **WHEN** the example source is inspected
- **THEN** it enables the temperature channel via MUX or SCAN

#### Scenario: SCN-008 Polling example
- **GIVEN** an example named for polling mode
- **WHEN** the example source is inspected
- **THEN** it does not call `attachIRQ()` or `attachMCLK()` and reads via `analogRead()` in `loop()`

#### Scenario: SCN-009 DMA example
- **GIVEN** an example named for DMA mode
- **WHEN** the example source is inspected
- **THEN** it uses DMA-capable board features (e.g., SAMD) or `analogReadContinuous()` with DMA descriptors

#### Scenario: SCN-010 Gain/OSR example
- **GIVEN** an example named for Gain/OSR settings
- **WHEN** the example source is inspected
- **THEN** it calls `mcp.config1()` with a non-default OSR and/or `mcp.config2()` with a non-default gain

### Requirement: REQ-003 Examples run independent from board env
Examples SHALL not depend on PlatformIO environment-specific build flags or board-specific pin definitions beyond the default SPI pins (CS=10, MOSI=11, MISO=12, SCK=13 on Nano) or the constructor's default arguments.

#### Scenario: SCN-011 No board-specific build flags required
- **GIVEN** an example sketch
- **WHEN** it is compiled for two different board envs (e.g., ATmega328P and SAMD21)
- **THEN** it compiles for both without environment-specific `#ifdef` guards for core functionality

#### Scenario: SCN-012 Default SPI pins used
- **GIVEN** an example instantiates `MCP3x6x mcp;` with no pin arguments
- **WHEN** the sketch runs on an ATmega328P Nano
- **THEN** CS=10, MOSI=11, MISO=12, SCK=13 are used by default

### Requirement: REQ-004 Examples use different MCP variants
Examples SHALL use different variants of the MCP3x6x family where applicable (e.g., MCP3462 16-bit 4ch, MCP3464 16-bit 8ch, MCP3562 24-bit 4ch, MCP3564 24-bit 8ch).

#### Scenario: SCN-013 Multiple MCP variants across examples
- **GIVEN** the set of all example sketches
- **WHEN** the class instantiation in each example is inspected
- **THEN** at least two distinct `MCP3x6x<N,M>` template specializations are used across the set

### Requirement: REQ-005 Examples print diagnostics to Serial
Examples SHALL print initialization results and key readings to Serial at 115200 baud so users can correlate behavior with oscilloscope captures.

#### Scenario: SCN-014 Serial diagnostics present
- **GIVEN** an example sketch running on hardware
- **WHEN** `setup()` executes
- **THEN** `Serial.begin(115200)` is called and initialization status is printed

## ADDED Requirements

### Requirement: REQ-006 Examples build in the existing examples matrix
The `examples/build.sh` matrix SHALL continue to build all examples under `examples/` for all declared PlatformIO envs without modification to the script.

#### Scenario: SCN-015 build.sh discovers new examples
- **GIVEN** a new example directory is added under `examples/`
- **WHEN** `./examples/build.sh` is run
- **THEN** the new example is auto-discovered and built for all envs
