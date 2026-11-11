# Spec Delta

## Purpose

Hardware bring-up capability: SPI-level verification that the MCP3x6x library drives correct transactions on an ATmega328P/Arduino Nano connected to a 16-bit 4-channel MCP3464-family ADC, observed via oscilloscope capture of MISO/MOSI/CS/CLK.

## ADDED Requirements

### Requirement: REQ-001 Default SPI initialization and idle CS
The library SHALL initialize SPI on the ATmega328P using default pins (CS=10, MOSI=11, MISO=12, SCK=13) and default SPI settings (4 MHz, MSBFIRST, MODE0), keeping CS held HIGH whenever no transaction is in progress.

#### Scenario: SCN-001 First transaction at default settings
- **GIVEN** Nano wired to MCP3464 on default SPI pins
- **WHEN** begin() is called
- **THEN** the scope decodes the first transaction at 4 MHz, MSBFIRST, MODE0, and CS is HIGH before the first CLK edge

#### Scenario: SCN-002 CS returns HIGH between transactions
- **GIVEN** two consecutive library transactions
- **WHEN** the scope observes the bus between them
- **THEN** CS returns to HIGH for the full inter-transaction gap with no mid-bus CS pulse

### Requirement: REQ-002 begin() reset and chip-ID validation
begin() SHALL issue the SPI reset fast command, then read the RESERVED3 register (SREAD|ADR_RESERVED3) and accept the device only when the MISO status byte's lower nibble equals the expected chip ID for a 16-bit 4-channel device (0x9); it SHALL return false on mismatch.

#### Scenario: SCN-003 Valid chip ID accepted
- **GIVEN** an MCP3464-family 4-channel device on the bus
- **WHEN** begin() executes
- **THEN** MOSI byte 1 = 0x78 (CMD_RESET), followed by MOSI 0x79 (SREAD|ADR_RESERVED3), MISO low nibble = 0x9, and begin() returns true

#### Scenario: SCN-004 Invalid chip ID rejected
- **GIVEN** a device whose RESERVED3 low nibble differs from 0x9
- **WHEN** begin() validates the chip ID
- **THEN** begin() returns false and no configuration-write transactions follow

### Requirement: REQ-003 Register write framing (IWRITE)
Every register write SHALL transfer one command|address byte on MOSI followed by the register data byte(s), read one status byte on MISO after the first byte, keep CS LOW for the entire transfer, and deassert CS only after the last byte.

#### Scenario: SCN-005 CONFIG0 write during begin()
- **GIVEN** config0() write during begin()
- **WHEN** the scope decodes the transaction
- **THEN** MOSI byte 1 = 0x86 (IWRITE|ADR_CONFIG0), byte 2 = CONFIG0 raw 0xC0, MISO byte 1 = status byte, and CS rises after byte 2

#### Scenario: SCN-006 MUX write for analogRead(CH0)
- **GIVEN** analogRead(CH0) in mux mode
- **WHEN** the scope decodes the MUX transaction
- **THEN** MOSI byte 1 = 0x9A (IWRITE|ADR_MUX), byte 2 = 0x08 (CH0+/AGND-), and CS stays LOW across both bytes

### Requirement: REQ-004 Register read framing (SREAD/IREAD)
Every register read SHALL transfer one command|address byte on MOSI, then read the data byte(s) on MISO while MOSI clocks 0x00, keep CS LOW for address+data, and deassert after the last byte.

#### Scenario: SCN-007 ADCDATA read, 16-bit default format
- **GIVEN** readADCDATA() with 16-bit resolution and default data format
- **WHEN** the scope decodes the transaction
- **THEN** MOSI byte 1 = 0x81 (SREAD|ADR_ADCDATA), MISO returns the status byte followed by exactly 2 data bytes in big-endian wire order, and CS rises after the last data byte

### Requirement: REQ-005 Fast-command framing
Fast commands (reset, conversion, standby) SHALL transfer exactly one command byte with CS LOW, read one status byte on MISO, then deassert CS; conversion() sends CMD_CONVERSION (0x68).

#### Scenario: SCN-008 Conversion command from analogRead()
- **GIVEN** conversion() called from analogRead()
- **WHEN** the scope decodes the transaction
- **THEN** MOSI byte 1 = 0x68 (CMD_CONVERSION), exactly 1 byte is transferred, and MISO byte 1 = status byte

### Requirement: REQ-006 analogRead polling completes on floating inputs
analogRead(CHn) with floating inputs SHALL write MUX, issue CONVERSION, read ADCDATA, and return an int32 value within the read timeout; the value is undefined noise because the inputs are undriven.

#### Scenario: SCN-009 Polling read returns on floating input
- **GIVEN** floating CH0, no IRQ, no MCLK, default indefinite read timeout
- **WHEN** analogRead(CH0) is called
- **THEN** MOSI shows MUX write (0x9A) → CONVERSION (0x68) → ADCDATA read (0x81), and the call returns an int32 once the status DR bit clears

#### Scenario: SCN-010 Finite read timeout enforced
- **GIVEN** a finite read timeout set and a non-responsive bus
- **WHEN** analogRead() exceeds the timeout
- **THEN** it returns MCP3x6x_TIMEOUT (INT32_MIN) instead of blocking forever

### Requirement: REQ-007 Serial diagnostics for capture correlation
The smoke sketch SHALL print begin() result and per-channel raw ADC values to Serial at 115200 baud so oscilloscope captures can be correlated with console output.

#### Scenario: SCN-011 Console output synchronized with captures
- **GIVEN** the smoke sketch running on the Nano
- **WHEN** begin() and analogRead() execute
- **THEN** Serial prints begin() success/failure and per-channel raw values at 115200 baud

### Requirement: REQ-008 Build and upload on ATMEGA328P env
The smoke sketch SHALL compile and upload via PlatformIO `ATMEGA328P` env (`board = nanoatmega328`) without errors and without modifying library source.

#### Scenario: SCN-012 Build and upload succeed
- **GIVEN** PlatformIO `ATMEGA328P` env selected
- **WHEN** `pio run -e ATMEGA328P` and the upload target run
- **THEN** the build succeeds, upload completes, and the sketch runs on the Nano

### Requirement: REQ-009 SCPI oscilloscope capture of SPI transactions
The oscilloscope SHALL be controllable via SCPI to arm a single-sequence acquisition triggered on CS falling edge across MISO/MOSI/CS/CLK and export the trace; the exported trace SHALL contain at least one complete address+data transaction decodable to the expected MOSI command byte and MISO bytes.

#### Scenario: SCN-013 CS-triggered SPI capture exported
- **GIVEN** scope connected to the 4 SPI lines
- **WHEN** SCPI single-sequence trigger on CS falling edge is armed and a library transaction occurs
- **THEN** the scope returns a trace containing ≥1 full transaction decodable to the expected MOSI command byte and MISO status/data bytes

### Requirement: REQ-010 Transaction latency bound
Each complete SPI transaction SHALL finish within 50 ms on the 16 MHz ATmega328P at 4 MHz SPI.

#### Scenario: SCN-014 Transaction duration within bound
- **GIVEN** a register read or write call
- **WHEN** measured from CS falling edge to CS rising edge
- **THEN** the scope shows transaction duration ≤ 50 ms
