# Change: DMA-backed scan mode (dma-scan)

## Why

Every SPI transaction is currently a synchronous blocking call executed inside
the IRQ handler. On 32-bit MCUs the MCU can move ADCDATA to a DMA peripheral so
the CPU returns from the interrupt immediately, and conversions can be captured
into a ring buffer at peak scan rate without loss. `PLAN.md` (now deleted)
captured this as a design document; this change turns it into a specification
and ships it. Project decision: ship as part of release **0.2.0** (no version
bump), with the DMA features documented as *Added* in the existing 0.2.0
CHANGELOG entry.

## What Changes

- Add `DmaSample` and a single-producer/single-consumer `DmaRingBuffer`
  (user-provided storage, no heap, ISR-safe).
- Add DMA public API: `enableDMA(buffer, capacity, dmaChannel = 0)`,
  `disableDMA()`, `isDMAEnabled()`, `samplesAvailable()`, `readSample()`.
- Route scan-mode IRQ handling through DMA when enabled: the IRQ handler starts
  a non-blocking ADCDATA read instead of the synchronous `_transfer()`.
- Extend `analogRead()` in scan mode: when DMA is enabled, samples are popped
  from the ring (FIFO order preserved) instead of the synchronous cache.
- Implement DMA engines for ESP32 (ESP-IDF `spi_master`), SAMD21/SAMD51 DMAC
  (SERCOM RX trigger), STM32 (`HAL_SPI_TransmitReceive_DMA`), nRF52 (SPIM
  EasyDMA + GPIOTE/PPI), Teensy IMXRT (`DMAChannel` + LPSPI). Platforms without
  a DMA-capable SPI (AVR, ESP8266) keep the synchronous path; `enableDMA()`
  fails cleanly with `false`.
- Keep CRC-16 validation on DMA reads **deferred** (timing analysis in
  design.md).

## Capabilities

### New Capabilities

- `dma-mode`: Behavioral contract for DMA-backed scan sampling (ring buffer,
  DMA lifecycle API, IRQ/drain paths, platform enablement rules).

### Modified Capabilities

- `adc-api`: scan-mode IRQ handling and `analogRead()` gain DMA branches.

## Impact

- Affected specs: `dma-mode` (new), `adc-api` (MODIFIED).
- Affected code: `src/Arduino_MCP3x6x_Library.hpp` (only source file, header
  only); doc/metadata updates (`README.md`, `keywords.txt`, `CHANGELOG.md`,
  `AGENTS.md`).
- PLAN.md is deleted; its content is migrated into this change's `design.md`.

## Non-Goals

- CRC-16 validation of DMA transfers (deferred per timing analysis;
  recommended only at OSR >= 256 if enabled later).
- DMA in mux mode (stays synchronous `_transfer()`).
- Heap allocation or vtable-based engines.
- Hardware validation (no HIL available; engines verified by compile matrix).

## Open Questions

- None requiring grilling. Bench validation of the five engines remains
  pending-until-hardware (recorded as unimplemented-datasheet feature).
