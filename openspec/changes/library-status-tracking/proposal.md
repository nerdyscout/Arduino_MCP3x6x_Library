# Proposal: library-status-tracking

## Objective

Formalize the `status_t _status` class member in `MCP3x6x` as the single source of truth for SPI communication status, requiring that every SPI read, write, and fast command updates `_status` from the device's status byte, and that all `status_*()` accessor methods return values derived from `_status`.

## Why (stakeholder needs)

Users and library maintainers need a reliable way to inspect the status of the most recent SPI transaction. Currently `_status` exists as a private member and is written by `_transfer()`, but this behavior is not specified as a requirement. Without a formal specification:

- Future refactoring could accidentally bypass `_status` updates (e.g., a new read/write path that doesn't call `_transfer()`)
- `status_*()` methods could be changed to read from a different source, causing inconsistency
- Users cannot rely on `status_dr()`, `status_crccfg()`, `status_por()` reflecting the actual last SPI transaction

## What changes

- **Requirement:** `status_t _status` SHALL exist as a private member of `MCP3x6x`
- **Requirement:** Every SPI read, write, and fast command SHALL update `_status` from the device's status byte
- **Requirement:** `status_dr()`, `status_crccfg()`, `status_por()` SHALL return values derived from `_status`
- **Requirement:** `status()` SHALL return `_status.raw`

## Impact

- Capability affected: `library` (the `MCP3x6x` class)
- No changes to public API
- No changes to example sketches
- No changes to existing test files (they already pass)

## Non-goals

- Adding new public status methods beyond what already exists
- Changing the bit layout of `status_t`
- Adding CRC or checksum validation to status

## Open questions

- OQ-1: Should `_status` be updated even when SPI transactions fail (e.g., timeout)?
- OQ-2: Should `_status` be reset to zero at construction, or retain POR default?
