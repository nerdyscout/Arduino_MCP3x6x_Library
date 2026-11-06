# Progress Notes

## 2026-11-06 — Grilling decisions (dma-scan)

- Complete DMA: native core + all five platform engines (ESP32, SAMD21/51,
  STM32, nRF52, Teensy). CRC-16 deferred per timing analysis.
- Version stays **0.2.0** — DMA features land as *Added* in the existing
  `[0.2.0]` CHANGELOG entry; no bump.
- `PLAN.md` deleted; content migrated into `design.md` of this change.
- No hardware: engines verified by compile matrix; core by native CI.
- `datasheet-correctness` archived to
  `openspec/changes/archive/2026-11-06-datasheet-correctness/` (specs promoted).
- Repo URLs synced to `github.com/nerdyscout/…` (README badge,
  library.json, library.properties).
- Stray `test_basic.cpp` whitespace diff discarded.

## Delivery log

- (update as tasks complete)
