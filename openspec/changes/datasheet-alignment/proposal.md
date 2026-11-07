# Change: Datasheet alignment (datasheet-alignment)

## Why

During a design review against the MCP3461/2/4R and MCP3561/2/4R datasheets
(DS20006404C, DS20006181C), several inconsistencies between the library code
and the documented register maps were identified. These range from inverted
bit logic (STATUS byte fields) to incorrect chip-ID detection (reading an
undocumented RESERVED3 register instead of the documented STATUS byte
device-address field) to inverted control loops (waiting for DR while it is
already asserted). This change aligns the implementation with the datasheets.

## What Changes

### Capability: register-map (MODIFIED)

1. **`getDeviceAddress()`** — The STATUS byte (Section 6.2.6) echoes the
   hard-coded device address as STAT[5:4] = DEV_ADDR[1:0], STAT[3] = DEV_ADDR[0].
   The current code returns `(~_status.addr) & 0x07`, which inverts all three
   bits and includes the duplicate STAT[3]. Fix: return `(_status.addr >> 1) & 0x03`.

2. **IRQ register status bits are read-only** — The datasheet (Section 6.8)
   documents POR_STATUS, CRCCFG_STATUS, DR_STATUS in the IRQ register (address
   0x5) as status bits cleared by reading the register. They are NOT writable
   configuration bits. The code's `Irq` union includes them as writable bitfields
   and `write(_irq)` would write them back, corrupting status. Fix: remove
   `por_status`, `crccfg_status`, `dr_status` from the writable union; they are
   read-only status bits.

3. **Chip ID detection** — `begin()` reads register 0xE (RESERVED3) and masks
   `& 0x0F` to guess the chip variant. The datasheet does not document RESERVED3
   as a chip-ID register; chip identification is via the STATUS byte DEV_ADDR
   field (STAT[5:4]). Fix: use STATUS byte DEV_ADDR for chip identification.

4. **CONFIG0/RESERVED delayed reset/restart** — Datasheet Section 5.16.3 states
   that writing CONFIG0 or RESERVED registers (0xB, 0xC) causes a delayed
   reset/restart with TADC_SETUP delay. The code writes these without accounting
   for the startup delay. Fix: add TADC_SETUP delay handling when writing these
   registers.

5. **Full Reset updates LOCK** — Datasheet Section 6.2.5 states Full Reset
   resets the entire register map including LOCK to default (0xA5 = unlocked).
   The code's `reset()` sends the Full Reset command but doesn't update the
   local `_lock` value. Fix: `reset()` sets `_lock.raw = 0xA5`.

### Capability: adc-api (MODIFIED)

1. **`analogRead()` mux mode DR-wait loop inverted** — The datasheet
   (Section 6.8.1) states DR_STATUS is logic '0' when data is ready (active-low).
   The current loop `while (!_status.dr)` continues while data IS ready and
   exits when it is not — backwards. Fix: `while (_status.dr)`.

2. **`readADCDATA()` vs `read(Adcdata*)` sign extension mismatch** — Both read
   ADCDATA but `readADCDATA()` applies sign extension for ID_SGNEXT_DATA while
   `read(Adcdata*)` returns raw via `_getValue()` without sign extension. Fix:
   make `read(Adcdata*)` apply the same sign extension as `readADCDATA()`.

## Impact

- Affected specs: `register-map` (MODIFIED), `adc-api` (MODIFIED).
- Affected code: `src/Arduino_MCP3x6x_Library.hpp` (only source file).
- Open question: IRQ register bit layout — the datasheet mentions EN_STP,
  EN_FASTCMD, IRQ_MODE[1:0], POR_STATUS, CRCCFG_STATUS, DR_STATUS but does not
  explicitly document bit positions in the text extracted. The current bitfield
  layout in the code (`en_stp:1, en_fastcmd:1, irq_mode:2, por_status:1,
  crccfg_status:1, dr_status:1, reserved:1`) is retained pending hardware
  verification.

## Non-Goals

- CRC-16 communication validation (deferred).
- Multi-device SPI address selection (deferred).
- Hardware validation (no HIL available; verified by compile matrix).

## Open Questions

- IRQ register exact bit positions for EN_STP, EN_FASTCMD, IRQ_MODE[1:0] —
  datasheet text extraction did not reveal explicit bit positions; current
  layout is retained pending hardware verification.
