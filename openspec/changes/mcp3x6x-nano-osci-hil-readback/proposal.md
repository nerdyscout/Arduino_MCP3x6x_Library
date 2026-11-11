# Proposal

## Why

The prior change `mcp3464-spi-debug-arduino-nano/` established the `hardware` capability: it specified that the MCP3x6x library must drive correct SPI transactions on an ATmega328P/Arduino Nano driving a 16-bit 4-channel MCP3464-family ADC, verified via oscilloscope capture of MISO/MOSI/CS/CLK. That change was manual: build/upload a smoke sketch, capture on a scope, and compare bytes by eye.

This change extends it into automated orchestration: `pio upload` of an extended `test_config.cpp` hardware build, then SPI decode readback from an auto-detected Rigol oscilloscope over USBTMC via `decoder.py`, comparing decoded instrument-side SPI events byte-for-byte against the expected MOSI bytes emitted by `test_config.cpp`. The goal is a repeatable, scripted hardware-in-the-loop (HIL) readback loop that exits 0 when the scope's decoded SPI events match the library's actual transactions.

## What Changes

- Extend `test/test_MCP3x6x/test_config.cpp` with a hardware-only path (`#ifndef PIO_NATIVE_TESTING`) providing `setup()`/`loop()` that runs the smoke sequence (`begin()`, `unlock()`, register writes config0/config1/config2/config3/irq/mux, then polling `analogRead(CH0..CH3)`) with Serial diagnostics at 115200 baud, mirroring `test_runner.cpp`'s hardware mode. Native mock tests under `#ifdef PIO_NATIVE_TESTING` stay unchanged.
- Add a shell orchestration script `scripts/osci_hil_readback.sh` that:
  1. `pio run -e ATMEGA328P && pio upload -e ATMEGA328P` (upload the `test_config.cpp` hardware build)
  2. Auto-detects the connected Rigol over USBTMC (decoder.py resource auto-detect; `--resource` override via env var)
  3. `decoder.py --protocol spi --decoder 1 --mode configure --json --sclk CHAN3 --mosi CHAN4 --miso CHAN2 --cs CHAN1 --width 8 --edge rise --polarity pos --endian msb --select cs`
  4. `decoder.py --protocol spi --decoder 1 --mode readback --json --etable on`
  5. Parses JSON and compares decoded SPI events against expected bytes (CMD_RESET 0x78, SREAD|RESERVED3 0x79, IWRITE|CONFIG0 0x86 + 0xC0, IWRITE|CONFIG1 0x88, IWRITE|CONFIG2 0x8A, IWRITE|CONFIG3 0x8C, IWRITE|IRQ 0x8E, IWRITE|MUX 0x9A + 0x08, CMD_CONVERSION 0x68, SREAD|ADCDATA 0x81 + status + 2 data bytes)
  6. Propagates exit codes; `--debug` passthrough
- Add delta requirements under the `hardware` capability: `openspec/changes/mcp3x6x-nano-osci-hil-readback/specs/hardware/spec.md` (REQ-011…REQ-017).

No library source code (`src/`) is modified. This change is verification/orchestration only.

## Stakeholder needs

- Library maintainer: a repeatable, automated HIL readback that confirms `begin()`, register writes, and `analogRead()` produce the expected SPI bytes on real ATmega328P hardware, without manual scope interpretation each time.
- Hardware debugger: correlation of instrument-side decoded SPI events (from the Rigol's own `:ETABle<n>:DATA?`) with `test_config.cpp`'s expected MOSI bytes, so command-byte mismatches are flagged automatically.
- CI-adjacent workflow: a scripted go/no-go command that exits 0 end-to-end and propagates failures, suitable for local validation before committing hardware-related changes.

## Capabilities

### New Capabilities

- None.

### Modified Capabilities

- `hardware`: Extended with automated orchestration requirements — test_config.cpp hardware build/upload on `ATMEGA328P`, Rigol USBTMC auto-detection, decoder.py configure+readback SPI decode, expected-byte comparison, and script exit-code propagation. The `hardware` capability was created by `mcp3464-spi-debug-arduino-nano/`; this change adds to `specs/hardware/spec.md` as a delta.

## In scope

- `test_config.cpp` hardware path builds and uploads via PlatformIO `ATMEGA328P` env (`board = nanoatmega328`, default SPI pins CS=10/MOSI=11/MISO=12/SCK=13, 4 MHz, MODE0).
- Hardware smoke sequence executes on the Nano: `begin()`, `unlock()`, register writes config0/config1/config2/config3/irq/mux, then polling `analogRead(CH0..CH3)`, with Serial output at 115200 baud.
- Oscilloscope auto-detected over USBTMC when exactly one VISA resource is present; `--resource` override via env var (`OSCI_RESOURCE` / decoder.py `--resource`).
- `decoder.py --mode configure` programs the scope's SPI decoder 1 on the confirmed channel mapping (CS=CHAN1, SCLK=CHAN3, MISO=CHAN2, MOSI=CHAN4, width 8, edge rise, polarity pos, endian msb, select cs).
- `decoder.py --mode readback` queries `:ETABle<1>:DATA?` and returns decoded SPI events as JSON.
- Orchestration script `scripts/osci_hil_readback.sh` runs the full flow end-to-end, compares decoded events to expected bytes, propagates exit codes, and passes `--debug` through.
- Native tests still pass (no regression): `test_config.cpp` mock tests under `#ifdef PIO_NATIVE_TESTING` unchanged.

## Out of scope (non-goals)

- No modifications to `src/` library code; any bug found is a separate change.
- Capture mode (`--mode capture`) is deferred; implemented only once readback is OK (grilling decision).
- No driven analog inputs; channels float (readings are noise/undefined — acceptable).
- No voltage-accuracy, calibration, IRQ, MCLK, or DMA validation.
- No library API changes, new features, or long-term logging/CI integration of scope captures.
- No modifications to `test_runner.cpp` or other test files.

## Impact

- Code: extension of `test/test_MCP3x6x/test_config.cpp` (hardware path only) + new `scripts/osci_hil_readback.sh`.
- Planning: delta requirements REQ-011…REQ-017 under the `hardware` capability.
- External: Rigol DS1000Z-family scope over USBTMC, PyVISA + `decoder.py` venv; `pio` toolchain for `ATMEGA328P`.

## Risks and trade-offs

- [Risk] Floating inputs return undefined noise; pass criterion is correct decoded SPI bytes, not value magnitude.
- [Risk] `begin()` chip-ID validation may fail if the installed part is not a 16-bit/4-ch device (expected ID nibble 0x9 for MCP3462/`<16,4>`). Capture the actual nibble and adjust the test vehicle type if needed.
- [Risk] USBTMC auto-detect fails when multiple VISA resources are present; `--resource` env override is the mitigation.
- [Risk] `decoder.py` re-execs into its own `.venv`; the venv must exist at the decoder script's location (exit 3 if missing).
- [Risk] 4 MHz SPI decode reliability on the Rigol; confirm sample/decode bandwidth before relying on automated readback.
- [Trade-off] Readback-first preference per grilling: capture mode is deferred to a follow-up change.
- [Trade-off] Expected bytes recorded in this spec are MCP346x-family (16-bit) bytes; if the hardware build uses a 24-bit MCP356x variant, the byte table must be reconciled (open question).

## Open questions

- OQ-1: Which `TEST_ADC_TYPE` does the hardware build of `test_config.cpp` use? Expected bytes (0x78 reset, 0x86 config0, etc.) are MCP346x-family 16-bit; confirm the hardware test vehicle type (MCP3464 family, 16-bit, 4-ch) vs the MCP3564 used in `test_runner.cpp` hardware mode.
- OQ-2: What chip-ID nibble does the installed part return via RESERVED3 (expected 0x9 for 16-bit 4-ch)? Confirms test-vehicle type.
- OQ-3: Exact env var name for the `--resource` override in `osci_hil_readback.sh` (candidate: `OSCI_RESOURCE`).
- OQ-4: Does `test_config.cpp` hardware `setup()`/`loop()` need a finite read timeout for `analogRead()` on floating inputs, or is the default blocking acceptable?
- OQ-5: Rigol USBTMC VISA resource string format observed on the dev machine (auto-detect should handle it, but confirm).
