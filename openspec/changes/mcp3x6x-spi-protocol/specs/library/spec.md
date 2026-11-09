# Spec Delta

## Purpose

Formalize the SPI communication protocol for the `MCP3x6x` class: the `status_t _status` member as the single source of truth for SPI communication status, the SPI command byte format, register map, fast commands, chip-ID validation, and status byte semantics. Every SPI read, write, and fast command SHALL update `_status` from the device's status byte, and all `status_*()` accessor methods SHALL return values derived from `_status`.

## ADDED Requirements

### Requirement: REQ-001 status_t member exists
The `MCP3x6x` class SHALL have a private member variable `_status` of type `status_t` that holds the status byte from the most recent SPI transaction.

#### Scenario: SCN-001 _status member is accessible internally
- **GIVEN** an `MCP3x6x` object is constructed
- **WHEN** any SPI read, write, or fast command is executed
- **THEN** `_status` is updated with the status byte returned by the device

### Requirement: REQ-002 _transfer updates _status
Every call to `_transfer()` SHALL write the status byte received from the device into `_status.raw` via `_spi->transfer(addr)`.

#### Scenario: SCN-002 _transfer writes status byte from MOSI address byte
- **GIVEN** an `MCP3x6x` object with an active SPI connection
- **WHEN** `_transfer(data, addr, size)` is called and the device returns status byte `0x0B` on MISO during the command byte transfer
- **THEN** `_status.raw == 0x0B`

#### Scenario: SCN-003 _transfer updates _status regardless of data size
- **GIVEN** an `MCP3x6x` object
- **WHEN** `_transfer(nullptr, addr, 0)` is called (fast command, no data payload)
- **THEN** `_status.raw` is updated with the status byte returned for the command byte

### Requirement: REQ-003 _fastcmd updates _status via _transfer
Every call to `_fastcmd()` SHALL route through `_transfer()` and therefore update `_status`.

#### Scenario: SCN-003 _fastcmd updates _status through _transfer
- **GIVEN** an `MCP3x6x` object
- **WHEN** `_fastcmd(MCP3x6x_CMD_RESET)` is called
- **THEN** `_status` is updated via `_transfer()` with the status byte returned by the device during the CMD_RESET transaction

### Requirement: REQ-004 status_dr returns _status.dr
`status_dr()` SHALL return the inverted value of `_status.dr` (data ready bit from the most recent SPI transaction).

#### Scenario: SCN-004 status_dr returns inverted _status.dr
- **GIVEN** `_status.dr == 1` (data ready)
- **WHEN** `status_dr()` is called
- **THEN** `status_dr()` returns `false` (inverted per hardware convention: DR=1 means "not ready" in the library's inverted convention)

#### Scenario: SCN-005 status_dr returns inverted _status.dr when DR=0
- **GIVEN** `_status.dr == 0` (data not ready)
- **WHEN** `status_dr()` is called
- **THEN** `status_dr()` returns `true`

### Requirement: REQ-005 status_crccfg returns _status.crccfg
`status_crccfg()` SHALL return the inverted value of `_status.crccfg` (CRC configuration bit from the most recent SPI transaction).

#### Scenario: SCN-006 status_crccfg reads _status.crccfg
- **GIVEN** `_status.crccfg == 1`
- **WHEN** `status_crccfg()` is called
- **THEN** `status_crccfg()` returns `false` (inverted per hardware convention)

#### Scenario: SCN-007 status_crccfg returns inverted _status.crccfg when 0
- **GIVEN** `_status.crccfg == 0`
- **WHEN** `status_crccfg()` is called
- **THEN** `status_crccfg()` returns `true`

### Requirement: REQ-006 status_por returns _status.por
`status_por()` SHALL return the inverted value of `_status.por` (power-on-reset bit from the most recent SPI transaction).

#### Scenario: SCN-008 status_por reads _status.por
- **GIVEN** `_status.por == 1` (POR occurred)
- **WHEN** `status_por()` is called
- **THEN** `status_por()` returns `false` (inverted per hardware convention)

#### Scenario: SCN-009 status_por returns inverted _status.por when 0
- **GIVEN** `_status.por == 0` (no POR)
- **WHEN** `status_por()` is called
- **THEN** `status_por()` returns `true`

### Requirement: REQ-007 status returns _status.raw
`status()` SHALL return the raw value of `_status.raw`.

#### Scenario: SCN-010 status reads _status.raw
- **GIVEN** `_status.raw == 0x0B`
- **WHEN** `status()` is called
- **THEN** `status()` returns `0x0B`

### Requirement: REQ-008 All SPI transactions update _status
Every public method that performs an SPI transaction (register write, register read, fast command, ADC read) SHALL result in `_status` being updated from the device's status byte via `_transfer()`.

#### Scenario: SCN-011 begin() updates _status via reset and chip-ID read
- **GIVEN** an `MCP3x6x` object
- **WHEN** `begin()` is called
- **THEN** `reset()` (which calls `_fastcmd(MCP3x6x_CMD_RESET)`) updates `_status`, and `_validateChipID()` (which calls `_transfer()` on RESERVED3) updates `_status` again

#### Scenario: SCN-012 write(Config0) updates _status
- **GIVEN** an `MCP3x6x` object
- **WHEN** `write(Config0 data)` is called
- **THEN** `_transfer()` is invoked with `MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0` as the address byte, and `_status.raw` is updated from the MISO status byte returned during that transfer

#### Scenario: SCN-013 read(Config0&) updates _status
- **GIVEN** an `MCP3x6x` object
- **WHEN** `read(Config0& data)` is called
- **THEN** `_transfer()` is invoked with `MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0` as the address byte, and `_status.raw` is updated from the MISO status byte

#### Scenario: SCN-014 analogRead(CH0) updates _status
- **GIVEN** an `MCP3x6x` object
- **WHEN** `analogRead(CH0)` is called
- **THEN** `_transfer()` is invoked for MUX write, `CONVERSION` command, and `ADCDATA` read, and `_status` is updated after each `_transfer()` call
