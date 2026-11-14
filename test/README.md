# Testing

## Layout

- `test/test_mock/` — mocked unit tests (ArduinoFake + Unity). Only expectations that can be fully simulated.
- `test/test_hil/` — hardware-in-the-loop Unity tests (real board).
- `test/test_hil_spi/` — SPI communication verification (real board + oscilloscope).

## Mocked tests (default, no hardware)

```sh
pio test
# or
pio test -e mocked -f test_mock
```

Runs from `test/test_mock/` across MCP variants via `FOR_EACH_ADC`.

## HIL (real board)

```sh
pio run -t test_hil -e ATMEGA328P
```

Builds/uploads/runs the Unity HIL suite in `test/test_hil/` on the selected hardware env.

## HIL + SPI (real board + oscilloscope)

```sh
# Full build+upload+verify (includes pin-probe for channel mapping)
pio run -t test_hil_spi -e ATMEGA328P

# Fast verify on an already-flashed/running board (authoritative functional gate)
python3 test/test_hil_spi/verify_cmd.py
# env overrides: BOARD_PORT, OSCI_RESOURCE, HIL_SPI_CONFIG, VERBOSE=1, HIL_SPI_RESET=1
```

Behavior:
1. Pin-probe (full flow): toggles SS/SCK/MOSI/MISO with serial markers for scope channel mapping/verification.
2. Firmware steps one MCP3x6x fast command at a time via the library instance (`mcp.reset()`, `mcp.conversion()`, `mcp.standby()`, `mcp.shutdown()`, `mcp.full_shutdown()`) on host trigger.
3. **Primary gate:** host-paced verification reads true on-bus MOSI+MISO bytes from the Rigol DS1000Z internal decoder via `:ETABle1:DATA?` (`scripts/decoder.py`/`scripts/scpi.py`) and compares against expected pairs in `test/hil_spi_config.json` under one fixed scope/decoder setting (arm single on CS → step → readback). Firmware-emitted serial JSON is diagnostic.
4. Reusable scope settings: `test/hil_spi_config.json` (channels, volts/div, offset, trigger, timebase/acquire, decoder thresholds/mode). VISA/serial resources are closed after use.

Notes: `pio test -c platformio.ini -e hil_spi` uses a custom `test_testing_command` and may report `ERRORED`/`SIGHUP` even when `verify_cmd.py` (or `pio run -t test_hil_spi`) passes; treat `verify_cmd.py` / the custom target exit status as authoritative for the internal-decoder gate. Hardware/scope tiers require a connected bench and are not run in CI.
