# register-map Delta

## MODIFIED Requirements

### Requirement: Full register map write access requires the LOCK password

The system SHALL allow write access to the full ADC register map only when the
LOCK register contains the write access password entry code 0xA5, and SHALL
restore LOCK to 0xA5 after a Full Reset fast command.

#### Scenario: Successful unlock
- **WHEN** the library writes the password entry code 0xA5 to the LOCK register
- **THEN** write access to the full register map is allowed

#### Scenario: Lock denies full register map writes
- **WHEN** the library writes a key other than 0xA5 to the LOCK register
- **THEN** write access to the full register map is not allowed

#### Scenario: Full reset restores full register map write access
- **WHEN** the Full Reset fast command is executed
- **THEN** the LOCK register is restored to 0xA5 and write access to the full
  register map is allowed

### Requirement: STATUS byte reserved bits and device address acknowledge are positioned

The system SHALL keep STATUS byte bits [7:6] equal to 0 and SHALL place the
device address acknowledge in bits [5:3]. The device address is read from bits
[5:4] without inversion.

#### Scenario: Reserved bits
- **WHEN** a STATUS byte is read
- **THEN** bits [7:6] are 0

#### Scenario: Device address acknowledge
- **WHEN** a device address acknowledge is signaled
- **THEN** the acknowledge occupies bits [5:4] and equals the device address,
  returned without bit inversion

## ADDED Requirements

### Requirement: IRQ register status bits are read-only

The library SHALL treat POR_STATUS, CRCCFG_STATUS, and DR_STATUS bits in the
IRQ register (address 0x5) as read-only status bits that are cleared by reading
the register, and SHALL NOT include them as writable configuration bits in the
IRQ register write path.

#### Scenario: IRQ status bits are not written back
- **WHEN** the IRQ register is written via `write(_irq)`
- **THEN** the POR_STATUS, CRCCFG_STATUS, and DR_STATUS bit fields are not
  included in the writable register image

#### Scenario: IRQ configuration bits are writable
- **WHEN** the IRQ register is written via `write(_irq)`
- **THEN** the EN_STP, EN_FASTCMD, and IRQ_MODE[1:0] bit fields are written to
  the device

### Requirement: Chip ID detection via STATUS DEV_ADDR field

The library SHALL identify the ADC chip variant by reading the STATUS byte and
extracting the device address from bits [5:4], SHALL NOT read the undocumented
RESERVED3 register for chip identification.

#### Scenario: Chip ID from STATUS byte
- **WHEN** the library boot sequence reads the STATUS byte
- **THEN** the device address is extracted from STATUS bits [5:4] as
  `(_status.addr >> 1) & 0x03` and mapped to the chip variant

### Requirement: CONFIG0 and RESERVED writes trigger delayed reset/restart

The library SHALL apply a TADC_SETUP delay (256 DMCLK periods) after writing
CONFIG0 (address 0x1) or RESERVED registers (0xB, 0xC), per datasheet Section
5.16.3 which states these writes cause a delayed reset/restart.

#### Scenario: CONFIG0 write triggers delay
- **WHEN** CONFIG0 register is written
- **THEN** a TADC_SETUP delay is applied before the next SPI transaction

#### Scenario: RESERVED write triggers delay
- **WHEN** RESERVED register (0xB or 0xC) is written
- **THEN** a TADC_SETUP delay is applied before the next SPI transaction
