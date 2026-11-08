# Design: library-status-tracking

## Context

The `MCP3x6x` class already has a `status_t _status` member (line 452 of `src/Arduino_MCP3x6x_Library.hpp`) and the `_transfer()` method already writes `_status.raw = _spi->transfer(addr)` (line 486). The `status_dr()`, `status_crccfg()`, `status_por()` methods already read from `_status` fields (lines 1566, 1576, 1586), and `status()` returns `_status.raw` (line 1780).

However, this behavior is not formalized as a specification. Without explicit requirements, future refactoring could:
- Add a new SPI code path that bypasses `_transfer()` and doesn't update `_status`
- Change `status_*()` methods to read from a different source
- Remove `_status` entirely, breaking code that depends on it

## Goals and non-goals

### Goals
- Formally specify that `_status` exists and is updated by every SPI transaction
- Formally specify that `status_*()` methods read from `_status`
- Prevent future regressions where `_status` is bypassed or stale

### Non-goals
- No changes to the `status_t` struct layout
- No changes to the `status_*()` method signatures
- No changes to example sketches or test files

## Technical approach

No code changes are required — the library already implements the specified behavior. The specification documents and formalizes the existing implementation:

1. `_status` is declared as `status_t _status;` (private member, line 452)
2. `_transfer()` writes `_status.raw = _spi->transfer(addr)` on every SPI transaction (line 486)
3. `_fastcmd()` calls `_transfer()`, so it indirectly updates `_status` (line 472)
4. All public SPI methods (write, read, fastcmd, analogRead) go through `_transfer()` or `_fastcmd()`
5. `status_dr()`, `status_crccfg()`, `status_por()` read from `_status` bit fields (lines 1566-1586)
6. `status()` returns `_status.raw` (line 1780)

## Decisions

| Decision | Rationale |
|----------|-----------|
| `_status` updated in `_transfer()` only | All SPI traffic goes through `_transfer()`, making it the single choke point for status updates |
| `status_*()` return inverted bit values | Hardware convention: status bits are active-low (e.g., DR=1 means "not ready") |
| `_status.raw` updated on every transfer | The status byte from the device contains POR, CRCCFG, DR, and ADDR bits in every transaction |
| Top-level methods return `status_t` not `void` | Retained for backward compatibility and convenience; `_status` already holds all status info, so the return value is redundant but not harmful (see `specs/library/spec.md` design note) |

## Risks and trade-offs

| Risk | Mitigation |
|------|------------|
| Future developer adds an SPI path bypassing `_transfer()` | Code review should catch this; the spec makes it explicit |
| `_status` becomes stale if read without a preceding SPI transaction | Callers should only call `status_*()` after an SPI operation; this is documented in method docs |

## File changes

No source code changes required — the library already implements the specified behavior. The specification documents existing implementation.

### No source files modified

| File | Change | Reason |
|------|--------|--------|
| None | N/A | Library already implements all specified behavior |

## OpenSpec artifacts

| File | Purpose |
|------|---------|
| `openspec/changes/library-status-tracking/.openspec.yaml` | Change metadata |
| `openspec/changes/library-status-tracking/proposal.md` | Why and what changes |
| `openspec/changes/library-status-tracking/specs/library/spec.md` | Delta requirements (REQ-001…REQ-008) |
| `openspec/changes/library-status-tracking/design.md` | Technical approach and decisions |
| `openspec/changes/library-status-tracking/tasks.md` | Implementation and verification checklist |
