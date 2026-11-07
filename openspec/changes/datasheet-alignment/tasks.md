# datasheet-alignment Tasks

## 1. `getDeviceAddress()` fix (register-map)
- [ ] 1.1 Change `getDeviceAddress()` from `(~_status.addr) & 0x07` to `(_status.addr >> 1) & 0x03`
  - Verified by: native test `test_getDeviceAddress_*` checks correct address extraction

## 2. IRQ register status bits read-only (register-map)
- [ ] 2.1 Remove `por_status`, `crccfg_status`, `dr_status` from `Irq` union bitfields
  - Verified by: native test `test_irq_read_only_status_bits` confirms these bits are not written by `write(_irq)`
- [ ] 2.2 Keep `en_stp`, `en_fastcmd`, `irq_mode` as writable config bits
  - Verified by: native test `test_irq_writable_config_bits` confirms these bits are written

## 3. Chip ID detection via STATUS DEV_ADDR (register-map)
- [ ] 3.1 Replace RESERVED3-based chip ID detection with STATUS byte `(_status.addr >> 1) & 0x03` in `begin()`
  - Verified by: native test `test_begin_chip_id_from_status` checks correct variant detection

## 4. CONFIG0/RESERVED delayed reset/restart (register-map)
- [ ] 4.1 Add `_writeWithResetDelay()` helper for CONFIG0 and RESERVED register writes
  - Verified by: native test `test_write_config0_delay` verifies delay is applied
- [ ] 4.2 Route CONFIG0 and RESERVED writes through `_writeWithResetDelay()`
  - Verified by: native test `test_write_reserved_delay` verifies delay is applied

## 5. Full Reset updates `_lock.raw` (register-map)
- [ ] 5.1 In `reset()`, set `_lock.raw = MCP3x6x_CFG_LOCK` after Full Reset command
  - Verified by: native test `test_reset_updates_lock` confirms `_lock.raw == 0xA5` after reset

## 6. `analogRead()` DR-wait loop fix (adc-api)
- [ ] 6.1 Change `while (!_status.dr)` to `while (_status.dr)` in mux-mode path
  - Verified by: native test `test_analogRead_mux_dr_wait` confirms correct loop behavior

## 7. `read(Adcdata*)` sign extension consistency (adc-api)
- [ ] 7.1 Apply ID_SGNEXT_DATA sign extension in `read(Adcdata*)` matching `readADCDATA()`
  - Verified by: native test `test_read_adcdata_sign_extension` confirms sign-extended value matches `readADCDATA()`

## 8. Verification sweep
- [ ] 8.1 Native CI: `pio test -e native_ci --project-conf platformio_ci.ini` → 124/124 pass
- [ ] 8.2 cppcheck: 0 findings on `src/Arduino_MCP3x6x_Library.hpp`
- [ ] 8.3 OpenSpec validate: `openspec validate --all --strict` passes
- [ ] 8.4 Compile matrix: all benchmarked environments compile

## Open Questions (documented, not assumed)

- IRQ register exact bit positions for EN_STP, EN_FASTCMD, IRQ_MODE[1:0] —
  datasheet text extraction did not reveal explicit bit positions; current
  layout is retained pending hardware verification.
- TADC_SETUP delay value (256 DMCLK periods) depends on MCLK frequency;
  implemented as a fixed delay assuming 4.9152 MHz MCLK (256 DMCLK ≈ 52 µs at
  PRE=0).
