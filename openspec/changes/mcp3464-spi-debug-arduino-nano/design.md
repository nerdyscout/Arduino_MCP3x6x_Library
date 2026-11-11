# Design

## Context

See `proposal.md` - Why and What Changes (do not restate). Current state:
- The library (`src/Arduino_MCP3x6x_Library.hpp`, header-only template) is untested on real MCP3x6x hardware.
- No OpenSpec specs exist yet; the specs inventory is empty, so `hardware` is a new capability.
- Target: ATmega328P (Arduino Nano), PlatformIO env `ATMEGA328P` (`board = nanoatmega328`), default SPI pins CS=10/MOSI=11/MISO=12/SCK=13.
- MCP part: 16-bit, 4-channel (MCP3464 family). Test vehicle type: `MCP3462` = `MCP3x6x<16, 4>` (16-bit, 4 channels), which expects chip-ID nibble 0x9.
- No IRQ connected (polling mode), no MCLK (MCP internal oscillator), all analog inputs floating.
- Oscilloscope on MISO/MOSI/CS/CLK; SCPI intended for scope control/capture; scpi-lab-automator likely needs fixes.

## Goals / Non-Goals

**Goals:**
- Observe and record the actual SPI bytes and CS/clock behavior the library emits on real hardware.
- Compare decoded scope traces byte-for-byte against the spec requirements (REQ-001…REQ-010).
- Produce a pass/fail verdict per requirement and document deviations.

**Non-Goals:**
- No library source, example, test, or PCB changes.
- No IRQ, MCLK, DMA, calibration, or voltage-accuracy validation.
- No driven analog signals; inputs remain floating.

## Decisions

**D1 - Test vehicle type: `MCP3462` (`MCP3x6x<16, 4>`).**
Rationale: matches the confirmed 16-bit, 4-channel MCP3464 family hardware; chip-ID validation expects nibble 0x9 (see `_validateChipID()`). Alternative `MCP3464` (`<16, 8>`) expects nibble 0xB and is only chosen if the captured ID nibble is 0xB (open question OQ-1).

**D2 - Polling `analogRead()` in mux mode, not IRQ-driven scan.**
Rationale: no IRQ is connected (grilling decision). `begin()` → per-channel `analogRead(CH0..CH3)` → Serial print. Avoids `attachIRQ()`/`attachMCLK()` entirely.

**D3 - Default SPI pins and default `SPISettings()`.**
Rationale: wiring uses library defaults (SS/MOSI/MISO/SCK). `SPISettings()` default on AVR is 4 MHz, MSBFIRST, MODE0, matching REQ-001.

**D4 - Scope trigger: CS falling edge, single-sequence, 4 channels.**
Rationale: every library transaction is bracketed by CS; triggering on CS falling edge captures complete address+data transactions without continuous acquisition.

**D5 - SCPI capture attempted first; manual capture is the fallback.**
Rationale: scpi-lab-automator likely needs fixes (grilling decision). If SCPI capture fails, the engineer records scope screenshots/traces manually and logs the SCPI gap as an open item.

**D6 - No library code changes in this change.**
Rationale: this change is verification only; any library fix discovered during debugging is a separate change with its own grilling and spec.

## SPI transaction flow (from `_transfer`, `src/Arduino_MCP3x6x_Library.hpp`)

Every library register access funnels through `_transfer(uint8_t* data, uint8_t addr, size_t size)`:

1. `_spi->beginTransaction(_spiSettings)` — applies clock/bit order/mode.
2. `digitalWrite(_pinCS, LOW)` — assert CS.
3. `_status.raw = _spi->transfer(addr)` — send command|address byte on MOSI; the status byte comes back on MISO into `_status.raw`.
4. `if (size) _spi->transfer(data, size)` — transfer register data bytes (write) or clock 0x00 while reading data on MISO (read).
5. `digitalWrite(_pinCS, HIGH)` — deassert CS only after the last byte.
6. `_spi->endTransaction()` — release the bus.

CS is therefore LOW for the full address+data transfer and HIGH between transactions (REQ-001, REQ-003, REQ-004).

## Default SPI settings

The constructor defaults `SPISettings theSPISettings = SPISettings()`. On AVR, `SPISettings()` = 4,000,000 Hz, MSBFIRST, SPI_MODE0. `begin()` on AVR calls `pinMode(CS, OUTPUT)`, `digitalWrite(CS, HIGH)`, `_spi->begin()`; each transaction then re-applies the settings via `beginTransaction`. Observable on scope: idle CS HIGH, 4 MHz CLK during transactions, MSB first, mode 0 (CPOL=0, CPHA=0).

## Chip-ID validation in `begin()`

1. `reset()` → fast command CMD_RESET (MOSI 0x78), 1 byte + status byte.
2. `_validateChipID()` → SREAD|ADR_RESERVED3 (MOSI 0x79); `hw_id = status_byte & 0x0F`.
3. For `MCP3x6x<16, 4>`: expected_id = 0x9; `begin()` returns false if `hw_id != 0x9`, before any configuration writes.
4. If valid: `_detectVariant()` reads RESERVED2 and auto-configures VREF.

## Concrete SPI bytes to verify on scope

| Library call | MOSI byte 1 | MOSI data byte(s) | MISO bytes |
|---|---|---|---|
| `reset()` | 0x78 (CMD_RESET) | — | status |
| chip-ID read (`_validateChipID`) | 0x79 (SREAD\|ADR_RESERVED3) | — | status; low nibble = ID |
| `config0()` write | 0x86 (IWRITE\|ADR_CONFIG0) | 0xC0 (default) | status |
| `analogRead(CH0)` MUX write | 0x9A (IWRITE\|ADR_MUX) | 0x08 (CH0+/AGND-) | status |
| `conversion()` | 0x68 (CMD_CONVERSION) | — | status |
| `readADCDATA()` | 0x81 (SREAD\|ADR_ADCDATA) | — | status + 2 data bytes (16-bit default, big-endian wire order) |

`_reverse_array` swaps the 2 ADCDATA bytes internally after transfer; the wire order is big-endian.

## Interfaces and data flow

- Library → MCP3464: SPI MOSI command|address + data; CS controlled by library on pin 10.
- MCP3464 → Library: SPI MISO status byte (first byte), then data bytes.
- Library → engineer: Serial at 115200 baud (`begin()` result, per-channel raw values) for correlation with scope captures.
- Scope → SCPI: single-sequence acquisition triggered on CS falling edge; trace exported for offline decode.
- SCPI tool: scpi-lab-automator (out-of-repo skill); fixes tracked outside this change if needed.

## File changes

- Repository code: none (read-only).
- `openspec/changes/mcp3464-spi-debug-arduino-nano/`: proposal.md, specs/hardware/spec.md, design.md, tasks.md, .openspec.yaml (this change).
- Test vehicle (engineer action, not created here): minimal smoke sketch under `examples/` using `MCP3462 mcp;` — `begin()`, then polling `analogRead(CH0..CH3)` in `loop()`, Serial output at 115200 baud; no `attachIRQ`, no `attachMCLK`. Alternatively adapt `MCP3462_Scan_SingleEnded` polling style without IRQ/MCLK.

## SCPI scope capture integration

- Arm single-sequence on CS falling edge (channel: CS/SS).
- Capture MISO, MOSI, CS, CLK simultaneously at a sample rate adequate for 4 MHz SPI.
- Export trace (CSV/WFM per scope SCPI) and decode MOSI command bytes + MISO status/data bytes offline.
- Compare against the byte table above per requirement.
- If scpi-lab-automator cannot control this scope model or export traces, record the required SCPI command set as an open item and capture manually (REQ-009 then verified via manual capture).

## Risks / Trade-offs

- Floating inputs → undefined noise values; pass criterion is correct transaction bytes, not value magnitude (proposal).
- Chip-ID nibble may not be 0x9 if the part differs; capture actual nibble and revisit D1.
- Default `clk_sel` = internal clock with output (config0 0xC0) with no MCLK attached: conversions may stall; observe and revisit clock selection if needed (OQ-2).
- 4 MHz SPI may exceed scope decode bandwidth; confirm before relying on automated decode.
- scpi-lab-automator fixes are out of repo; manual capture fallback preserves progress (D5).

## Open Questions (deferrable, do not change specs or tasks)

- OQ-1: What chip-ID nibble does the installed part return? Confirms `MCP3462` (`<16,4>`, 0x9) vs `MCP3464` (`<16,8>`, 0xB) test-vehicle type.
- OQ-2: Does the default clock setting convert correctly with no MCLK attached, or is `clk_sel::INTERN` required?
- OQ-3: Which SCPI command set controls the connected oscilloscope; does scpi-lab-automator support CS-triggered capture + export?
- OQ-4: Should `analogRead()` use a finite read timeout (default is indefinite blocking)?
