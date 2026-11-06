# Task Index

No usable remote issue/task tracker is currently available for this repository.

- Tracker CLI resolution: `forge-cli.sh resolve` → `origin` → GitHub (`github.com:nerdyscout/Arduino_MCP3x6x_Library`) → `gh`.
- `forge-cli.sh check` → `gh` is not installed or not in PATH (exit code 4). Install via the package manager / `cli.github.com`, then run `gh auth login` before remote tracker use.
- Until the GitHub CLI is installed and authenticated, this file is the local fallback working index. It links to the OpenSpec change and must not duplicate requirement/design prose.

## Change: `datasheet-correctness` (archived 2026-11-06)

- OpenSpec change: `openspec/changes/archive/2026-11-06-datasheet-correctness/`
- Delivered on `develop` as stacked commits, not yet pushed.

### Execution items

| # | Task ID | Title | Status | Commit |
|---|---------|-------|--------|--------|
| 1 | 1.1, 1.2 | `fix/lock-semantics` | Done | `faf84ca` |
| 2 | 2.1, 2.2 | `fix/config2-fields` | Done | `69f4bd0` |
| 3 | 3.1, 3.2 | `fix/config0-vref` | Done | `4950b5d` |
| 4 | 4.1, 4.2 | `fix/status-layout` | Done | `c64f5b8` |
| 5 | 5.1 | `fix/c-header` | Done | `d13e3a0` |
| 6 | 6.1–6.4 | `fix/adc-api-semantics` | Done | `0603459` |
| 7 | 7.1, 7.2 | `docs/metadata-sync` | Done | `c93d7ac` |
| 8 | 8.1 | `test/hil-smoke` | Done | `24a81c0` |
| 9 | 9.1 | `chore(release): 0.2.0` | Done | `3d28d04` |

## Change: `dma-scan`

- OpenSpec change: `openspec/changes/dma-scan/`
- Project decisions (grilled 2026-11-06): DMA for ESP32 + SAMD only; no stubs
  for other controllers; CRC-16 deferred per timing analysis; version stays
  **0.2.0** (CHANGELOG gains an *Added* section, no bump); no hardware
  available — engines verified by compile matrix; native-testable core
  verified by `pio test -e native_ci`.

### Execution items

| # | Task ID | Title | Status | Priority | Owner | Effort |
|---|---------|-------|--------|----------|-------|--------|
| 1 | 1.1–1.3 | Core: `DmaSample` + `DmaRingBuffer` + members | Done | Critical | engineer | M |
| 2 | 2.1–2.4 | DMA lifecycle API (enable/disable/state/samples) | Done | Critical | engineer | M |
| 3 | 3.1–3.5 | Scan-path integration (IRQ/completion/analogRead) | Done | Critical | engineer | L |
| 4 | 4.1, 4.2 | ESP32 engine (spi_master async transaction) | Done | High | engineer | L |
| 5 | 5.1, 5.2 | SAMD21/51 engine (SERCOM DMAC RX trigger) | Done | High | engineer | L |
| 6 | 9.1–9.4 | Packaging, docs, 0.2.0 CHANGELOG *Added* | Done | Medium | engineer | S |

### Next action

None — DMA delivery stack complete. Next: bench validation on real hardware.

## Backlog

- Bench validation of `datasheet-correctness` fixes + HIL smoke on real hardware.
- `startScanOnce()` CONV_MODE=10 datasheet assumption to confirm on hardware.
- Deferred Medium/Low datasheet features (see archived OpenSpec proposal):
  CRC validation, calibration setters, `MCP3x6x_SPI_ADR` selection, burnout
  sensor-fault detection, incremental-read/burst SPI, scan-timer API,
  IRQ polarity for push-pull high, SPI Mode 1,1 support.
- Install + authenticate `gh`, migrate tracking to GitHub issues, retire this
  file.
