# Proposal: MCP3x6x SPI Protocol Specification

## Objective and Problem Statement

The `Arduino_MCP3x6x_Library` implements SPI communication between Arduino-class MCUs and Microchip MCP346x/MCP356x family ADCs (16-bit and 24-bit delta-sigma converters). The current implementation (`src/Arduino_MCP3x6x_Library.hpp`) contains working SPI transaction logic but lacks a formal specification of the SPI wire protocol, command byte structure, register map, chip-ID validation, and status byte semantics.

Without a formal specification, the following risks exist:
- Future refactoring could accidentally bypass the `_status` tracking mechanism, causing `status_dr()`, `status_crccfg()`, `status_por()`, and `status()` to return stale or incorrect values.
- New SPI transaction paths could be added without routing through `_transfer()`, breaking the single-source-of-truth guarantee for `_status`.
- The relationship between `status_t` return values on top-level methods and the `_status` member is not documented, leading to potential confusion about when return values differ from `_status`.

## Background

The MCP3x6x family (MCP3461/2/4, MCP3561/2/4, and their R variants) communicates over SPI at up to 20 MHz. Each transaction consists of:
1. Chip Select (CS) falling edge to initiate
2. One command byte (8 bits) sent on MOSI
3. Optional data bytes (register read/write payload) on MOSI
4. Status byte + optional data bytes on MISO
5. CS rising edge to terminate

The command byte format is `[DEVICE_ADDR[7:6] | REG_ADDR[5:2] | CMD_TYPE[1:0]]`. The device address is factory-configured via fuses and read back via the RESERVED_E register.

The `_status` member (type `status_t`) is updated on every SPI transaction in `_transfer()` and captures the status byte returned by the device. Public `status_*()` accessor methods (`status_dr()`, `status_crccfg()`, `status_por()`, `status()`) read from `_status`.

## Stakeholder Needs

- **Library users** need reliable, documented SPI behavior to build applications on top of the library.
- **Library maintainers** need a spec that prevents accidental regression of the SPI transaction flow and `_status` tracking.
- **Downstream projects** (e.g., Linux kernel driver `drivers/iio/adc/mcp3564.c`) reference this library's behavior as a de facto spec; formalizing it reduces cross-implementation drift.

## In Scope

- Definition of the SPI command byte format (device address, register address, command type bits).
- Definition of all fast commands (reset, conversion, standby, shutdown, full shutdown, static read, incremental read, incremental write).
- Definition of all register addresses (ADCDATA through CRCCFG).
- Definition of chip-ID nibbles for all MCP3x6x variants.
- Definition of the `status_t` bit layout (POR, CRCCFG, DR, ADDR, reserved).
- Requirement that `_status` is updated on every SPI transaction via `_transfer()`.
- Requirement that all `status_*()` accessors read from `_status`.
- Design note explaining why top-level methods return `status_t` rather than `void`.

## Out of Scope

- DMA engine implementation details (DMA ring buffer, SAMD/ESP32 DMA setup).
- Pin-level wiring diagrams or PCB layout guidance.
- Calibration algorithms or voltage conversion formulas beyond `toVoltage()`.
- Register value semantics beyond what is needed for SPI transport (e.g., specific OSR bit patterns beyond the data_format field).

## Non-Goals

- Defining analog front-end behavior (gain, bias, reference selection beyond what's in CONFIG registers).
- Defining conversion timing or data rate calculations.
- Defining interrupt handling beyond the IRQ register address and mode bits.

## Impact

- **Library users**: No API change; this is a documentation/specification change.
- **Library maintainers**: Future changes to SPI transaction code must maintain `_status` consistency.
- **Downstream projects**: A stable reference for MCP3x6x SPI behavior.

## Risks and Trade-offs

| Risk | Mitigation |
|------|------------|
| Future developer adds an SPI path bypassing `_transfer()` | Spec makes `_transfer()` the single choke point explicit; code review gate |
| `_status` becomes stale if read without a preceding SPI transaction | Documented in method docs; callers should call `status_*()` only after an SPI operation |
| `status_t` return type on top-level methods could be confused with fresh SPI status | Documented in spec: return value is redundant but harmless; `_status` is authoritative |

## Open Questions

- OQ-1: Should `_status` be reset to `0x00` at construction, or should it reflect POR state (which the datasheet says is POR=1)?
- OQ-2: Should `_validateChipID()` read RESERVED_E as 16-bit or 8-bit? The Linux driver reads 16-bit; the current library code reads 16-bit but extracts only the low nibble of byte[0].
