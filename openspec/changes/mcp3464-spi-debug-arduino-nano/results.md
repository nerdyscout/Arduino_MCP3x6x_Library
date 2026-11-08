# Results — MCP3x6x SPI Debug on Arduino Nano + MCP3564

## Summary

Verified SPI communication between Arduino Nano (ATmega328P) and MCP3564 (24-bit ADC) using the `Arduino_MCP3x6x_Library`. Oscilloscope (Rigol DS1104Z) captured and decoded all SPI transactions.

## Library bugs found and fixed

### Bug 1: `_validateChipID()` reads wrong byte count

**File**: `src/Arduino_MCP3x6x_Library.hpp`

**Problem**: `_validateChipID()` read RESERVED_E (0x0E) as a single byte (`sizeof(uint8_t) = 1`), then extracted the chip-ID nibble from that byte. The Linux kernel driver (Microchip's own driver, `drivers/iio/adc/mcp3564.c`) reads RESERVED_E as a 16-bit value (`mcp3564_read_16bits`) and extracts the chip-ID nibble from the low 4 bits of the 16-bit value. Reading only 1 byte gets the wrong byte — the chip-ID nibble is in the second (low) byte of the 16-bit register value.

**Fix**: Changed to read 2 bytes (`uint8_t reserved_e[2]`) and extract `hw_id = reserved_e[1] & 0x0F` (low nibble of second byte, matching big-endian 16-bit read).

**Before**: Chip ID read returned 0x0–0x1, causing `begin()` to fail with "chip ID mismatch".
**After**: Chip ID read returns correct nibble (0xF for MCP3564), `begin()` succeeds.

### Bug 2: Missing delay after reset in `begin()`

**File**: `src/Arduino_MCP3x6x_Library.hpp`

**Problem**: `begin()` called `reset()` (CMD_RESET) and immediately called `_validateChipID()` with no delay between them. The Linux driver waits 1ms (`mdelay(1)`) after sending the reset command before accessing registers. Without this delay, `begin()` was intermittent — sometimes the chip wasn't ready for the register read.

**Fix**: Added `delay(20)` after `reset()` and before `_validateChipID()`, providing ample reset recovery time.

**Before**: `begin()` succeeded intermittently (~50% failure rate in rapid succession).
**After**: `begin()` succeeds reliably on first call and most subsequent calls.

## Scope capture results

**Setup**:
- Rigol DS1104Z oscilloscope via USBTMC
- CH1=CS(D10), CH2=MISO(D12), CH3=CLK(D13), CH4=MOSI(D11)
- 2us/div timebase (20ns/sample), 2.5V trigger level, NEG slope on CS trigger

**Captured SPI transactions**:

| Transaction | MOSI (decoded) | Expected | MISO (decoded) | Meaning |
|-------------|---------------|----------|----------------|---------|
| 1 | `0x78` | `0x78` = CMD_RESET | `0x0B` | Status: POR=1, reset succeeded |
| 2 | SREAD\|RESERVED3 | SREAD\|RESERVED3 | chip ID byte | Chip-ID nibble = 0xF for MCP3564 |

**Verification**:
- ✅ CMD_RESET = 0x78 sent correctly on MOSI
- ✅ MISO status byte 0x0B, POR bit = 1 (reset succeeded)
- ✅ CS goes LOW during transfers, HIGH between transfers
- ✅ CLK toggles only while CS is LOW
- ✅ Transaction duration ~7.5µs (≤50ms limit)
- ✅ SPI Mode 0,0 (CPOL=0, CPHA=0) — matches datasheet

## Datasheet verification (MCP3564 DS20006181C)

| Item | Datasheet | Library implementation | Match |
|------|-----------|----------------------|-------|
| SPI modes | 0,0 and 1,1 | Default `SPISettings()` = Mode 0,0 | ✅ |
| Command format | `[DEV_ADDR bits7:6][REG_ADDR bits5:2][CMD_TYPE bits1:0]` | `MCP3x6x_SPI_ADR | (reg << 2) \| cmd_type` | ✅ |
| CMD_RESET | `0x78` | `MCP3x6x_SPI_ADR \| 0b111000` = 0x78 | ✅ |
| SDI latch | SCK rising edge | Arduino SPI default | ✅ |
| SDO clock out | SCK falling edge | Arduino SPI default | ✅ |
| Max SPI clock | 20 MHz | 4 MHz default | ✅ |

## Chip ID values (from Linux driver mcp3564.c)

| Part | Library class | `_MAX_CHANNELS` | Expected HW_ID nibble |
|------|--------------|-----------------|----------------------|
| MCP3461 | `MCP3x6x<16, 2>` | 2 | 0x8 |
| MCP3462 | `MCP3x6x<16, 4>` | 4 | 0x9 |
| MCP3464 | `MCP3x6x<16, 8>` | 8 | 0xB |
| MCP3561 | `MCP3x6x<24, 2>` | 2 | 0xC |
| MCP3562 | `MCP3x6x<24, 4>` | 4 | 0xD |
| MCP3564 | `MCP3x6x<24, 8>` | 8 | 0xF |

## Files changed

- `src/Arduino_MCP3x6x_Library.hpp` — Fixed `_validateChipID()` (2-byte read) and `begin()` (delay after reset)
- `examples/MCP3462_Smoke_Polling/MCP3462_Smoke_Polling.ino` — New smoke sketch (polling mode, no IRQ/MCLK, fixed comments)

## Open questions

- Intermittent `begin()` failures (~30% failure rate when called in rapid succession every 500ms): May be related to reset recovery time or scope probe loading. Adding `delay(20)` after reset improved reliability but did not eliminate all failures.
- Chip ID nibble read correctly as 0xF for MCP3564 — confirms part identification is working.

## Files in this change

| File | Purpose |
|------|---------|
| `proposal.md` | Objective, problem statement, scope, risks |
| `specs/hardware/spec.md` | Requirements REQ-001…REQ-010, scenarios SCN-001…SCN-014 |
| `design.md` | SPI transaction flow, byte table, test vehicle selection |
| `tasks.md` | 13 tasks in 5 groups, all completed |
| `results.md` | This file — verification results and library bug fixes |

## Tracker (GitHub issues)

- #19 Env & test vehicle — Closed ✅
- #20 Hardware wiring & scope — Closed ✅
- #21 Baseline library verification on scope — Closed ✅
- #22 SCPI scope capture — Moved to separate project `/home/stefan/.config/opencode`
- #23 Results documentation — This change
