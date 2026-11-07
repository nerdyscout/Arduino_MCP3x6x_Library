# adc-api Delta

## MODIFIED Requirements

### Requirement: Data readback strips the channel identifier and sign-extends the value

The library SHALL strip the channel identifier from the data read for the
configured data format and SHALL sign-extend the ADC value from the data-format
sign bit, consistently across all ADCDATA read paths.

#### Scenario: Channel identifier stripping
- **WHEN** a data read uses the ID_SGNEXT_DATA format
- **THEN** the channel identifier bits are stripped before sign extension

#### Scenario: Sign extension
- **WHEN** the data format requires sign extension
- **THEN** the ADC value is sign-extended from the data-format sign bit

#### Scenario: Sign extension consistency across read paths
- **WHEN** `readADCDATA()` and `read(Adcdata*)` both read ADCDATA with
  ID_SGNEXT_DATA format
- **THEN** both return the same sign-extended value

## ADDED Requirements

### Requirement: analogRead() mux mode waits for DR flag with correct polarity

The library SHALL wait for the data-ready flag with correct polarity in mux mode:
DR_STATUS is logic '0' when data is ready (active-low per datasheet Section
6.8.1), so the wait loop continues while DR_STATUS is '1' (not ready) and exits
when DR_STATUS becomes '0' (ready).

#### Scenario: Mux mode DR wait uses correct polarity
- **WHEN** `analogRead()` is called in mux mode
- **THEN** the loop waits while `_status.dr == 1` (data not ready) and exits
  when `_status.dr == 0` (data ready)
