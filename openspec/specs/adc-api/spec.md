# adc-api Specification

## Purpose
Define the behavioral contract for the library-level ADC operations that depend on or produce
correct datasheet behavior for MCP346xR and MCP356xR ADCs.

## Requirements

### Requirement: Boot performs reset and chip identification without LOCK writes

The system SHALL reset the ADC and probe the power-on-reset status and chip identification during
boot, and SHALL not write the LOCK register.

#### Scenario: Boot does not write the LOCK register
- **WHEN** the library boot sequence runs
- **THEN** the LOCK register is not written

#### Scenario: Boot resets and probes status
- **WHEN** the library boot sequence runs
- **THEN** the ADC is reset and the power-on-reset status is probed before chip identification

### Requirement: Continuous and one-shot scan conversion modes

The system SHALL start a continuous scan conversion cycle by selecting continuous conversion mode
and SHALL start a one-shot conversion cycle that places the ADC in standby after the conversion.

#### Scenario: Continuous scan
- **WHEN** the library starts a scan
- **THEN** the conversion mode is set to continuous and a conversion is triggered

#### Scenario: One-shot scan
- **WHEN** the library starts a one-shot scan
- **THEN** the conversion mode is set to one-shot and the ADC is placed in standby after the
  conversion

### Requirement: Differential input read maps the two selectors without range validation

The system SHALL map the differential input pair from the two supplied input selectors and SHALL
accept any 4-bit selector combination without imposing channel range validation.

#### Scenario: Differential read maps selectors
- **WHEN** a differential input read is requested with two selectors
- **THEN** the input multiplexer is programmed with the positive selector as VIN+ and the negative
  selector as VIN-

#### Scenario: Any selector pair is accepted
- **WHEN** a differential input read is requested with selectors outside the validated channel set
- **THEN** the differential input read is still performed

### Requirement: Raw ADC code to voltage conversion applies gain and full-scale

The system SHALL convert a raw ADC code to input-referred voltage with the datasheet transfer
function and SHALL report a maximum representable code of 2^(N-1).

#### Scenario: Voltage conversion
- **WHEN** a raw code, configured gain, and configured reference are given
- **THEN** the input-referred voltage equals the raw code times the reference divided by 2^(N-1)
  times the gain, with gain 1/3 handled as multiplication by 3

#### Scenario: Maximum representable code
- **WHEN** the maximum value is requested
- **THEN** 2^(N-1) is returned

### Requirement: Data readback strips the channel identifier and sign-extends the value

The system SHALL strip the channel identifier from the data read for the configured data format and
SHALL sign-extend the ADC value from the data-format sign bit.

#### Scenario: Channel identifier stripping
- **WHEN** a data read uses the ID_SGNEXT_DATA format
- **THEN** the channel identifier bits are stripped before sign extension

#### Scenario: Sign extension
- **WHEN** the data format requires sign extension
- **THEN** the ADC value is sign-extended from the data-format sign bit

### Requirement: Last conversion channel is exposed

The system SHALL expose the channel of the most recent conversion through a last-result accessor.

#### Scenario: Last channel access
- **WHEN** a conversion result has been read
- **THEN** the last conversion channel accessor returns that channel
