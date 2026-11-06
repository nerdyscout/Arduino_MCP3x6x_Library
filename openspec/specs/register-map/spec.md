# register-map Specification

## Purpose
Define the behavior of the Microchip ADC register map and status reporting implemented by the
Arduino MCP3x6xR library.

## Requirements

### Requirement: Full register map write access requires the LOCK password

The system SHALL allow write access to the full ADC register map only when the LOCK register
contains the write access password entry code 0xA5.

#### Scenario: Successful unlock
- **WHEN** the library writes the password entry code 0xA5 to the LOCK register
- **THEN** write access to the full register map is allowed

#### Scenario: Lock denies full register map writes
- **WHEN** the library writes a key other than 0xA5 to the LOCK register
- **THEN** write access to the full register map is not allowed

#### Scenario: Full reset restores full register map write access
- **WHEN** the Full Reset fast command is executed
- **THEN** the LOCK register is restored to 0xA5 and write access to the full register map is
  allowed

### Requirement: CONFIG2 bias, gain, and auto-zeroing fields use fixed bit positions

The system SHALL place the ADC bias current selection in CONFIG2 bits [7:6], the ADC gain
selection in bits [5:3], the auto-zeroing MUX setting in bit [2], the auto-zeroing reference buffer
setting in bit [1], and the reserved bit in bit [0], which SHALL be set to 1.

#### Scenario: Field placement
- **WHEN** a CONFIG2 value with bias, gain, and auto-zeroing settings is written
- **THEN** each field is placed in its datasheet-defined bit position

#### Scenario: Reserved bit is set
- **WHEN** a CONFIG2 value is written
- **THEN** the reserved bit is set to 1

### Requirement: CONFIG0 internal voltage reference is selected by VREF_SEL

The system SHALL select the internal voltage reference through CONFIG0 bit 7 and SHALL default to
the internal 2.4 V reference after power-on reset.

#### Scenario: Default internal reference after reset
- **WHEN** the ADC completes power-on reset
- **THEN** CONFIG0 bit 7 selects the internal voltage reference set to 2.4 V

#### Scenario: External reference selection
- **WHEN** an external voltage reference is selected
- **THEN** CONFIG0 bit 7 is cleared

### Requirement: STATUS interrupt status bits are positioned by the STATUS byte

The system SHALL report the data ready, CRC checksum, and power-on-reset interrupt status in
STATUS byte bits [2], [1], and [0] respectively.

#### Scenario: Data ready status
- **WHEN** the STATUS byte data ready status bit is set
- **THEN** the data ready interrupt status is reported

#### Scenario: CRC status
- **WHEN** the STATUS byte CRC status bit is set
- **THEN** the CRC status is reported

#### Scenario: Power-on reset status
- **WHEN** the STATUS byte power-on-reset status bit is set
- **THEN** the power-on-reset status is reported

### Requirement: STATUS byte reserved bits and device address acknowledge are positioned

The system SHALL keep STATUS byte bits [7:6] equal to 0 and SHALL place the device address
acknowledge in bits [5:3].

#### Scenario: Reserved bits
- **WHEN** a STATUS byte is read
- **THEN** bits [7:6] are 0

#### Scenario: Device address acknowledge
- **WHEN** a device address acknowledge is signaled
- **THEN** the acknowledge occupies bits [5:3] and equals the device address
