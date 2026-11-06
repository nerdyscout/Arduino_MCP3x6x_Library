# Design: DMA-backed scan mode

Context migrated from PLAN.md (deleted) and grilling decisions recorded in the
conversation: complete DMA (core + all five platform engines), CRC deferred,
version stays 0.2.0, no HIL available (compile-matrix verified).

## Scope

DMA applies to **scan mode only**; mux mode stays synchronous `_transfer()`.

## Hardware Interaction

### IRQ/MDAT pin
The ADC signals conversion complete via IRQ/MDAT (falling edge, open drain by
default). In DMA mode:
- `attachIRQ(pin, callback)` still registers the GPIO interrupt.
- The ISR calls `_dma.start()` instead of the synchronous `_transfer()`.
- DMA reads ADCDATA (address byte + payload) in the background; the CPU
  returns from the ISR immediately.
- The DMA completion IRQ pushes a `DmaSample` to the ring buffer and re-arms.
- When IRQ_MODE is push-pull active-high the effective edge is RISING; the
  library keeps the existing FALLING attach behavior (low-priority deferred
  item, unchanged by this change).

### MCLK pin
MCLK via `tone(pin, 4.9152 MHz)` or a hardware timer. The MCLK timer channel
must not conflict with the DMA channel; platform engines perform conflict
checks in `enableDMA()` and return `false` on conflict. For internal clock mode
(`clk_sel::INTERN`) the MCLK pin is unused and no timer conflict is possible.

### CRC (EN_CRCCOM) — DEFERRED
Timing analysis (from PLAN.md, unchanged):
- Data Ready pulse width = OSR - 16 DMCLK periods.
- At OSR=32, 16-bit CRC format: transfer ~2.4 us, pulse ~3.25 us → <1 us margin.
- At OSR=32, 32-bit CRC format: transfer ~4 us → negative margin (misses next
  conversion).
- Software CRC validation adds ~2 us in ISR context, increasing timing
  pressure at low OSR. Recommended only at OSR >= 256; add later with:
  `enableCRC` parameter, 2 extra transfer bytes, `DmaSample::crcOk` field.

## Data Flow

```
Non-DMA:                              DMA:
 ADC IRQ                                 ADC IRQ
   → IRQ_handler()                         → IRQ_handler()
     → _transfer() (blocking)                 → _dma.start() (non-blocking)
     → _result[channel] = value                CPU returns immediately
     → _dataAvailable = true                   → DMA complete IRQ
                                                  → ring.push(DmaSample)
                                                  → _dma.start() (re-arm)
```

## Core types

### DmaSample
```cpp
struct DmaSample {
    uint8_t channel;    // ID from ADCDATA[31:28] (ID_SGNEXT_DATA)
    int32_t value;      // sign-extended 25-bit ADC value
    uint32_t timestamp; // micros() at DMA completion
};
```

### DmaRingBuffer (SPSC)
Lock-free single-producer (DMA completion ISR) / single-consumer (user
context); volatile head/count, no mutex in ISR.

```cpp
class DmaRingBuffer {
    DmaSample* _buf;    // user-provided
    size_t _cap;
    volatile size_t _head;
    volatile size_t _count;
public:
    void setup(DmaSample* buf, size_t cap);
    bool push(DmaSample s);      // ISR context
    bool pop(DmaSample& s);      // user context
    size_t available() const;
    void clear();
};
```

`push()` drops the sample (returns `false`) when the ring is full — overflow is
reported, never blocked.

## Library API

```cpp
bool enableDMA(DmaSample* buffer, size_t capacity,
               uint8_t dmaChannel = 0);
void disableDMA();
bool isDMAEnabled() const;
size_t samplesAvailable();     // ring count (0 if !_dmaEnabled)
bool readSample(DmaSample& s); // pop oldest (false if empty)
```

- `enableDMA()` returns `false` if `capacity < 1`, if the platform has no
  DMA-capable SPI engine, if the DMA channel/timer conflicts with MCLK, or if
  the platform engine fails to initialize.
- `disableDMA()` releases the DMA channel, disables peripheral clocks, and
  stops the MCLK tone (`noTone(_pinMCLK)`), matching destructor behavior.

## Platform engine (union, no vtable, no heap)

```cpp
union {
#ifdef ARDUINO_ARCH_ESP32
    struct { spi_device_handle_t handle; spi_transaction_t tx; } esp32;
#endif
#ifdef ARDUINO_ARCH_SAMD
    struct { uint8_t dmacChannel; } samd;
#endif
#ifdef ARDUINO_ARCH_STM32
    struct { } stm32;
#endif
    uint8_t _dmaPad[128];
} _dma;

DmaRingBuffer _dmaRing;
DmaSample*    _dmaRingBuf;
size_t        _dmaRingCap;
bool          _dmaEnabled;
```

Per-platform engines (implemented inside `#ifdef ARDUINO_ARCH_*`):

- **ESP32**: `spi_device_transmit()` via ESP-IDF `spi_master` driver; separate
  device handle (`spi_bus_add_device`) on the Arduino SPI bus host (SPI2_HOST
  by default, selectable via `dmaChannel`); GPIO IRQ on DR pin triggers
  `_dma.start()`; MCLK via LEDC PWM — must not conflict with SPI DMA.
- **SAMD21/SAMD51**: SERCOM SPI + DMAC, RX-buffer-enable trigger; DR pin GPIO
  ISR sets CHCTRLA.ENABLE; MCLK via TCC0/TCC1 — document pad conflicts in
  `enableDMA()`.
- **STM32**: `HAL_SPI_TransmitReceive_DMA()` with `hdmarx`/`hdmatx`; DR pin
  EXTI ISR calls it; MCLK via TIM1/TIM8 PWM — verify stream conflicts.
- **nRF52**: SPIM EasyDMA (DMA built into the peripheral) + GPIOTE event and
  PPI channel to start the transfer on DR; MCLK via TIMER+PPI; PPI channel
  must not conflict with `dmaChannel`.
- **Teensy (IMXRT)**: LPSPI + eDMA via `DMAChannel`; GPIO ISR starts transfer;
  MCLK via FlexPWM; eDMA channel must not be shared.
- **AVR / ESP8266 / native**: no engine — `enableDMA()` returns `false`;
  everything else compiles and behaves unchanged.

The staged transfer is 4 bytes of ADCDATA (ID_SGNEXT_DATA), decoded with the
same `ID_SGNEXT_DATA` semantics as `readADCDATA()` (strip CH_ID nibble,
sign-extend from bit 27).

## analogRead() DMA path (scan mode)

```cpp
int32_t analogRead(Mux chan) {
    if (_scan.channel.raw == 0)
        return _analogReadMux(chan);  // synchronous (unchanged)

    if (_dmaEnabled) {
        DmaSample s;
        if (popSamplesUntil(chan, s))  // FIFO order preserved
            return s.value;
        return MCP3x6x_TIMEOUT;        // empty ring, timed out
    }
    // legacy _result[] cache (unchanged)
    ...
}
```
Pops samples in FIFO order until a sample of the requested channel arrives
(samples of other channels are consumed along the way — FIFO order preserved,
nothing reordered). Honors `_readTimeout` (0 = wait until data is present).

## Timing constraints

```
MCLK = 4.9152 MHz; DMCLK = MCLK / PRE; conversion = OSR / DMCLK
At OSR=256, PRE=1: ~52 us per channel; 8-channel scan ≈ 416 us per cycle.
DMA must complete one ADCDATA transfer (4 bytes; 6 with future CRC) before the
next conversion finished. At 10 MHz SPI: 4 x 0.4 us = 1.6 us — well within
the window. Ring depth recommendation: >= 2 x enabled channels.
```

## Constraints

1. ISR safety: `push()` from DMA ISR, `pop()` from user context — SPSC only.
2. No heap: ring buffer memory = user-provided; engine = union member.
3. Cache coherency: Cortex-M7 / ESP32 — DMA buffers must live in DMA-capable,
   non-cached memory or be flushed after DMA completion (engine-level detail).
4. DMA channel conflict: `enableDMA()` accepts a channel selector;
   `disableDMA()` releases it; MCLK timer channel must not conflict.
5. Power: `disableDMA()` shuts down DMA channels/peripheral clocks and the
   MCLK tone.
6. User buffer sizing: recommend `capacity >= 2 * enabled channels`; reject
   `capacity < 1`.

## Test Strategy

- Steps 1–3 (core: DmaSample/ring/lifecycle/analogRead path) verified by native
  CI with ArduinoFake mocks (`pio test -e native_ci …`).
- Steps 4–8 (platform engines) verified by compile matrix across the
  representative benchmarked environments (AVR/ESP8266: engine compiled out,
  `enableDMA()` rejects; ESP32/SAMD21/SAMD51/STM32/nRF52/Teensy: engine
  compiles).
- cppcheck clean on the single header.

## Migration Plan / Versioning

- Ships inside release 0.2.0 per project decision: no version bump; CHANGELOG
  gains DMA features under the existing `[0.2.0]` entry as *Added*; README gets
  a DMA section; keywords.txt gains the new API names.
- PLAN.md deleted after content migration into this document.
