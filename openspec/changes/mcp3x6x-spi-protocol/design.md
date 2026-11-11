# Design: MCP3x6x SPI Protocol Specification

## Context

The `MCP3x6x` library supports Microchip MCP346x/MCP356x family 16/24-bit delta-sigma ADCs over SPI. The library already implements the SPI transaction flow in `src/Arduino_MCP3x6x_Library.hpp`, but this behavior was not formally specified. This change documents and formalizes the existing implementation.

The current implementation already:
- Declares `status_t _status` as a private member (line 452)
- Writes `_status.raw = _spi->transfer(addr)` in `_transfer()` (line 486)
- Routes `_fastcmd()` through `_transfer()` (line 472)
- Reads `_status` in `status_dr()` (line 1566), `status_crccfg()` (line 1576), `status_por()` (line 1586), and `status()` (line 1780)
- Routes all public SPI methods through `_transfer()` or `_fastcmd()`

No code changes are required — this specification documents and formalizes existing behavior to prevent future regressions.

## Goals and Non-Goals

### Goals
- Formally specify that `_status` exists and is updated by every SPI transaction
- Formally specify that `status_*()` methods read from `_status`
- Document the SPI command byte format, register map, fast commands, and chip-ID values
- Prevent future regressions where `_status` is bypassed or stale

### Non-Goals
- No changes to the `status_t` struct layout
- No changes to the `status_*()` method signatures
- No changes to example sketches or test files
- No changes to library source code

## Technical Approach

### SPI Transaction Flow

All SPI communication flows through a single choke point: `_transfer(uint8_t* data, uint8_t addr, size_t size)`:

```
┌─────────────────────────────────────────────────────────────┐
│                     MCP3x6x SPI Transaction                 │
├─────────────────────────────────────────────────────────────┤
│  1. _spi->beginTransaction(_spiSettings)                    │
│  2. digitalWrite(CS, LOW)                                   │
│  3. _status.raw = _spi->transfer(addr)  ← MOSI: command    │
│                                                      │     MISO: status byte            │
│  4. if (size) _spi->transfer(data, size)  ← MOSI: data     │
│                                                      │     MISO: data bytes             │
│  5. digitalWrite(CS, HIGH)                                  │
│  6. _spi->endTransaction()                                  │
└─────────────────────────────────────────────────────────────┘
```

Every public SPI method (`write()`, `read()`, `analogRead()`, `conversion()`, `reset()`, `lock()`, `unlock()`, etc.) ultimately calls `_transfer()` or `_fastcmd()` (which calls `_transfer()`), ensuring `_status` is always updated.

### Command Byte Format

The command byte sent on MOSI has the following layout:

```
Bit 7   Bit 6   Bit 5   Bit 4   Bit 3   Bit 2   Bit 1   Bit 0
+-------+-------+-------+-------+-------+-------+-------+-------+
| DEV_ADDR[1] | DEV_ADDR[0] | REG_ADDR[3] | REG_ADDR[2] | CMD[1] | CMD[0] |
+-------+-------+-------+-------+-------+-------+-------+-------+
```

- **Device Address (bits 7:6)**: Factory-configured via fuses; default `0b01`
- **Register Address (bits 5:2)**: Register address (0x0–0x0F)
- **Command Type (bits 1:0)**: `00`=fast command, `01`=static read, `10`=incremental write, `11`=incremental read

### Register Map

The library defines register addresses as `MCP3x6x_SPI_ADR | (reg << 2)` where `MCP3x6x_SPI_ADR = 0x40`:

| Register | Address | Size | Library Constant |
|----------|---------|------|------------------|
| ADCDATA | 0x00 | 2–4 bytes | `MCP3x6x_ADR_ADCDATA` |
| CONFIG0 | 0x01 | 1 byte | `MCP3x6x_ADR_CONFIG0` |
| CONFIG1 | 0x02 | 1 byte | `MCP3x6x_ADR_CONFIG1` |
| CONFIG2 | 0x03 | 1 byte | `MCP3x6x_ADR_CONFIG2` |
| CONFIG3 | 0x04 | 1 byte | `MCP3x6x_ADR_CONFIG3` |
| IRQ | 0x05 | 1 byte | `MCP3x6x_ADR_IRQ` |
| MUX | 0x06 | 1 byte | `MCP3x6x_ADR_MUX` |
| SCAN | 0x07 | 3 bytes | `MCP3x6x_ADR_SCAN` |
| TIMER | 0x08 | 3 bytes | `MCP3x6x_ADR_TIMER` |
| OFFSET | 0x09 | 3 bytes | `MCP3x6x_ADR_OFFSET` |
| GAIN | 0x0A | 3 bytes | `MCP3x6x_ADR_GAIN` |
| RESERVED1 | 0x0B | 3 bytes | `MCP3x6x_ADR_RESERVED1` |
| RESERVED2 | 0x0C | 1 byte | `MCP3x6x_ADR_RESERVED2` |
| LOCK | 0x0D | 1 byte | `MCP3x6x_ADR_LOCK` |
| RESERVED3 | 0x0E | 2 bytes | `MCP3x6x_ADR_RESERVED3` |
| CRCCFG | 0x0F | 2 bytes | `MCP3x6x_ADR_CRCCFG` |

### Chip-ID Validation

`begin()` validates the chip by reading RESERVED3 (0x0E) as a 16-bit value and extracting the low nibble from byte[0]:

| Part | Resolution | Channels | Chip-ID Nibble |
|------|------------|----------|----------------|
| MCP3461 | 16-bit | 2 | 0x8 |
| MCP3462 | 16-bit | 4 | 0x9 |
| MCP3464 | 16-bit | 8 | 0xB |
| MCP3561 | 24-bit | 2 | 0xC |
| MCP3562 | 24-bit | 4 | 0xD |
| MCP3564 | 24-bit | 8 | 0xF |

### Status Byte Format

The status byte returned on MISO during every SPI transaction:

| Bit 7:6 | Bit 5:3 | Bit 2 | Bit 1 | Bit 0 |
|---------|---------|-------|-------|-------|
| Reserved (0) | ADDR (inverted device address ack) | DR | CRCCFG | POR |

- **POR (bit 0)**: Power-On Reset occurred
- **CRCCFG (bit 1)**: CRC configuration error
- **DR (bit 2)**: Data Ready
- **ADDR (bits 5:3)**: Inverted device address acknowledge
- **Bits 7:6**: Reserved, always 0

All `status_*()` accessors invert the raw bit value because the MCP3x6x uses active-low convention for some status bits.

### SPI Mode and Clock

The library uses `SPISettings()` default = 4 MHz, MSBFIRST, SPI_MODE0 (CPOL=0, CPHA=0). The MCP3x6x supports both SPI Mode 0,0 and Mode 1,1 at up to 20 MHz.

## Decisions

| Decision | Rationale |
|----------|-----------|
| `_status` updated in `_transfer()` only | All SPI traffic goes through `_transfer()`, making it the single choke point for status updates |
| `status_*()` return inverted bit values | Hardware convention: status bits are active-low |
| `_status.raw` updated on every transfer | The status byte from the device contains POR, CRCCFG, DR, and ADDR bits in every transaction |
| Top-level methods return `status_t` not `void` | Retained for backward compatibility and convenience; `_status` already holds all status info, so the return value is redundant but harmless |

## Risks and Trade-offs

| Risk | Mitigation |
|------|------------|
| Future developer adds an SPI path bypassing `_transfer()` | Spec makes `_transfer()` the single choke point explicit; code review gate |
| `_status` becomes stale if read without a preceding SPI transaction | Callers should only call `status_*()` after an SPI operation; documented in method docs |
| `status_t` return type on top-level methods could be confused with fresh SPI status | Documented in spec: return value is redundant but harmless; `_status` is authoritative |

## File Changes

No source code changes required — the library already implements the specified behavior. The specification documents existing implementation.

### No source files modified

| File | Change | Reason |
|------|--------|--------|
| None | N/A | Library already implements all specified behavior |

## OpenSpec Artifacts

| File | Purpose |
|------|---------|
| `openspec/changes/mcp3x6x-spi-protocol/.openspec.yaml` | Change metadata |
| `openspec/changes/mcp3x6x-spi-protocol/proposal.md` | Why and what changes |
| `openspec/changes/mcp3x6x-spi-protocol/specs/library/spec.md` | Delta requirements (REQ-001…REQ-008) |
| `openspec/changes/mcp3x6x-spi-protocol/design.md` | Technical approach and decisions |
| `openspec/changes/mcp3x6x-spi-protocol/tasks.md` | Implementation and verification checklist |
