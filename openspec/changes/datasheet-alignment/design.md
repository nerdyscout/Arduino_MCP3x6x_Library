# Design: Datasheet alignment

## Context

This change corrects register-map and API behavior to match the MCP3461/2/4R
and MCP3561/2/4R datasheets (DS20006404C, DS20006181C). All changes are
backward-incompatible at the bit level but preserve the public API surface
(method names and signatures are unchanged).

## Decisions

### D1: `getDeviceAddress()` returns `(_status.addr >> 1) & 0x03`

The STATUS byte (Section 6.2.6) layout is:
- STAT[7:6] = 0 (reserved)
- STAT[5:4] = DEV_ADDR[1:0]
- STAT[3] = DEV_ADDR[0] (duplicate)
- STAT[2] = DR_STATUS
- STAT[1] = CRCCFG_STATUS
- STAT[0] = POR_STATUS

The current code returns `(~_status.addr) & 0x07` which inverts bits and
includes the duplicate STAT[3]. Fix: extract DEV_ADDR[1:0] from bits [5:4] as
`(_status.addr >> 1) & 0x03`.

### D2: IRQ register status bits are read-only

The `Irq` union currently includes `por_status`, `crccfg_status`, `dr_status`
as writable bitfields. The datasheet (Section 6.8) documents these as status
bits cleared by reading the IRQ register — they are not writable configuration
bits. Fix: remove them from the union; keep only `en_stp`, `en_fastcmd`,
`irq_mode` as writable configuration bits.

### D3: Chip ID via STATUS DEV_ADDR

Replace RESERVED3-based chip ID detection with STATUS byte DEV_ADDR reading.
During `begin()`, after `reset()`, read the STATUS byte and extract
`(_status.addr >> 1) & 0x03` as the device address. Map to chip variants:
- 0x0 → MCP3461/MCP3561 (2-channel)
- 0x1 → MCP3462/MCP3562 (4-channel)
- 0x2 → MCP3464/MCP3564 (8-channel)
- 0x3 → reserved/invalid

### D4: CONFIG0/RESERVED delayed reset/restart

Writes to CONFIG0 (address 0x1) or RESERVED (0xB, 0xC) trigger a delayed
reset/restart per datasheet Section 5.16.3. The `write()` path for these
registers must wait TADC_SETUP (256 DMCLK periods) before the next SPI
transaction. Implementation: add a `_writeWithResetDelay(addr, data)` helper
that calls `_transfer()` then delays `TADC_SETUP` microseconds before
returning.

### D5: Full Reset updates `_lock.raw = 0xA5`

The `reset()` fast command (command address 0x1110) resets the entire register
map including LOCK to 0xA5. Fix: `reset()` sets `_lock.raw = MCP3x6x_CFG_LOCK`
after issuing the command.

### D6: `analogRead()` DR-wait loop condition

Change `while (!_status.dr)` to `while (_status.dr)` in the mux-mode path.
DR_STATUS is logic '0' when data is ready (active-low per datasheet Section
6.8.1).

### D7: `read(Adcdata*)` sign extension consistency

`read(Adcdata*)` must apply the same ID_SGNEXT_DATA sign extension as
`readADCDATA()`:
```cpp
if (_config3.data_format == data_format::ID_SGNEXT_DATA) {
    value = (int32_t)((uint32_t)raw << 4) >> 4;
}
```

## File Changes

- `src/Arduino_MCP3x6x_Library.hpp` — all changes above (single-header library)

## Verification

- Native CI tests: 124/124 must pass.
- cppcheck: 0 findings.
- Compile matrix: all benchmarked environments must compile.
- OpenSpec validation: `openspec validate --all --strict` must pass.

## Risks

- IRQ register bit layout not explicitly documented in datasheet text
  extraction; current layout retained pending hardware verification.
- TADC_SETUP delay value (256 DMCLK periods) depends on MCLK frequency;
  implemented as a fixed delay in microseconds assuming 4.9152 MHz MCLK
  (256 DMCLK ≈ 52 µs at PRE=0).
