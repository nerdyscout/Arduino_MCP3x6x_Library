# Proposal

## Why

The Arduino `MCP3x6xR` library (`src/Arduino_MCP3x6x_Library.hpp`) does not yet implement the
datasheet behavior of the MCP3461/2/4R and MCP3561/2/4 ADC families correctly. Critical and High
severity mismatches remain in the register map handling, scan-mode conversion sequencing, raw data
readback, and voltage conversion, and the native test suite encodes several of the wrong mappings.
Library users therefore receive incorrect LOCK, CONFIG, STATUS, scan, and voltage behavior.

## Background

The library is a single-header C++17 class library (`MCP3x6x<MAX_RESOLUTION, MAX_CHANNELS>`) that
communicates over SPI with Microchip ADCs and exposes register unions plus fast commands. The
authoritative datasheets are available locally (git-ignored) at
`docs/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf` and
`docs/MCP3561-2-4-Family-Data-Sheet-DS20006181C.pdf`. The currently shipped behavior is release
0.1.x.

## Stakeholder Needs

- Firmware engineers need trustworthy LOCK/unlock and `configure()` behavior so field firmware can
  reliably re-configure devices.
- Bench and hobby developers need gain- and reference-aware voltage conversion (`toVoltage()`).
- Scan-mode users need correct continuous versus one-shot conversion sequencing for power-sensitive
  designs.

## What Changes

- **BREAKING** Correct LOCK register semantics (§8.14, §6.6): `MCP3x6x_CFG_LOCK = 0xA5`, `unlock()`
  writes the password entry code `0xA5`, `lock(key)` writes the supplied key (default locks), and
  `configure()` unlocks before register writes and writes the supplied Lock value last.
- **BREAKING** Correct CONFIG2 field placement (§8.4): BOOST[7:6], GAIN[5:3], AZ_MUX[2], AZ_REF[1],
  RESERVED[0] (must be 1).
- **BREAKING** Correct CONFIG0 VREF selection and `begin()` boot sequence (§8.2, §6.2.5 Table 6-2):
  reset plus POR/chip-ID probe only, no LOCK write; Full Reset is a fast command that restores
  LOCK to `0xA5`.
- **BREAKING** Correct STATUS byte layout (§6.2.5/§6.2.6, Figure 6-1): STAT[7:6]=0, DEV_ADDR ack at
  [5:3], DR_STATUS=2, CRCCFG_STATUS=1, POR_STATUS=0.
- **BREAKING** `startScan()` sets CONV_MODE=11 (continuous) in addition to its current setup; add
  `startScanOnce()` with CONV_MODE=10 (one-shot then standby). `startContinuous()` remains valid.
- **BREAKING** `analogReadDifferential(pinP, pinN)` keeps two args, maps the selectors, and allows
  any 4-bit selector without range validation; update `examples/MCP3464_Mux_Differential`.
- **BREAKING** `toVoltage()` implements the datasheet transfer function (Equation 5-5); `getMaxValue()`
  returns 2^(N-1); examples and README that divide by `getMaxValue()` are updated.
- **BREAKING** `readADCDATA()` strips channel ID bits for ID_SGNEXT_DATA and sign-extends from bit
  27; add `getLastChannel()`; update tests.
- **BREAKING** Delete invalid `src/MCP3x6x.h` C wrapper; update all 11 examples to include
  `Arduino_MCP3x6x_Library.hpp`.
- Documentation and metadata sync; release 0.2.0 with CHANGELOG.md and README "Upgrading to 0.2.0"
  section.

## In Scope

- Fix all Critical + High issues from the grilling outcome.
- Rewrite all tests that encode wrong datasheet behavior.
- Cheap documentation/metadata sync.

## Out of Scope / Non-Goals

- Medium/Low refactors: IRQ polarity in `attachIRQ()`, `attachMCLK()` tone limits, `write(Crccfg)`
  to read-only register, MUX default constant vs hardware reset, visibility of private register
  types, broader error-handling cleanup.
- Multi-byte register read endianness (classified Medium; deferred).
- REUSE.toml versus SPDX headers for examples (licensing decision pending explicit instruction).

## Capabilities

### New Capabilities

- `register-map`: Behavioral contract for the ADC register map as implemented by the library
  (LOCK, CONFIG0, CONFIG2, STATUS).
- `adc-api`: Behavioral contract for library ADC operations (boot, scan modes, differential reads,
  data conversion, data readback).
- `library-packaging`: Metadata and documentation shipped with the library release.

### Modified Capabilities

- None. The project currently has no existing OpenSpec specs; all three capabilities are new for
  this change.

## Impact

- `src/Arduino_MCP3x6x_Library.hpp`: register unions, lock/configure/begin, status, scan, ADC data
  path, voltage conversion.
- `src/MCP3x6x.h`: deleted.
- `examples/`: all 11 examples updated to include the C++ header and corrected behavior.
- `test/test_MCP3x6x/`: rewrite tests encoding wrong mappings.
- Docs/metadata: README.md, keywords.txt, Doxyfile, library.json, library.properties, CHANGELOG.md.

## Risks / Trade-offs

- Breaking API changes in a 0.x library are acceptable to the stakeholders; mitigated by release
  0.2.0 CHANGELOG and README upgrade section.
- No hardware is available; verification is limited to datasheet-derived native tests and compile
  matrix until the HIL smoke example is bench-validated.

## Open Questions

- The `startScanOnce()` CONV_MODE=10 (one-shot then standby) behavior is assumed from the datasheet
  conversion-mode encoding and is noted as the only unresolved technical assumption; it requires no
  further grilling decision before implementation.
- The REUSE.toml versus SPDX header licensing approach for examples remains open pending explicit
  instruction.

## Success Criteria

- All Critical + High datasheet mismatches are corrected.
- Native tests pass against corrected datasheet-derived mappings.
- Compile matrix passes across representative PlatformIO environments.
- Documentation and metadata are consistent at release 0.2.0.
