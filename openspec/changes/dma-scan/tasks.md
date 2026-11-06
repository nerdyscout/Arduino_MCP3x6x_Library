# dma-scan Tasks

## 1. Core primitives (native-testable)
- [ ] 1.1 `DmaSample` struct (channel/value/timestamp) in the header — verified by +cppcheck and `pio test -e native_ci`
- [ ] 1.2 `DmaRingBuffer` setup/push/pop/available/clear (SPSC, no heap, drop-and-report on overflow) — verified by new Unity tests `test_ring_*` (push/pop FIFO, overflow drop, clear→0)
- [ ] 1.3 Library members `_dma` union (padded), `_dmaRing(Buffer)`, `_dmaEnabled` — verified by compile

## 2. DMA lifecycle API (native-testable)
- [ ] 2.1 `enableDMA(buffer, capacity, dmaChannel=0)` — validation (capacity<1, no-engine platform, MCLK/timer conflict) → `false`; success arms the platform engine — verified by `test_enableDMA_*`
- [ ] 2.2 `disableDMA()` (release channel, stop MCLK tone, `_dmaEnabled=false`) and `isDMAEnabled()` — verified by `test_disableDMA_*`, `test_isDMAEnabled_*`
- [ ] 2.3 `samplesAvailable()` (0 when disabled) and `readSample(DmaSample&)` — verified by `test_samplesAvailable_*`, `test_readSample_*`
- [ ] 2.4 `enableDMA()` on unsupported platform (AVR/ESP8266/native) returns `false` — verified by native CI itself

## 3. Scan-path integration (native-testable)
- [ ] 3.1 `IRQ_handler()`: when DMA enabled in scan mode → platform `_dmaStart()` non-blocking, keep `_dataAvailable` semantics — verified by `test_IRQ_handler_dma_*`
- [ ] 3.2 DMA completion callback `_dmaComplete()`: decode ADCDATA (same ID_SGNEXT_DATA strip/sign-extend as `readADCDATA()`), `push(DmaSample)` timestamped, re-arm — verified by `test_dmaComplete_*`
- [ ] 3.3 `analogRead()` scan branch: DMA path drains FIFO until requested channel; empty ring + finite timeout → timeout code; timeout 0 → wait — verified by `test_analogRead_dma_*`
- [ ] 3.4 Mux-mode read keeps synchronous `_transfer()` even with DMA enabled — verified by `test_analogRead_mux_dma_enabled`
- [ ] 3.5 Full native suite green (no regressions, count > 109) — verified by `pio test -e native_ci --project-conf platformio_ci.ini`

## 4. ESP32 engine
- [ ] 4.1 `#ifdef ARDUINO_ARCH_ESP32` engine: `spi_bus_add_device` on the Arduino host (default SPI2_HOST via `dmaChannel`), queue async transaction started by `_dmaStart()`, completion task event → `_dmaComplete()` — verified by compile matrix env ESP32
- [ ] 4.2 MCLK via LEDC PWM; `enableDMA()` rejects conflicting LEDC/timer channel — verified by compile

## 5. SAMD21/SAMD51 engine
- [ ] 5.1 SERCOM RX-buffer trigger via DMAC (CHCTRLA/CHINTENSET), completion → `_dmaComplete()` — verified by compile matrix envs SAMD21G18A + SAMD51P20A
- [ ] 5.2 MCLK via TCC0/TCC1; conflict check in `enableDMA()` — verified by compile

## 6. STM32 engine
- [ ] 6.1 `HAL_SPI_TransmitReceive_DMA()` on SPI handle, DMA IRQ → `_dmaComplete()` — verified by compile matrix env STM32F405RGT6
- [ ] 6.2 MCLK via TIM1/TIM8 PWM; stream conflict check — verified by compile

## 7. nRF52 engine
- [ ] 7.1 SPIM EasyDMA sequence + GPIOTE/PPI start on DR pin; PPI channel ≠ `dmaChannel` — verified by compile matrix env NRF52832
- [ ] 7.2 MCLK via TIMER+PPI; conflict check — verified by compile

## 8. Teensy engine
- [ ] 8.1 LPSPI + `DMAChannel` (eDMA) transfer, completion IRQ → `_dmaComplete()` — verified by compile matrix env IMXRT1062
- [ ] 8.2 MCLK via FlexPWM; channel conflict check — verified by compile

## 9. Packaging, docs, release (version stays 0.2.0)
- [ ] 9.1 `keywords.txt` gains DmaSample/DmaRingBuffer/enableDMA/disableDMA/isDMAEnabled/samplesAvailable/readSample — verified by inspection
- [ ] 9.2 README gains a DMA section (Buffer length ≥ 2× channels, channel-lifetime note, optional ring status dump) and mention in examples table if an example is added — verified by inspection
- [ ] 9.3 CHANGELOG `[0.2.0]` entry gains *Added* DMA items (no version bump) — verified by inspection
- [ ] 9.4 Full verification sweep: `pio test -e native_ci`, cppcheck 0 findings, compile matrix across benchmarked envs (AVR/ESP8266 compile with engines out; ESP32, SAMD21, SAMD51, STM32F405, NRF52, Teensy compile with engines) — verified by tool output
