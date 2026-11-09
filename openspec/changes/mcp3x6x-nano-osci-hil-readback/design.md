# Design

## Context

See `proposal.md` - Why and What Changes (do not restate). Current state:
- The `hardware` capability exists (created by `mcp3464-spi-debug-arduino-nano/`, REQ-001…REQ-010) specifying manual oscilloscope verification of MCP3x6x SPI transactions on ATmega328P/Arduino Nano.
- Target board: Arduino Nano = ATmega328P, PlatformIO env `ATMEGA328P` (`board = nanoatmega328`), default SPI pins CS=10, MOSI=11, MISO=12, SCK=13, 4 MHz, MODE0.
- `test/test_MCP3x6x/test_config.cpp` is currently native-test-only (`#ifdef PIO_NATIVE_TESTING`).
- `test/test_MCP3x6x/test_runner.cpp` shows the hardware-mode pattern (`#ifndef PIO_NATIVE_TESTING` `setup()`/`loop()`, Serial at 115200 baud).
- Oscilloscope: Rigol DS1000Z-family over USBTMC; `decoder.py` at `/home/stefan/.config/opencode/skills/scpi/scripts/decoder.py`, verified over USBTMC via PyVISA, re-execs into its own `.venv`.
- Git remote: github.com/nerdyscout/Arduino_MCP3x6x_Library → `gh` CLI.

## Goals / Non-Goals

**Goals:**
- Scripted HIL readback: build/upload `test_config.cpp` hardware build, then decoder.py configure+readback, compare decoded SPI events to expected bytes, exit 0 end-to-end.
- Readback-first: `--mode readback` (`:ETABle<1>:DATA?`) is the primary decode path; capture mode deferred.

**Non-Goals:**
- No `src/` library changes; no `test_runner.cpp` changes; no capture mode in this change.
- No driven analog inputs, IRQ, MCLK, DMA, calibration, or voltage validation.

## Decisions

**D1 - Change is a single intent: automated HIL readback orchestration.**
Rationale: build/upload + Rigol auto-detect + decoder configure + decoder readback + byte comparison + exit propagation form one coherent go/no-go flow. Splitting would lose the end-to-end exit-code contract.

**D2 - Extend the existing `hardware` capability, do not create a new one.**
Rationale: `hardware` already exists (REQ-001…REQ-010). This change adds orchestration requirements REQ-011…REQ-017 as a delta under `specs/hardware/spec.md`.

**D3 - `test_config.cpp` hardware path mirrors `test_runner.cpp` hardware mode.**
Rationale: `test_runner.cpp` already proves the `#ifndef PIO_NATIVE_TESTING` `setup()`/`loop()` + Serial 115200 pattern compiles for AVR hardware. `test_config.cpp` reuses the same structure with its own smoke sequence (begin/unlock/register writes/analogRead polling).

**D4 - Oscilloscope channel mapping (confirmed by user).**
- CS = pin 10 → CHAN1 → `--cs CHAN1`
- CLK/SCK = pin 13 → CHAN3 → `--sclk CHAN3`
- MISO = pin 12 → CHAN2 → `--miso CHAN2`
- MOSI = pin 11 → CHAN4 → `--mosi CHAN4`

**D5 - Readback mode is primary; configure precedes readback.**
Rationale: decoder.py must program `:DECoder<1>` before `:ETABle<1>:DATA?` returns decoded events. Capture mode (`--mode capture`) is deferred to a follow-up change.

**D6 - Exit-code propagation; `--debug` passthrough.**
Rationale: the script is a go/no-go gate; every step's exit code must propagate, and `--debug` must reach decoder.py for SCPI traffic logging.

**D7 - No library source changes in this change.**
Rationale: verification/orchestration only; any library fix is a separate change.

## SPI transaction flow (from `_transfer`, `src/Arduino_MCP3x6x_Library.hpp`)

Every library register access funnels through `_transfer(uint8_t* data, uint8_t addr, size_t size)`:

1. `_spi->beginTransaction(_spiSettings)` — applies 4 MHz, MSBFIRST, MODE0.
2. `digitalWrite(_pinCS, LOW)` — assert CS (pin 10).
3. `_status.raw = _spi->transfer(addr)` — command|address byte on MOSI; status byte returns on MISO.
4. `if (size) _spi->transfer(data, size)` — register data bytes.
5. `digitalWrite(_pinCS, HIGH)` — deassert CS after the last byte.
6. `_spi->endTransaction()` — release the bus.

CS is LOW for the full address+data transfer and HIGH between transactions. Observable on scope: idle CS HIGH, 4 MHz CLK during transactions, MSB first, mode 0 (CPOL=0, CPHA=0).

## Oscilloscope channel mapping table

| Nano pin | Signal | Scope channel | decoder.py flag |
|----------|--------|---------------|-----------------|
| 10 | CS / SS | CHAN1 | `--cs CHAN1` |
| 13 | CLK / SCK | CHAN3 | `--sclk CHAN3` |
| 12 | MISO | CHAN2 | `--miso CHAN2` |
| 11 | MOSI | CHAN4 | `--mosi CHAN4` |

Decoder settings: width 8, edge rise, polarity pos, endian msb, select cs, decoder 1.

## Concrete SPI bytes to verify (decoded MOSI from readback JSON)

| Library call in smoke sequence | MOSI byte 1 | MOSI data byte(s) | MISO bytes |
|---|---|---|---|
| reset() | 0x78 (CMD_RESET) | — | status |
| chip-ID read (_validateChipID) | 0x79 (SREAD|ADR_RESERVED3) | — | status; low nibble = ID |
| config0() write | 0x86 (IWRITE|ADR_CONFIG0) | 0xC0 (default) | status |
| config1() write | 0x88 (IWRITE|ADR_CONFIG1) | — | status |
| config2() write | 0x8A (IWRITE|ADR_CONFIG2) | — | status |
| config3() write | 0x8C (IWRITE|ADR_CONFIG3) | — | status |
| irq() write | 0x8E (IWRITE|ADR_IRQ) | — | status |
| mux() write | 0x9A (IWRITE|ADR_MUX) | 0x08 (CH0+/AGND-) | status |
| conversion() | 0x68 (CMD_CONVERSION) | — | status |
| readADCDATA() | 0x81 (SREAD|ADR_ADCDATA) | — | status + 2 data bytes (big-endian wire order) |

## Interfaces and data flow

1. Orchestration script `scripts/osci_hil_readback.sh`:
   - `pio run -e ATMEGA328P && pio upload -e ATMEGA328P` → uploads `test_config.cpp` hardware build to Nano.
   - Auto-detects Rigol (decoder.py resource auto-detect; `--resource $OSCI_RESOURCE` override).
   - decoder.py configure → decoder.py readback → JSON parse → byte comparison → exit code.
2. Nano `test_config.cpp` hardware `setup()`/`loop()`:
   - SPI MOSI command|address + data on pin 11; CS on pin 10; clock on pin 13; MISO on pin 12.
   - Serial at 115200 baud prints begin() result and per-channel raw values for correlation.
3. Rigol DS1000Z over USBTMC (PyVISA via decoder.py `.venv`):
   - `:DECoder<1>` programmed in configure mode.
   - `:ETABle<1>:DATA?` queried in readback mode; raw decoded-event bytes returned as JSON.

## File changes

- `test/test_MCP3x6x/test_config.cpp` — add `#ifndef PIO_NATIVE_TESTING` hardware `setup()`/`loop()` smoke sequence (engineer implements; specified here).
- `scripts/osci_hil_readback.sh` — new orchestration script (engineer implements; specified here).
- `openspec/changes/mcp3x6x-nano-osci-hil-readback/` — proposal.md, specs/hardware/spec.md, design.md, tasks.md, .openspec.yaml (this change).
- Repository code: no `src/` changes.

## Script flow and exit codes

```
scripts/osci_hil_readback.sh [--debug]
  1. pio run -e ATMEGA328P        (exit nonzero → abort, propagate)
  2. pio upload -e ATMEGA328P     (exit nonzero → abort, propagate)
  3. auto-detect Rigol / $OSCI_RESOURCE --resource (no resource → exit nonzero)
  4. decoder.py ... --mode configure --json ... (exit nonzero → abort, propagate)
  5. decoder.py ... --mode readback --json --etable on (exit nonzero → abort)
  6. parse JSON, compare decoded MOSI vs expected bytes (mismatch → exit nonzero)
  7. exit 0 iff all steps succeeded
```

Exit codes:
- 0: all steps passed, decoded bytes match expected.
- Non-zero: first failing step's exit code propagated (build/upload/configure/readback/comparison).
- decoder.py venv missing → decoder.py exits 3; propagated.
- `--debug` is appended to every decoder.py invocation for SCPI traffic logging to stderr.

## Risks / Trade-offs

- Floating inputs → undefined noise; pass criterion is correct decoded bytes, not value magnitude.
- chip-ID nibble may not be 0x9 if the part differs; capture actual nibble and revisit test-vehicle type (OQ-1/OQ-2).
- USBTMC auto-detect ambiguous with multiple VISA resources; `--resource` env override is the mitigation (OQ-3).
- decoder.py re-execs into its own `.venv`; venv must exist (exit 3 if missing).
- 4 MHz SPI decode reliability on the Rigol; confirm before relying on automated readback.
- Capture mode deferred per readback-first preference; add in a follow-up once readback is OK.

## Open Questions (deferrable, do not change specs or tasks)

- OQ-1: Which `TEST_ADC_TYPE` does the hardware build of `test_config.cpp` use? Expected bytes are MCP346x-family 16-bit; `test_runner.cpp` hardware mode uses MCP3564 (24-bit). Reconcile.
- OQ-2: What chip-ID nibble does the installed part return (expected 0x9 for 16-bit 4-ch)?
- OQ-3: Exact env var name for the `--resource` override (candidate: `OSCI_RESOURCE`).
- OQ-4: Does `analogRead()` on floating inputs need a finite read timeout, or is default blocking acceptable?
- OQ-5: Observed Rigol USBTMC VISA resource string on the dev machine (auto-detect should handle it).
