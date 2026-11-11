# Proposal

## Why

The Arduino_MCP3x6x_Library has never been verified against real MCP3x6x hardware. There is no confirmed evidence that the SPI transactions it emits (address/command byte, data bytes, CS framing, clock mode) match what an MCP3464-family 16-bit 4-channel ADC expects on an ATmega328P (Arduino Nano). Before the library can be trusted on real hardware, its SPI behavior must be observed on an oscilloscope and compared byte-for-byte against the implementation.

## What Changes

- Add a new OpenSpec capability `hardware` (specs/hardware/spec.md) specifying the observable SPI-level behavior the library SHALL exhibit when driving an MCP3464-family ADC on an ATmega328P.
- No changes to library source code, examples, tests, or PCB files. This change is verification-only: build/upload an existing or minimal smoke sketch, capture SPI on an oscilloscope, and compare decoded bytes against the spec.
- Add operational tasks for scope capture (MISO/MOSI/CS/CLK), SCPI-based capture via scpi-lab-automator (with fixes if needed), per-requirement verification, and result documentation.

## Stakeholder needs

- Library maintainer: confidence that `begin()`, register reads/writes, and `analogRead()` produce correct SPI transactions on real ATmega328P hardware.
- Hardware debugger: a repeatable way to correlate oscilloscope SPI captures with library calls so mismatches (wrong command byte, CS glitches, wrong clock mode) can be isolated.
- Future contributors: a documented baseline of expected SPI bytes and timing for the MCP3464 family on the Nano.

## Capabilities

### New Capabilities

- `hardware`: Observable SPI behavior of the MCP3x6x library on ATmega328P/Arduino Nano driving a 16-bit 4-channel MCP3464-family ADC, verified via oscilloscope capture of MISO/MOSI/CS/CLK. Creates `specs/hardware/spec.md`.

### Modified Capabilities

- None. No existing capability under `openspec/specs/` exists yet (spec inventory is empty), and no library behavior is being changed.

## In scope

- Verify `begin()` on `MCP3462` (`MCP3x6x<16, 4>`, matching the 16-bit 4-channel MCP3464 family): SPI reset command, chip-ID validation read (RESERVED3), default configuration writes.
- Verify register read/write framing: CS LOW for the full address+data transfer, CS HIGH between transactions, `beginTransaction`/`endTransaction` bracketing.
- Verify MOSI command bytes against library constants (e.g. CMD_RESET 0x78, SREAD|ADR_RESERVED3 0x79, IWRITE|ADR_CONFIG0 0x86, IWRITE|ADR_MUX 0x9A, SREAD|ADR_ADCDATA 0x81, CMD_CONVERSION 0x68).
- Verify MISO bytes: status byte after the first MOSI byte; ADCDATA read returns 2 data bytes (16-bit default format) in big-endian wire order.
- Verify `analogRead(CH0..CH3)` polling completes with floating inputs (no driven signal) and prints raw values to Serial at 115200 baud for correlation.
- Build/upload via PlatformIO `ATMEGA328P` env (`board = nanoatmega328`) with default SPI pins CS=10, MOSI=11, MISO=12, SCK=13.
- Attempt SCPI-controlled oscilloscope capture/export; fix scpi-lab-automator only if it blocks capture (out-of-repo skill, documented separately).

## Out of scope (non-goals)

- No modifications to `src/`, `test/`, `examples/`, or `pcb/`.
- No IRQ connection, no MCLK (MCP internal oscillator used), no DMA (not supported on AVR).
- No driven analog inputs; all channels float (readings are noise/undefined — acceptable).
- No voltage-accuracy or calibration validation (no known reference voltage).
- No library API changes, bug fixes, or new features.
- No long-term logging or CI integration of scope captures.

## Impact

- Code: none (read-only inspection of `src/Arduino_MCP3x6x_Library.hpp`).
- Planning: new capability `hardware` with delta spec under `openspec/changes/mcp3464-spi-debug-arduino-nano/specs/hardware/spec.md`.
- External: oscilloscope time, SCPI tooling (scpi-lab-automator) possibly requiring fixes outside this repo.

## Risks and trade-offs

- [Risk] Floating inputs return undefined noise; a "successful" read only proves the transaction completed, not that data is meaningful. → Mitigation: accept noise as expected; scope decode of bytes is the pass criterion, not value magnitude.
- [Risk] `begin()` may fail chip-ID validation if the installed part is not a 16-bit/4-ch device or returns a different ID nibble. → Mitigation: capture the actual MISO ID nibble on scope and adjust the template type / expected ID in the spec; record as a deviation.
- [Risk] Default clock selection (config0 = 0xC0 → internal clock with output) with no MCLK attached may or may not convert correctly. → Mitigation: observe conversions on scope; if they stall, open a question about `clk_sel`.
- [Risk] 4 MHz SPI on a 16 MHz AVR may exceed the oscilloscope's reliable decode bandwidth. → Mitigation: confirm scope sample rate/decode capability before relying on automated decode.
- [Trade-off] Verification uses `MCP3462` (`<16,4>`) as the test vehicle even though the board holds an MCP3464-family part; this matches 16-bit/4-channel and expects chip-ID nibble 0x9. If the part enumerates as 8-channel (`<16,8>` → ID 0xB), the test vehicle type changes.

## Open questions

- Exact MCP variant on the board (MCP3464R or sibling): what chip-ID nibble does RESERVED3 return? Confirmed only after first capture.
- Does the default `clk_sel` (internal clock, clock output) convert correctly with no MCLK attached, or is `clk_sel::INTERN` required?
- Which SCPI command set controls the connected oscilloscope, and does scpi-lab-automator support capture+export of a CS-triggered SPI acquisition? Manual capture is the fallback.
- Should `analogRead()` use a finite read timeout instead of the default indefinite blocking, given floating inputs and no DR pin?
