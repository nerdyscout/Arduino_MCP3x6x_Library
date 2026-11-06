# adc-api Delta

## ADDED Requirements

### Requirement: Scan-mode reads SHALL drain the DMA ring when DMA is enabled
The library SHALL return scan-mode values from the synchronous results cache
when DMA is disabled, and SHALL return scan-mode values by draining the DMA
sample ring in FIFO order until the requested channel is found when DMA is
enabled, else a timeout code after the configured read timeout.

#### Scenario: Scan read without DMA
- **WHEN** an analog read is requested in scan mode while DMA is disabled
- **THEN** the value is returned from the synchronous results cache

#### Scenario: Scan read with DMA enabled
- **WHEN** an analog read is requested in scan mode while DMA is enabled
- **THEN** the value is returned by draining the DMA sample ring in FIFO order
  until the requested channel is found, or a timeout code when the ring is
  empty until the configured read timeout expires

### Requirement: IRQ handler SHALL use non-blocking transfer in DMA scan mode
The library SHALL read conversion data synchronously when DMA is disabled, and
SHALL start a non-blocking ADCDATA-to-DMA transfer when the channel-ready event
is raised in scan mode with DMA enabled, keeping the data-available flag
semantics.

#### Scenario: Conversion ready without DMA
- **WHEN** the conversion-ready event is raised while DMA is disabled
- **THEN** conversion data is read synchronously and the data-available state
  is raised

#### Scenario: Conversion ready with DMA in scan mode
- **WHEN** the conversion-ready event is raised in scan mode while DMA is
  enabled
- **THEN** a non-blocking transfer of ADCDATA into the DMA buffer is started
  and the data-available state is raised, and the completed transfer is pushed
  into the sample ring
