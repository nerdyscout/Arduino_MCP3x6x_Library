# Tasks

## 1. Verify _status member exists

- [ ] T-001.1 Verify `status_t _status;` is declared as a private member of `MCP3x6x` (line ~452 of `src/Arduino_MCP3x6x_Library.hpp`). (Priority: high — est. ~5 min. Verifies REQ-001.)

## 2. Verify _transfer updates _status

- [ ] T-002.1 Verify `_transfer(uint8_t* data, uint8_t addr, size_t size)` contains `_status.raw = _spi->transfer(addr)` (line ~486). (Priority: high — est. ~5 min. Verifies REQ-002.)
- [ ] T-002.2 Verify all register write methods (`write(Config0)`, `write(Config1)`, `write(Config2)`, `write(Config3)`, `write(Irq)`, `write(Mux)`, `write(Scan)`, `write(Timer)`, `write(Offset)`, `write(Gain)`, `write(Lock)`, `write(Crccfg)`) call `_transfer()` or `_fastcmd()`. (Priority: high — est. ~15 min. Verifies REQ-008.)
- [ ] T-002.3 Verify all register read methods (`read(Config0&)`, `read(Config1&)`, `read(Config2&)`, `read(Config3&)`, `read(Irq&)`, `read(Mux&)`, `read(Scan&)`, `read(Timer&)`, `read(Offset&)`, `read(Gain&)`, `read(Lock&)`, `read(Crccfg&)`, `read(Adcdata*)`) call `_transfer()`. (Priority: high — est. ~15 min. Verifies REQ-008.)
- [ ] T-002.4 Verify `_fastcmd()` calls `_transfer()`. (Priority: high — est. ~5 min. Verifies REQ-003.)
- [ ] T-002.5 Verify `analogRead()` calls `_transfer()` via `read()` or `conversion()`. (Priority: high — est. ~5 min. Verifies REQ-008.)

## 3. Verify status_* methods read from _status

- [ ] T-003.1 Verify `status_dr()` returns `!_status.dr` (line ~1566). (Priority: high — est. ~5 min. Verifies REQ-004.)
- [ ] T-003.2 Verify `status_crccfg()` returns `!_status.crccfg` (line ~1576). (Priority: high — est. ~5 min. Verifies REQ-005.)
- [ ] T-003.3 Verify `status_por()` returns `!_status.por` (line ~1586). (Priority: high — est. ~5 min. Verifies REQ-006.)
- [ ] T-003.4 Verify `status()` returns `_status.raw` (line ~1780). (Priority: high — est. ~5 min. Verifies REQ-007.)

## 4. Run tests

- [ ] T-004.1 Run native tests with `pio test -e native` and confirm all tests pass. (Priority: high — est. ~10 min. Verifies REQ-001…REQ-008.)
- [ ] T-004.2 Run example builds with `./examples/build.sh` and confirm all examples compile. (Priority: medium — est. ~15 min. Verifies no regression.)

## Workflow follow-up

- No code changes are needed — the library already implements the specified behavior
- If any requirement is not met by the current implementation, create a separate change through grilling to address the gap
- Archive this change with `/opsx-archive` once review is complete
