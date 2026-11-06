# Tasks

Delivery stack for change `datasheet-correctness`. Each item below is its own change/PR and
includes its own test deltas.

## 1. `fix/lock-semantics`

- [x] 1.1 Implement LOCK register write-access semantics (LOCK=0xA5 unlock, non-0xA5 lock, Full
      Reset restore, configure unlock/last-lock ordering); verify via native LOCK tests.
      (commit faf84ca)

- [x] 1.2 Rewrite LOCK-related native tests for corrected datasheet values; verify with
      `pio test -e native_ci --project-conf platformio_ci.ini`. (commit faf84ca)

## 2. `fix/config2-fields`

- [x] 2.1 Correct CONFIG2 bit field placement (BOOST[7:6], GAIN[5:3], AZ_MUX[2], AZ_REF[1], RESERVED[0])
      and reserved-bit handling; verify via native CONFIG2 tests. (commit 69f4bd0)

- [x] 2.2 Rewrite CONFIG2 native tests for corrected bit positions; verify with
      `pio test -e native_ci --project-conf platformio_ci.ini`. (commit 69f4bd0)

## 3. `fix/config0-vref`

- [x] 3.1 Correct CONFIG0 VREF selection and begin() boot sequence (reset + POR/chip-ID probe only,
      no LOCK write, Full Reset restores LOCK=0xA5); verify via native CONFIG0/VREF/begin tests.
      (commit 4950b5d)

- [x] 3.2 Rewrite CONFIG0/VREF/begin native tests for corrected behavior; verify with
      `pio test -e native_ci --project-conf platformio_ci.ini`. (commit 4950b5d)

## 4. `fix/status-layout`

- [x] 4.1 Correct STATUS byte bit layout (STAT[7:6]=0, DEV_ADDR ack [5:3], DR=2, CRCCFG=1, POR=0);
      verify via native STATUS fast-command/IRQ tests. (commit c64f5b8)

- [x] 4.2 Rewrite STATUS native tests for corrected bit positions; verify with
      `pio test -e native_ci --project-conf platformio_ci.ini`. (commit c64f5b8)

## 5. `fix/c-header`

- [x] 5.1 Delete invalid `src/MCP3x6x.h` C wrapper and update all 11 examples to include
      `Arduino_MCP3x6x_Library.hpp`; verify via example compile checks and repository search.
      (commit d13e3a0; repository search verified, full example compile matrix pending)

## 6. `fix/adc-api-semantics`

- [x] 6.1 Set `startScan()` to CONV_MODE=11 and add `startScanOnce()` with CONV_MODE=10 while
      keeping `startContinuous()`/`stopContinuous()` valid; verified via native scan-mode tests.
      (commit 0603459)

- [x] 6.2 Correct `analogReadDifferential()` selector mapping and remove range validation; verified
      via native differential read tests. (commit 0603459)

- [x] 6.3 Implement `toVoltage()` datasheet transfer function and `getMaxValue()` = 2^(N-1); verified
      via native voltage conversion tests. (commit 0603459)

- [x] 6.4 Implement `readADCDATA()` channel strip/sign extension and `getLastChannel()`; native
      ADCDATA readback tests added. (commit 0603459)

## 7. `docs/metadata-sync`

- [x] 7.1 Sync README.md, keywords.txt, Doxyfile INPUT, library.json/library.properties, and add
      CHANGELOG.md; verified via documentation/metadata inspection. (README upgrade section,
      keywords for new API, Doxyfile ./src/, library.json typo, CHANGELOG.md created)

- [x] 7.2 Record REUSE.toml versus SPDX licensing open question; no licensing change without explicit
      instruction. (open question recorded in proposal.md; no licensing files touched)

## 8. `test/hil-smoke`

- [ ] 8.1 Add HIL smoke example exercising LOCK/unlock, CONFIG0 VREF, CONFIG2 gain/boost, STATUS
      bits, and voltage conversion; mark hardware-only and exclude from CI.

## 9. `chore(release): 0.2.0` commit at top

- [ ] 9.1 Cut release 0.2.0 (version bump, CHANGELOG entry, README upgrade section, release commit);
      verify via release metadata inspection.
