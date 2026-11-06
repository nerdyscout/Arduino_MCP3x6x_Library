# dma-mode Specification

## Purpose
Define the behavioral contract for DMA-backed scan-mode sampling: the ring
buffer data structure, the DMA lifecycle API, how the IRQ handler and
`analogRead()` behave when DMA is enabled, and per-platform enablement rules.
DMA applies to scan mode only; mux mode remains synchronous.

## ADDED Requirements

### Requirement: Sample records carry channel, value, and timestamp
The library SHALL provide a DMA sample record that contains the ADC channel
identifier, the sign-extended ADC value, and the timestamp of the moment the
sample was captured.

#### Scenario: Sample delivered by DMA
- **WHEN** a DMA transfer completes
- **THEN** the pushed sample carries the channel identifier decoded from the
  ADCDATA channel bits, the sign-extended 25-bit value, and a timestamp

### Requirement: Ring buffer stores samples in user-provided memory without heap
The library SHALL store DMA samples in a caller-provided buffer with no heap
allocation, and SHALL report the number of stored samples.

#### Scenario: Buffer of capacity one
- **WHEN** DMA is enabled with a buffer of capacity at least one
- **THEN** samples are stored in that buffer and the reported count matches the
  number of stored samples

#### Scenario: Buffer capacity below one
- **WHEN** DMA is enabled with a buffer capacity of zero
- **THEN** enablement fails and DMA remains disabled

### Requirement: Ring buffer drop policy on overflow
The library SHALL reject a pushed sample when the ring is full without
blocking.

#### Scenario: Overflow attempted
- **WHEN** a sample is pushed into a full ring buffer
- **THEN** the sample is not stored and the ring reports available capacity of
  zero

### Requirement: Ring buffer FIFO order
The library SHALL deliver samples in first-in-first-out order and SHALL support
emptying the ring.

#### Scenario: Order preserved
- **WHEN** samples are pushed and popped in differing orders
- **THEN** each popped sample matches the oldest stored sample

#### Scenario: Ring cleared
- **WHEN** the ring is cleared
- **THEN** the reported count is zero

### Requirement: DMA lifecycle enablement
The library SHALL enable DMA sampling for scan mode given a user buffer, its
capacity, and a platform DMA channel selector, and SHALL reject enablement when
capacity is zero, the platform has no DMA-capable engine, or the selected DMA
channel conflicts with the MCLK timer channel. DMA-capable engines exist only
for ESP32 and SAMD; all other platforms reject enablement and stay synchronous.

#### Scenario: Enable succeeded
- **WHEN** DMA is enabled with capacity at least one on ESP32 or SAMD
- **THEN** subsequent scan-mode IRQ handling uses DMA

#### Scenario: Enable rejected on unsupported platform
- **WHEN** DMA is enabled on a platform other than ESP32 or SAMD
- **THEN** the call fails and the scan path stays synchronous

### Requirement: DMA disablement releases resources and MCLK tone
The library SHALL release the DMA channel, disable the associated peripheral
clocks, and stop the MCLK tone when DMA is disabled.

#### Scenario: Disable after enable
- **WHEN** DMA is disabled while enabled
- **THEN** further IRQ-triggered conversions are read synchronously and the
  MCLK tone output is stopped

### Requirement: Enabled-state query
The library SHALL report whether DMA sampling is currently enabled.

#### Scenario: Query before and after enable
- **WHEN** the enabled-state query is called before enablement and after failed
  or successful enablement
- **THEN** it reports disabled before enablement, disabled after a failed
  enablement, and enabled after a successful enablement

### Requirement: Periodic sample population in scan mode
The library SHALL populate the ring buffer with one sample per completed
conversion when scan mode conversion is ongoing and DMA is enabled on ESP32 or
SAMD, without blocking the scan-mode IRQ handler. On all other platforms DMA
enablement fails and sampling stays synchronous.

#### Scenario: Scan sampling with DMA
- **WHEN** a scan-mode conversion completes while DMA is enabled on ESP32 or SAMD
- **THEN** the scan-mode IRQ handler starts a non-blocking ADCDATA transfer and
  the completed sample is pushed into the ring buffer

### Requirement: Sample count query
The library SHALL report the number of samples currently stored in the ring,
and SHALL report zero when DMA is disabled.

#### Scenario: Count reflects pushes and pops
- **WHEN** samples are pushed and popped
- **THEN** the reported count equals the number of stored samples

#### Scenario: Count without DMA
- **WHEN** the sample count is queried while DMA is disabled
- **THEN** the count is zero

### Requirement: Sample retrieval
The library SHALL pop the oldest sample into a caller-provided record and SHALL
report failure when the ring is empty.

#### Scenario: Pop from empty ring
- **WHEN** a sample is requested while the ring is empty
- **THEN** the retrieval fails without blocking

#### Scenario: Pop from partially filled ring
- **WHEN** a sample is requested while samples are stored
- **THEN** the oldest stored sample is returned and the count decreases

### Requirement: Mux mode stays synchronous
The library SHALL keep mux-mode reads on the synchronous transfer path
regardless of DMA state.

#### Scenario: Mux read with DMA enabled
- **WHEN** a mux-mode read is requested while DMA is enabled
- **THEN** the read does not consult the ring buffer

### Requirement: analogRead DMA drain in scan mode
The library SHALL, in scan mode with DMA enabled, drain samples in FIFO order
until a sample of the requested channel is found and SHALL return a timeout
code when the ring is empty until the configured read timeout expires, with a
zero timeout selecting unbounded waiting.

#### Scenario: Requested channel found in FIFO
- **WHEN** the requested channel appears in the FIFO
- **THEN** its value is returned and earlier samples of other channels are
  consumed in FIFO order

#### Scenario: Empty ring with finite timeout configured
- **WHEN** the ring is empty and a finite read timeout is configured
- **THEN** the timeout code is returned after the configured period elapses

#### Scenario: Zero timeout waits
- **WHEN** the ring is empty and the read timeout is set to zero
- **THEN** the drain keeps waiting until a sample of the requested channel
  arrives
