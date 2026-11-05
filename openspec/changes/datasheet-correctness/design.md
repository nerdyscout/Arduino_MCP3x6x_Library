# Design

## Context

See `proposal.md` - Why. The library is a single-header C++17 class template that drives
MCP346xR/MCP356xR ADCs over SPI. Register state is held in unions (`Config0`-`Config3`, `Irq`, `Mux`,
`Scan`, `Timer`, `Offset`, `Gain`, `Lock`, `Crccfg`) that mirror the datasheet register map, and
access goes through fast commands (`MCP3x6x_CMD_*`) and register read/write helpers.

## Goals / Non-Goals

**Goals:**

- Correct Critical + High datasheet mismatches in register semantics, scan sequencing, data
  readback, and voltage conversion.
- Keep behavior testable through datasheet-derived native unit tests.

**Non-Goals:**

- Defer Medium/Low refactors (IRQ polarity, MCLK tone limits, read-only CRCCFG write, MUX default,
  private type visibility, broader error handling).
- Defer multi-byte register read endianness.

## Decisions

### D1: LOCK semantics - `fix/lock-semantics`

Datasheet §8.14 (LOCK register) and §6.6: the LOCK register is the write access password entry code;
writing `0xA5` allows full register map writes, any other value locks the full register map (only
the LOCK register stays writable). At POR and after Full Reset the register map is unlocked
(`LOCK = 0xA5`).

- `MCP3x6x_CFG_LOCK` becomes `0xA5`.
- `lock(uint8_t key = 0x00)` writes the supplied key; the default writes a value that locks.
- `unlock()` delegates to `lock(0xA5)`.
- `configure(...)` unlocks before any register write and writes the supplied Lock value last.

### D2: CONFIG2 field placement - `fix/config2-fields`

Datasheet §8.4: BOOST occupies bits [7:6], GAIN bits [5:3], AZ_MUX bit [2], AZ_REF bit [1], and
RESERVED bit [0] (must be 1).

- `Config2` union field bit widths/positions are corrected; `write(Config2)` forces RESERVED[0] = 1.

### D3: CONFIG0 VREF and boot sequence - `fix/config0-vref`

Datasheet §8.2 (CONFIG0) and §6.2.5 (Table 6-2): VREF_SEL is CONFIG0 bit 7; internal reference is
buffered at 2.4 V by default.

- `begin()` resets the ADC (Full Reset fast command) and probes POR/chip ID only, without writing
  the LOCK register; Full Reset restores LOCK to `0xA5`.

### D4: STATUS byte layout - `fix/status-layout`

Datasheet §6.2.5/§6.2.6 and Figure 6-1: STATUS byte bits [7:6] = 0, DEV_ADDR acknowledge at
[5:3], DR_STATUS at [2], CRCCFG_STATUS at [1], POR_STATUS at [0].

### D5: Scan modes and ADC data semantics - `fix/adc-api-semantics`

Datasheet §8.5 (CONFIG3) and Equation 5-5: CONV_MODE[1:0] values `11` = continuous conversion,
`10` = one-shot conversion (ends in standby).

- `startScan()` now also writes CONV_MODE=11 and triggers a conversion; new `startScanOnce()` writes
  CONV_MODE=10.
- `startContinuous()` / `stopContinuous()` remain valid.
- `analogReadDifferential(pinP, pinN)` maps pinP to VIN+ and pinN to VIN- without range validation.
- `toVoltage(raw)` implements Equation 5-5: code = ((VIN+ - VIN-) / (VREF+ - VREF-)) * 2^(N-1) * GAIN;
  GAIN_033 handled as x3; `getMaxValue()` returns 2^(N-1).
- `readADCDATA()` strips CHID[3:0] from bits [31:28] for ID_SGNEXT_DATA and sign-extends from bit 27;
  `getLastChannel()` exposes the last channel.

### D6: C header removal - `fix/c-header`

`src/MCP3x6x.h` only wraps the class template in `extern "C"` and is invalid; it is deleted and all
examples include `Arduino_MCP3x6x_Library.hpp`.

## Interfaces

- `void lock(uint8_t key = 0x00);`
- `void unlock();`
- `void configure(Config0, Config1, Config2, Config3, Irq, Mux, Scan, Timer, Offset, Gain, Lock,
  Crccfg);`
- `bool begin();`
- `void startScan(enum irq_mode mode = IRQ_OPEN_DRAIN, data_format fmt = data_format::ID_SGNEXT_DATA);`
- `void startScanOnce();`
- `void startContinuous();`
- `void stopContinuous();`
- `int32_t analogReadDifferential(Mux pinP, Mux pinN);`
- `int32_t toVoltage(int32_t raw);`
- `uint32_t getMaxValue();`
- `int32_t readADCDATA();`
- `uint8_t getLastChannel();`

## Data Flow

- Unlock/write sequence: `configure()` -> `unlock()` (LOCK=0xA5) -> conditional register writes ->
  supplied Lock value last.
- Status polling: STATUS byte bits [2]/[1]/[0] for DR/CRCCFG/POR; bits [5:3] for DEV_ADDR ack,
  [7:6] reserved zero.
- ADCDATA read: byte count depends on resolution and data format; channel strip + sign extension for
  ID_SGNEXT_DATA.

## File Changes

- `src/Arduino_MCP3x6x_Library.hpp` - register unions, lock/configure/begin, status, scan, ADC data
  path, voltage conversion.
- `src/MCP3x6x.h` - deleted.
- `examples/*` - include the C++ header; corrected scan/voltage/differential usage.
- `test/test_MCP3x6x/*` - rewrite tests encoding wrong mappings.
- Docs/metadata: README.md, keywords.txt, Doxyfile, library.json, library.properties, CHANGELOG.md.

## Test Strategy

- Datasheet-derived native unit tests via `pio test -e native_ci --project-conf platformio_ci.ini`.
- Compile matrix via `pio run` across representative environments (ESP32, ESP8266, AVR, SAMD,
  STM32, nRF52, Teensy).
- Manual cppcheck command documented in AGENTS.md.
- HIL smoke example exercising LOCK/unlock, CONFIG0 VREF, CONFIG2 gain/boost, STATUS bits, voltage
  conversion; hardware-only, not run in CI.

## Migration Plan / Versioning

- Breaking changes released as 0.2.0; CHANGELOG.md entry; README "Upgrading to 0.2.0" section.

## Risks / Trade-offs

- No hardware available; native tests plus compile matrix only until HIL smoke example is bench
  validated.

## Open Questions

- The `startScanOnce()` CONV_MODE=10 (one-shot then standby) assumption.
- REUSE.toml vs SPDX headers for examples.
