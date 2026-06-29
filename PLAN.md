# MCP3x6x Library — Architecture Plan

## Overview

Arduino library for Microchip MCP346xR (16-bit) and MCP356xR (24-bit) ADCs.
Header-only C++17 template library. SPI communication, supports single-ended
mux mode and scan mode.

## Key Design Decisions

### No Stream Inheritance

The library **no longer extends `Stream`**. This was removed because:

- The `Stream` vtable added overhead (~8 bytes + pointer per instance)
- `Stream::read()` returns `int` (16-bit on AVR), requiring a `static_assert`
  that prevented 24-bit ADC usage on AVR via the Stream API
- The `write()` method was always a no-op (ADC has no Stream output)
- `peek()` and `flush()` were trivial wrappers with no real value
- The `read()` name conflicted with `read(ConfigX&)` register read overloads

Replaced with:
- `samplesAvailable()` — returns number of samples in ring buffer (0 without DMA)
- `readSample(DmaSample&)` — non-blocking pop from ring buffer
- `analogRead()` — blocks until sample available (unchanged behavior)

### Single Header

All library code lives in `src/Arduino_MCP3x6x_Library.hpp`. Thin C wrapper
in `src/MCP3x6x.h`.

### Template Architecture

```cpp
template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
class MCP3x6x { ... };
```

Type aliases for concrete devices:
```
MCP3461 = MCP3x6x<16, 2>    16-bit, 1-channel (2 mux inputs)
MCP3462 = MCP3x6x<16, 4>    16-bit, 2-channel
MCP3464 = MCP3x6x<16, 8>    16-bit, 4-channel
MCP3561 = MCP3x6x<24, 2>    24-bit, 1-channel
MCP3562 = MCP3x6x<24, 4>    24-bit, 2-channel
MCP3564 = MCP3x6x<24, 8>    24-bit, 4-channel
MCP3465 = MCP3x6x<16, 2>    same as MCP3461 (UQFN variant)
MCP3565 = MCP3x6x<24, 2>    same as MCP3561 (UQFN variant)
```

## Modes of Operation

### Mux Mode (single channel)

- Select channel via `_mux` register
- `conversion()` triggers one-shot conversion
- `analogRead()` busy-waits for DR flag, returns value

### Scan Mode (multi-channel)

- `enableScanChannel()` builds scan list in `_scan` register
- `startScan()` configures IRQ mode, data format, wakes ADC
- IRQ pin triggers `IRQ_handler()` ISR
- ISR reads `ADCDATA` via SPI, caches result in `_result` union
- `analogRead()` in scan mode returns cached values (no SPI traffic)

### Continuous Mode

- `startContinuous()` / `stopContinuous()` for free-running single-channel
- `analogReadContinuous()` returns cached value

## Data Flow

```
ADC Conversion Complete
  → DR pin goes low
    → IRQ fires (if IRQ mode configured)
      → IRQ_handler() ISR
        → read(Adcdata*) SPI transaction
          → _result.raw[channel] = value
          → _dataAvailable = true
```

## API Surface

### Core
- `begin()` / `end()` — SPI init/deinit
- `attachIRQ(pin, callback)` / `attachMCLK(pin)`
- `configure(...)` — batch-write all registers
- Individual register setters: `config0()`, `config1()`, ..., `crccfg()`

### Data Read
- `analogRead(Mux)` — mux or scan mode (auto-detects)
- `analogReadDifferential(pinP, pinN)` — differential mux
- `analogReadContinuous(Mux)` — continuous mode
- `readADCDATA()` — direct ADCDATA read (returns int32_t)
- `dataAvailable()` — check if IRQ has cached data
- `read(Adcdata*)` — read ADCDATA into struct
- `read(ConfigX&)` — read any config register by reference

### Fast Commands
- `conversion()`, `standby()`, `shutdown()`, `full_shutdown()`, `reset()`

### Status
- `status_dr()`, `status_crccfg()`, `status_por()`
- `isComplete()`, `isContinuous()`, `isDifferential()`

### Configuration
- `setDataFormat()`, `setConversionMode()`, `setAdcMode()`
- `setReference()`, `getReference()`
- `setAveraging()` / `setOversamplingRatio()`
- `setReadTimeout()`, `getReadTimeout()`
- `singleEndedMode()` / `differentialMode()`
- `enableScanChannel()` / `disableScanChannel()`

### Utility
- `toVoltage(int32_t raw)` — raw counts → microvolts
- `readTemperature()` — centi-Celsius
- `getMaxValue()`, `getDeviceAddress()`

## Register Map (Bitfield Unions)

All registers use packed bitfield unions for type-safe access:

| Register | Union | Access |
|---|---|---|
| CONFIG0 | `Config0` | adc, bias, clk, vref_sel, cfg0 |
| CONFIG1 | `Config1` | osr, pre |
| CONFIG2 | `Config2` | boost, gain, az_mux, az_ref |
| CONFIG3 | `Config3` | en_gaincal, en_offcal, en_crccom, crc_format, data_format, conv_mode |
| IRQ | `Irq` | en_stp, en_fastcmd, irq_mode, por_status, crccfg_status, dr_status |
| MUX | `Mux` | vin_minus, vin_plus |
| SCAN | `Scan` | channel bitmap, dly |
| TIMER | `Timer` | 3-byte raw |
| OFFSET | `Offset` | 3-byte raw |
| GAIN | `Gain` | 3-byte raw |
| LOCK | `Lock` | 1-byte raw |
| CRCCFG | `Crccfg` | 2-byte raw (CRC-16) |

## SPI Protocol

- SPI Mode 0,0 (CPOL=0, CPHA=0)
- Big-endian wire format for multi-byte registers
- `_reverse_array()` for endian conversion
- STATUS byte: POR=bit5, CRCCFG=bit4, DR=bit3
- ADCDATA: 2/3/4 bytes depending on resolution and data format

## Platform Support

| Platform | Status | Notes |
|---|---|---|
| ESP32 | Tested | `begin()` uses 4-arg SPI init |
| ESP8266 | Tested | `begin()` uses pin remapping |
| AVR (Uno, Mega) | Tested | Works in mux mode |
| SAMD (M0/M4) | Tested | Uses `pinPeripheral()` for SERCOM |
| STM32 | Tested | `begin()` allocates new SPIClass |
| nRF52 | Pending | Standard Arduino SPI |
| Teensy | Pending | Standard Arduino SPI |

## Build & Test

- PlatformIO with Arduino framework
- `pio run -e <env>` for compilation verification
- `pio test -e native_ci --project-conf platformio_ci.ini` for native unit tests
- Uses ArduinoFake + Unity for host-based testing
- 90+ tests covering all major API paths

## Planned / Unimplemented Features

Not yet implemented (from datasheet):

- **CRC validation** — hardware CRC-16 checksums on ADCDATA reads
- **One-shot scan** — single scan cycle then standby
- **Calibration helpers** — typed offset/gain setters
- **AVDD/VCM voltage conversion** — `readAVDD()` / `readVCM()`
- **Multi-device SPI address** — selectable device address for shared bus
- **DMA transfers** — background ADCDATA reads on 32-bit MCUs
- **Burst/continuous SPI read** — keep CS low for multi-sample reads
- **Sensor fault detection** — burnout current + open/short detection
- **Scan timer API** — user-friendly scan interval configuration

## DMA Support

### Motivation

Currently all SPI transactions, including ADCDATA reads in `IRQ_handler()`, are
synchronous blocking calls inside the ISR. On 32-bit MCUs this wastes CPU cycles
that could otherwise handle higher-priority tasks or sleep. DMA moves the SPI
data movement to a dedicated peripheral, freeing the CPU entirely.

### Scope

DMA applies to **scan mode only**. Mux mode (single-channel one-shot
conversions) always uses the existing synchronous `_transfer()` path.

### Target Platforms

All 32-bit MCUs where the SPI peripheral has DMA support:

| Platform | SPI DMA Mechanism | Interrupt Trigger |
|---|---|---|
| ESP32 | `spi_device_transmit()` with DMA descriptors via ESP-IDF `spi_master` driver | GPIO IRQ (DR pin) or DMA done callback |
| SAMD21/G | SERCOM SPI + DMAC (DMAC channel triggered by SERCOM RX complete) | DMAC complete interrupt |
| STM32 | `HAL_SPI_TransmitReceive_DMA()` with circular mode | SPI RXNE or DMA transfer complete |
| nRF52 | SPIM EasyDMA (built-in, no separate DMA controller) | SPIM END event |
| Teensy (IMXRT) | LPSPI + eDMA | DMA completion or SPI RX FIFO |

### Ring Buffer (User-Provided)

When DMA is active, samples accumulate in a ring buffer backed by a
**user-provided buffer**. The user passes a pointer + capacity to `enableDMA()`.
The library never allocates ring buffer memory.

```cpp
struct DmaSample {
    uint8_t channel;     // channel ID from ADCDATA[31:28]
    int32_t value;       // signed 25-bit ADC value
    uint32_t timestamp;  // micros() at DMA completion
};
```

The ring buffer is an internal lock-free single-producer (DMA ISR) /
single-consumer (user context) ring:

```cpp
class DmaRingBuffer {
    DmaSample* _buffer;  // user-provided
    size_t _capacity;
    volatile size_t _head;
    volatile size_t _count;
public:
    void setup(DmaSample* buf, size_t cap);
    bool push(DmaSample s);      // DMA ISR context
    bool pop(DmaSample& s);      // user context
    size_t available() const;
    void clear();
};
```

### DMA Engine (Embedded Union)

The DMA hardware state is stored in a **platform-union member** inside
`MCP3x6x`. No vtable, no heap. Sized for the largest platform. Each platform
defines its own `DmaState` struct selected by `#ifdef`.

```cpp
class MCP3x6x {
    // ...
#ifdef ARDUINO_ARCH_ESP32
    struct Esp32DmaState { spi_device_handle_t handle; /* ... */ };
#elif defined(ARDUINO_ARCH_SAMD)
    struct SamdDmaState { uint8_t dmacChannel;          /* ... */ };
// ... etc for each platform
#endif

    union {
        Esp32DmaState esp32;
        SamdDmaState samd;
        // one member per supported platform
        uint8_t _dmaStorage[128];  // padding to largest
    } _dma;

    DmaRingBuffer _ring;           // ring buffer controller
    DmaSample* _ringBuf;           // user-provided buffer pointer
    size_t _ringCap;               // user-provided buffer capacity
    bool _dmaEnabled;
};
```

`enableDMA()` activates the appropriate union member and sets up the
peripheral. `disableDMA()` tears it down.

### Data Flow

```
Non-DMA scan mode:                         DMA scan mode:
                                           (user calls enableDMA())
ADC Conversion Complete                    ADC Conversion Complete
  → DR pin goes low                          → DR pin goes low
    → IRQ_handler()                            → IRQ_handler()
      → _transfer() (blocking)                   → _dma.start() (non-blocking)
      → _result[channel] = value                   CPU returns immediately
      → _dataAvailable = true                   SPI runs in background
                                                → DMA complete interrupt
                                                  → _ring.push(sample)
                                                  → _dataAvailable = true
                                                  → _dma.start() (re-arm)
```

### API Surface

```cpp
// Enable DMA for scan mode. buffer/capacity are user-provided ring buffer.
// Returns false if platform doesn't support DMA.
bool enableDMA(DmaSample* buffer, size_t capacity, uint8_t dmaChannel = 0);

// Disable DMA, free hardware resources
void disableDMA();

// Check if DMA is active
bool isDMAEnabled() const;

// Number of samples waiting in ring buffer (0 in non-DMA mode)
size_t samplesAvailable();

// Pop oldest sample from ring buffer. Non-DMA mode returns false.
bool readSample(DmaSample& s);
```

`analogRead()` in scan mode selects the storage path based on `_dmaEnabled`:

```cpp
int32_t analogRead(Mux chan) {
    if (_scan.channel.raw == 0)
        return _analogReadMux(chan);  // synchronous mux path (unchanged)

    // Scan mode
    if (_dmaEnabled) {
        DmaSample s;
        // Busy-wait until a sample for the requested channel arrives
        // (consistent with existing mux-mode blocking behavior)
        while (!readSample(s)) { /* yield or wait */ }
        return s.value;  // channel matching done by DMA hardware
    }

    // Legacy scan path: return from _result[] cache (unchanged)
    for (size_t i = 0; i < 16; i++)
        if (_channelID[i] == chan.raw)
            return _result.raw[i];
    return -1;
}
```

### DmaSample Channel Matching

`readSample()` always pops the **oldest** sample from the ring buffer (FIFO).
The user can match against a specific channel after popping:

```cpp
DmaSample s;
if (mcp.readSample(s) && s.channel == MCP3x6x_CH0) {
    // process CH0 sample
}
```

`analogRead()` does this matching internally — it pops samples in a loop
discarding ones that don't match the requested channel, blocking until the
right one arrives. This guarantees FIFO ordering is preserved: no sample is
ever lost or re-queued.

### Architectural Approach

```
MCP3x6x
  │
  ├─ _transfer()         ← synchronous (always available, default)
  │
  └─ _dma (union member) ← activated by enableDMA()
       │
       ├─ init()         ← platform-specific SPI+DMA setup
       ├─ start()        ← arm DMA for next ADCDATA read
       ├─ callback()     ← DMA complete: push to ring, re-arm
       └─ stop()         ← disarm DMA channels
```

### Platform Implementation Details

#### ESP32

Uses the ESP-IDF `spi_device_transmit()` API with DMA descriptors. Requires
separate SPI device handle (not the Arduino `SPI` class) to access DMA.

```cpp
#ifdef ARDUINO_ARCH_ESP32
  struct Esp32DmaState {
      spi_device_handle_t handle;
      spi_transaction_t    transaction;
  };

  // init: spi_bus_add_device(SPI3_HOST, &devcfg, &handle)
  // start: spi_device_queue_trans(handle, &transaction, portMAX_DELAY)
  // callback: called from spi_transaction completion ISR
```

#### SAMD21/G

Uses SERCOM SPI with the DMAC peripheral. SERCOM RX triggers a DMAC channel.

Key registers:
- `DMAC->Channel[n].CHCTRLA` — channel config
- `DMAC->Channel[n].CHDESCRIPTOR` — BTCTRL, BTCNT, SRCADDR, DSTADDR
- `SERCOMn->SPI.INTFLAG.bit.RXC` — SERCOM RX complete flag

#### STM32

Uses `HAL_SPI_TransmitReceive_DMA()` with:
- `hspi->hdmarx` — DMA handle for RX
- `hspi->hdmatx` — DMA handle for TX
- `HAL_SPI_TxRxCpltCallback()` → ring buffer push

#### nRF52

SPIM peripheral has **EasyDMA** built in — no separate DMA controller needed.

```cpp
#ifdef ARDUINO_ARCH_NRF52
  // SPIM: set TXD.MAXCNT, RXD.MAXCNT, trigger START task
  // RX data written directly to RAM buffer
```

#### Teensy (IMXRT)

Uses LPSPI + eDMA via `DMAChannel`.

```cpp
#ifdef TEENSYDUINO
  DMAChannel _dmaChannel;
  // LPSPI + DMAChannel: trigger on LPSPI RX, circular buffer
```

### Critical Design Constraints

1. **ISR safety**: DMA completion ISR calls `DmaRingBuffer::push()`. User
   context calls `pop()` / `available()`. Must be lock-free
   single-producer/single-consumer (volatile head/count, no mutex in ISR).

2. **No heap**: Ring buffer memory is user-provided. DMA engine is a union
   member (no dynamic allocation).

3. **Power**: `disableDMA()` must shut down DMA channels and peripheral clocks
   properly.

4. **DMA channel conflict**: On limited-channel platforms (SAMD21: 12 channels),
   `enableDMA()` should accept a channel selector parameter. `disableDMA()`
   releases the channel.

5. **Cache coherency**: On Cortex-M7 (Teensy 4.x) and ESP32, DMA buffers must
   reside in non-cacheable memory or be explicitly flushed/invalidated.

### Current API Status (Post-Stream Cleanup)

| Removed | Replacement |
|---|---|
| `Stream::read()` | `analogRead()` / `readSample(DmaSample&)` |
| `Stream::available()` | `samplesAvailable()` |
| `Stream::peek()` | _(no replacement — use `samplesAvailable()` + `readSample()`)_ |
| `Stream::write()` | _(no replacement — was always no-op)_ |
| `Stream::flush()` | _(no replacement — was always no-op)_ |
| `readADCDATA()` | _(removed — use `analogRead()` or `readSample()`)_ |
| `dataAvailable()` | _(removed — use `samplesAvailable() > 0`)_ |

### Implementation Priority

| Step | Description | Testable |
|---|---|---|
| 1. DmaSample + DmaRingBuffer | Ring buffer logic (no DMA hardware) | Native CI |
| 2. API stubs | `enableDMA()`, `disableDMA()`, `samplesAvailable()`, `readSample()` | Native CI |
| 3. analogRead() DMA path | Wire scan-mode analogRead to ring buffer when DMA active | Native CI |
| 4. ESP32 DMA | Full ESP32 _dmaEngine using ESP-IDF | Hardware |
| 5. SAMD DMA | SAMD21/G DMAC + SERCOM | Hardware |
| 6. STM32 DMA | STM32 HAL SPI DMA | Hardware |
| 7. nRF52 DMA | nRF52 SPIM EasyDMA | Hardware |
| 8. Teensy DMA | IMXRT LPSPI + eDMA | Hardware |

Steps 1–3 can be fully tested via native CI (ArduinoFake mocks). Steps 4–8
require hardware-in-the-loop.

## Test Strategy

- **Native (host)**: Unity tests with ArduinoFake mocking SPI/GPIO
- **Hardware**: PlatformIO envs with real boards, `TEST_ADC_TYPE` define
- Key coverage: instance creation, begin(), configure(), register R/W,
  fast commands, analogRead (mux/scan/diff/continuous), IRQ handler,
  data format conversion, status flags, Stream replacement API
