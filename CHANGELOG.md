# Changelog

All notable changes to the MCP3x6xR library are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] — 2026-11

### Fixed (datasheet correctness)

- **LOCK register**: POR default is `0xA5` (unlocked). `lock()` now locks by default
  (writes `0x00`); `unlock()` re-unlocks with `lock(MCP3x6x_CFG_LOCK)` (`0xA5`).
  `configure()` unlocks before register writes and writes the supplied lock value last.
- **CONFIG2 bitfields**: field order corrected to datasheet bit layout
  (BOOST[7:6], GAIN[5:3], AZ_MUX[2], AZ_REF[1], RESERVED[0]). `write(Config2)`
  forces the reserved bit to `1` as required by the datasheet.
- **CONFIG0 bitfields**: VREF_SEL moved to bit 7 and CONFIG0[6] to bit 6
  (fields were swapped).
- **STATUS register**: layout corrected to STAT[7:6]=0, DEV_ADDR[5:3], DR=2,
  CRCCFG=1, POR=0. `getDeviceAddress()` now reads the inverted address correctly.
- **Voltage conversion**: `toVoltage()` now divides by the real gain factor
  (`2^(gain-1)`, `x3` for the `x1/3` gain setting) per datasheet equation;
  `getMaxValue()` returns half-scale `2^(N-1)` instead of `2^N`.
- **Differential reads**: `analogReadDifferential(pinP, pinN)` maps the VIN+
  channel of each `Mux` selector (the previous raw OR-ing produced the wrong
  mux register). Any 4-bit selector is accepted.
- **Scan readback**: `readADCDATA()` strips the CH_ID nibble and sign-extends
  values received in `ID_SGNEXT_DATA` format; `getLastChannel()` reports the
  channel of the last conversion. `startScan()` sets CONV_MODE=continuous;
  new `startScanOnce()` performs a single scan cycle ending in standby.

### Changed (breaking)

- Removed the invalid `src/MCP3x6x.h` C wrapper (an `extern "C"` wrapper around a
  C++ class template is not valid C++ and exposed no C API). Include
  `Arduino_MCP3x6x_Library.hpp` directly; all examples were updated accordingly.

### Added

- `CHANGELOG.md` (this file) and an upgrade section in the README.
