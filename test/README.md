# Testing

## Mocked unit tests (CI — no hardware)

Run the mocked unit test suite via ArduinoFake + Unity:

```sh
pio test -e mocked
```

The mocked environment compiles and executes the same mock test suite for all
6 MCP variants via `FOR_EACH_ADC` macro expansion, verifying chip-ID validation
and per-variant register behavior.

## Hardware test (real board)

Build and upload the hardware smoke test to a physical board. The default
environment is `ATMEGA328P` (Arduino Nano); select the MCP variant with
`-DTEST_ADC_TYPE=<variant>`:

```sh
pio test -e ATMEGA328P
```

The hardware `setup()` runs `runUnityTests()` which prints PASS/FAIL for every
test to Serial at 115200 baud for oscilloscope correlation.

## Hardware-in-the-loop test (real board + Unity Serial readback)

Run the full HIL test on real hardware using Unity asserts read back via Serial:

```sh
# Run on default ATMEGA328P (uses MCP3564 variant)
pio run -t test_hil -e ATMEGA328P

# Run on any environment with a specific MCP variant
pio run -t test_hil -e ATMEGA328P --build-flags "-DTEST_ADC_TYPE=MCP3564"
```

This builds, uploads, and monitors the hardware test via Serial. `pio test`
parses the Unity output and reports results.

## SPI communication test (real board + decoder.py readback)

Verify real SPI transactions end-to-end using the `test_hil_spi` custom target,
which builds/uploads the hardware test, resets the board, reads back decoded SPI
events from a Rigol oscilloscope via `decoder.py`, and compares decoded MOSI
bytes against the expected table:

```sh
# Run on default ATMEGA328P / MCP3564
pio run -t test_hil_spi -e ATMEGA328P

# Run with verbose decoder output
pio run -t test_hil_spi -e ATMEGA328P --build-flags "-DTEST_ADC_TYPE=MCP3464"

# Scope VISA resource and serial port are auto-detected when exactly one
# candidate exists; otherwise use OSCI_RESOURCE / OSCI_PORT env vars.
```

The target verifies the decoded MOSI byte sequence: `reset()`, `chip-ID read`,
`config0()` through `config3()`, `irq()`, `mux()`, `conversion()`, and
`readADCDATA()`.

Options are controlled via the PlatformIO environment and build flags:
- `-e <env>` — PlatformIO environment (default: `ATMEGA328P`)
- `--build-flags "-DTEST_ADC_TYPE=<variant>"` — MCP variant (default: `MCP3564`)
- `OSCI_RESOURCE` — VISA resource string for the scope (auto-detected if single candidate)
- `OSCI_PORT` — Board serial port for DTR reset (auto-detected if single candidate)
