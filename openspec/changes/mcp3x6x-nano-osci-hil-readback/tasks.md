# Tasks

Change: `mcp3x6x-nano-osci-hil-readback` — single intent: automated HIL readback (pio upload test_config.cpp + decoder.py configure/readback vs expected bytes, exit 0 end-to-end).
Capability: `hardware` (modified; delta REQ-011…REQ-017).
One intent only — no parallel independent changes.

## 1. test_config.cpp hardware path (no native change)

- [ ] T-001.1 Add `#ifndef PIO_NATIVE_TESTING` hardware `setup()`/`loop()` to `test/test_MCP3x6x/test_config.cpp`: `begin()`, `unlock()`, register writes config0/config1/config2/config3/irq/mux, then polling `analogRead(CH0..CH3)`, Serial at 115200 baud; mirror `test_runner.cpp` hardware mode. Keep the `#ifdef PIO_NATIVE_TESTING` mock tests and registration block unchanged. (Priority: High — est. ~60 min. Verifies REQ-011, REQ-012.)
- [ ] T-001.2 Confirm `pio run -e ATMEGA328P` compiles the hardware path and that `#ifdef PIO_NATIVE_TESTING` code is excluded for AVR (no AVR-only symbols in native mode). (Priority: High — est. ~15 min. Verifies REQ-011.)
- [ ] T-001.3 Run `pio test -e native` and confirm all native mock tests in `test_config.cpp` still pass — no regression from adding the hardware path. (Priority: High — est. ~15 min. Verifies REQ-017.)

## 2. Orchestration script `scripts/osci_hil_readback.sh`

- [ ] T-002.1 Write `scripts/osci_hil_readback.sh [--debug]`: (1) `pio run -e ATMEGA328P && pio upload -e ATMEGA328P`; (2) auto-detect Rigol over USBTMC (decoder.py resource auto-dect) with `--resource $OSCI_RESOURCE` env override; (3) `decoder.py --protocol spi --decoder 1 --mode configure --json --sclk CHAN3 --mosi CHAN4 --miso CHAN2 --cs CHAN1 --width 8 --edge rise --polarity pos --endian msb --select cs`; (4) `decoder.py --protocol spi --decoder 1 --mode readback --json --etable on`; (5) parse JSON, compare decoded MOSI bytes vs expected (0x78, 0x79, 0x86+0xC0, 0x88, 0x8A, 0x8C, 0x8E, 0x9A+0x08, 0x68, 0x81+status+2 data); (6) propagate exit codes; (7) pass `--debug` through to decoder.py. (Priority: High — est. ~90 min. Verifies REQ-013, REQ-014, REQ-015, REQ-016.)
- [ ] T-002.2 Verify exit-code propagation: fail each step in turn (bad build, no upload target, no VISA resource, configure failure, readback failure, byte mismatch) and confirm the script exits non-zero at the first failing step without masking. (Priority: Medium — est. ~30 min. Verifies REQ-016.)
- [ ] T-002.3 Verify `--debug` passthrough: run with `--debug` and confirm SCPI traffic is logged to stderr via decoder.py. (Priority: Medium — est. ~15 min. Verifies REQ-016.)

## 3. End-to-end HIL verification on real hardware

- [ ] T-003.1 Wire Nano default SPI pins to scope per channel map (CS=10→CHAN1, SCK=13→CHAN3, MISO=12→CHAN2, MOSI=11→CHAN4); connect Rigol over USBTMC; verify continuity and single-Rigol VISA presence. (Priority: High — est. ~20 min. Supports REQ-013, REQ-014.)
- [ ] T-003.2 Run `scripts/osci_hil_readback.sh` end-to-end; confirm exit 0 and decoded SPI events match expected bytes; correlate with Serial console output at 115200 baud. (Priority: High — est. ~30 min. Verifies REQ-011…REQ-016.)
- [ ] T-003.3 Test failure path: run with the scope disconnected (or wrong `--resource`) and confirm non-zero exit and a clear error. (Priority: Medium — est. ~15 min. Verifies REQ-013, REQ-016.)

## 4. Validation, records, and handoff

- [ ] T-004.1 Record a pass/fail verdict for every REQ ID (REQ-011…REQ-017) and list any deviations against `specs/hardware/spec.md`. (Priority: High — est. ~30 min.)
- [ ] T-004.2 Record status of open questions OQ-1…OQ-5 (proposal/design) and any new deviations for the engineer to review. (Priority: Medium — est. ~15 min.)

## Workflow follow-up

- The engineer publishes execution items derived from these tasks to the `gh` tracker (origin: github.com/nerdyscout/Arduino_MCP3x6x_Library), linked to this change folder and task IDs; requirement text is not duplicated in tracker items — links only.
- Review results with the user; any library behavior deviating from the spec becomes a separate change through grilling.
- Capture mode (`--mode capture`) is intentionally deferred; add in a follow-up change once readback is OK.
- Archive this change with `/opsx-archive` once review is complete (which also syncs delta specs to main `openspec/specs/`).
