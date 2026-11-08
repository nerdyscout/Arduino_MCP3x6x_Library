# Tasks

## 1. Environment and test vehicle

- [ ] T-001.1 Confirm PlatformIO `ATMEGA328P` toolchain installs and an empty sketch builds and uploads to the Nano — verify `pio run -e ATMEGA328P` exits 0 and upload target succeeds. (Priority: high — est. ~15 min. Verifies REQ-008.)
- [ ] T-001.2 Select/create the smoke-sketch test vehicle: `MCP3462 mcp;` (`MCP3x6x<16, 4>`), `begin()` then polling `analogRead(CH0..CH3)` in `loop()`, Serial at 115200 baud, no `attachIRQ`, no `attachMCLK`; create a minimal sketch under `examples/` if none fits. Verify it compiles for `ATMEGA328P`. (Priority: high — est. ~30 min. Verifies REQ-007, REQ-008.)

## 2. Hardware wiring and scope

- [ ] T-002.1 Wire Nano default SPI pins to the MCP3464 (CS=10, MOSI=11, MISO=12, SCK=13); no IRQ, no MCLK; analog inputs floating; verify continuity with a multimeter. (Priority: high — est. ~20 min.)
- [ ] T-002.2 Connect the scope to MISO/MOSI/CS/CLK; verify 4-channel acquisition and a working single-sequence trigger on CS falling edge. (Priority: high — est. ~15 min.)

## 3. Baseline library verification on scope

- [ ] T-003.1 Upload the smoke sketch; capture the `begin()` sequence: CMD_RESET 0x78, SREAD|RESERVED3 0x79, chip-ID nibble, CONFIG0 write 0x86 + 0xC0. Verify CS is LOW for the full transfer and HIGH between transactions, and confirm `begin()`'s return value via Serial. (Priority: high — est. ~30 min. Verifies REQ-001, REQ-002, REQ-003, REQ-004.)
- [ ] T-003.2 Capture `analogRead(CH0..CH3)`: MUX write 0x9A + 0x08, CONVERSION 0x68, ADCDATA read 0x81 + status byte + 2 MISO data bytes (big-endian wire order). Verify the sequence per channel and the Serial-printed raw values. (Priority: high — est. ~30 min. Verifies REQ-005, REQ-006, REQ-007.)
- [ ] T-003.3 Measure transaction duration from CS falling to CS rising (≤ 50 ms) and confirm CS idles HIGH between transactions. (Priority: medium — est. ~15 min. Verifies REQ-001, REQ-010.)

## 4. SCPI scope capture integration

- [ ] T-004.1 Attempt SCPI single-sequence CS-triggered capture + trace export via scpi-lab-automator; record whether the scope responds and the trace is decodable, plus the scope's SCPI command set used. (Priority: medium — est. ~45 min. Verifies REQ-009.)
- [ ] T-004.2 If SCPI capture fails, document the required scpi-lab-automator fixes as open items and fall back to manual scope capture/traces for the same byte verifications in T-003. (Priority: medium — est. ~30 min. Verifies REQ-009 via fallback.)

## 5. Results documentation and handoff

- [ ] T-005.1 Correlate decoded scope bytes with Serial console output (`begin()` result, per-channel raw values); record a pass/fail verdict for every REQ ID (REQ-001…REQ-010) and list deviations. (Priority: high — est. ~30 min.)
- [ ] T-005.2 Write a short results note in the change folder (deviations, status of OQ-1…OQ-4) for the engineer to review and commit. (Priority: high — est. ~15 min.)
- [ ] T-005.3 [engineer follow-up] Commit only the results note — no library code changes. (Priority: low — est. ~10 min.)

## Workflow follow-up

- Review results with the user; any library behavior deviating from the spec becomes a separate change through grilling.
- Archive this change with `/opsx-archive` once review is complete.
