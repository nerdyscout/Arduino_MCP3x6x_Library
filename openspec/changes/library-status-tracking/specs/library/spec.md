# Spec Delta

## Purpose

Formalize the `status_t _status` class member as the single source of truth for SPI communication status in the `MCP3x6x` class. Every SPI read, write, and fast command SHALL update `_status` from the device's status byte, and all `status_*()` accessor methods SHALL return values derived from `_status`.

## ADDED Requirements

### Requirement: REQ-001 status_t member exists
The `MCP3x6x` class SHALL have a private member variable `_status` of type `status_t` that holds the status byte from the most recent SPI transaction.

#### Scenario: SCN-001 _status member is accessible internally
- **GIVEN** an MCP3x6x object is constructed
- **WHEN** any SPI read, write, or fast command is executed
- **THEN** `_status` is updated with the status byte returned by the device

### Requirement: REQ-002 _transfer updates _status
Every call to `_transfer()` SHALL write the status byte received from the device into `_status.raw`.

#### Scenario: SCN-002 _transfer writes status byte
- **GIVEN** an MCP3x6x object with an active SPI connection
- **WHEN** `_transfer(data, addr, size)` is called and the device returns status byte `0x0B` on MISO
- **THEN** `_status.raw == 0x0B`

### Requirement: REQ-003 _fastcmd updates _status via _transfer
Every call to `_fastcmd()` SHALL result in `_status` being updated through `_transfer()`.

#### Scenario: SCN-003 _fastcmd updates _status
- **GIVEN** an MCP3x6x object
- **WHEN** `_fastcmd(MCP3x6x_CMD_RESET)` is called
- **THEN** `_status` is updated via `_transfer()` with the status byte returned by the device

### Requirement: REQ-004 status_dr returns _status.dr
`status_dr()` SHALL return the value of `_status.dr` (data ready bit from the most recent SPI transaction).

#### Scenario: SCN-004 status_dr reads _status.dr
- **GIVEN** `_status.dr == 1`
- **WHEN** `status_dr()` is called
- **THEN** `status_dr()` returns `false` (inverted per hardware convention)

### Requirement: REQ-005 status_crccfg returns _status.crccfg
`status_crccfg()` SHALL return the value of `_status.crccfg` (CRC configuration bit from the most recent SPI transaction).

#### Scenario: SCN-005 status_crccfg reads _status.crccfg
- **GIVEN** `_status.crccfg == 1`
- **WHEN** `status_crccfg()` is called
- **THEN** `status_crccfg()` returns `false` (inverted per hardware convention)

### Requirement: REQ-006 status_por returns _status.por
`status_por()` SHALL return the value of `_status.por` (power-on-reset bit from the most recent SPI transaction).

#### Scenario: SCN-006 status_por reads _status.por
- **GIVEN** `_status.por == 1`
- **WHEN** `status_por()` is called
- **THEN** `status_por()` returns `false` (inverted per hardware convention)

### Requirement: REQ-007 status returns _status.raw
`status()` SHALL return the raw value of `_status.raw`.

#### Scenario: SCN-007 status reads _status.raw
- **GIVEN** `_status.raw == 0x0B`
- **WHEN** `status()` is called
- **THEN** `status()` returns `0x0B`

## ADDED Requirements

### Requirement: REQ-008 All SPI transactions update _status
Every public method that performs an SPI transaction (register write, register read, fast command, ADC read) SHALL result in `_status` being updated from the device's status byte.

#### Scenario: SCN-008 All public SPI methods update _status
- **GIVEN** an MCP3x6x object
- **WHEN** any public method that performs SPI communication is called (e.g., `analogRead()`, `write(Config0)`, `read(Config0&)`, `conversion()`, `unlock()`)
- **THEN** `_status` is updated with the status byte from that transaction
