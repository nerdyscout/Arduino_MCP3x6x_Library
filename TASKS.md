# Task Index

No usable remote issue/task tracker is currently available for this repository.

- Tracker CLI resolution: `forge-cli.sh resolve` → `origin` → GitHub (`github.com:nerdyscout/Arduino_MCP3x6x_Library`) → `gh`.
- `forge-cli.sh check` → `gh` is not installed or not in PATH (exit code 4). Install via the package manager / `cli.github.com`, then run `gh auth login` before remote tracker use.
- Until the GitHub CLI is installed and authenticated, this file is the local fallback working index. It links to the OpenSpec change and must not duplicate requirement/design prose.

## Change: `datasheet-correctness`

- OpenSpec change: `openspec/changes/datasheet-correctness/`
- Proposal/design/tasks/specs: `openspec/changes/datasheet-correctness/{proposal.md,design.md,tasks.md,specs/}`

### Execution items

| # | Task ID | Title | Status | Priority | Owner | Effort |
|---|---------|-------|--------|----------|-------|--------|
| 1 | 1.1, 1.2 | `fix/lock-semantics` | In Progress | Critical | engineer | M |
| 2 | 2.1, 2.2 | `fix/config2-fields` | Pending | Critical | engineer | M |
| 3 | 3.1, 3.2 | `fix/config0-vref` | Pending | Critical | engineer | M |
| 4 | 4.1, 4.2 | `fix/status-layout` | Pending | Critical | engineer | M |
| 5 | 5.1 | `fix/c-header` | Pending | Critical | engineer | S |
| 6 | 6.1–6.4 | `fix/adc-api-semantics` | Pending | Critical | engineer | L |
| 7 | 7.1, 7.2 | `docs/metadata-sync` | Pending | Medium | engineer | S |
| 8 | 8.1 | `test/hil-smoke` | Pending | Low | engineer | S |
| 9 | 9.1 | `chore(release): 0.2.0` | Pending | Medium | engineer | S |

### Acceptance for item 1 (task 1.1 / 1.2)

- LOCK register default / unlock key is `0xA5`; `unlock()` uses the same key; `lock()` takes a key with default that locks.
- `begin()` resets and does not write LOCK; Full Reset restores `LOCK = 0xA5`.
- `configure()` unlocks before writes and writes the supplied lock last.
- Native tests rewritten for corrected LOCK values; `pio test -e native_ci --project-conf platformio_ci.ini` passes.

### Next action

Implement delivery-stack item 1 (`fix/lock-semantics`) on a stacked branch, then continue to item 2.
