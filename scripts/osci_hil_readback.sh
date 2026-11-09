#!/usr/bin/env bash
#
# osci_hil_readback.sh — 3-mode test orchestrator for MCP3x6x library.
#
# Modes:
#   mocked   Run native unit tests with ArduinoFake mocks (GitHub pipeline)
#   hil      Upload test_config.cpp to Nano, configure Rigol scope SPI decoder,
#            read back :ETABle1:DATA? events, compare MOSI bytes to expected
#   hardware Upload test_config.cpp to Nano, read Serial Unity output, assert pass
#   all      Run mocked -> hil -> hardware sequentially
#
# Usage:
#   ./osci_hil_readback.sh --mode mocked|hil|hardware|all [--debug]
#
# Environment:
#   OSCI_RESOURCE    VISA resource string for the Rigol (auto-detected if unset)
#   OSCI_UPLOAD_PORT Upload port for the Nano (e.g. /dev/ttyUSB1)
#   OSCI_DECODER     Path to decoder.py (default: bundled scpi skill script)
#   OSCI_TEST_FILE   Path to test_config.cpp (default: test/test_MCP3x6x/test_config.cpp)
#   OSCI_EXPECTED    Path to expected_values.json (default: scripts/expected_values.json)
#
# Exit codes: 0 = all modes pass, non-zero = first failing mode/test
#

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${ROOT_DIR}/.." && pwd)"
DECODER="${OSCI_DECODER:-/home/stefan/.config/opencode/skills/scpi/scripts/decoder.py}"
TEST_FILE="${OSCI_TEST_FILE:-${REPO_DIR}/test/test_MCP3x6x/test_config.cpp}"
EXPECTED="${OSCI_EXPECTED:-${ROOT_DIR}/expected_values.json}"
TMP_ROOT="/tmp/opencode/mcp3x6x_hil"

MODE=""
DEBUG=0

while [ $# -gt 0 ]; do
  case "$1" in
    --mode)
      shift
      MODE="${1:-}"
      ;;
    --debug)
      DEBUG=1
      ;;
    *)
      echo "Unknown argument: $1" >&2
      exit 2
      ;;
  esac
  shift
done

if [ -z "$MODE" ]; then
  echo "usage: $0 --mode mocked|hil|hardware|all [--debug]" >&2
  exit 2
fi

if [ ! -f "$DECODER" ]; then
  echo "error: decoder.py not found at $DECODER" >&2
  exit 3
fi

if [ ! -f "$TEST_FILE" ]; then
  echo "error: test file not found at $TEST_FILE" >&2
  exit 4
fi

if [ ! -f "$EXPECTED" ]; then
  echo "error: expected values not found at $EXPECTED" >&2
  exit 5
fi

RESOURCE_ARG=()
if [ -n "${OSCI_RESOURCE:-}" ]; then
  RESOURCE_ARG=(--resource "$OSCI_RESOURCE")
fi

UPLOAD_ARG=()
if [ -n "${OSCI_UPLOAD_PORT:-}" ]; then
  UPLOAD_ARG=(--upload-port "${OSCI_UPLOAD_PORT}")
fi

DEBUG_ARG=()
if [ "$DEBUG" -eq 1 ]; then
  DEBUG_ARG=(--debug)
fi

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

run_decoder() {
  python3 "$DECODER" --protocol spi --decoder 1 --json "${DEBUG_ARG[@]:-}" "${RESOURCE_ARG[@]:-}" "$@"
}

json_extract() {
  # json_extract <json_file> <jq_path>
  python3 -c "
import json, sys
with open(sys.argv[1]) as f:
    d = json.load(f)
val = d
for part in sys.argv[2].split('.'):
    val = val[part]
print(val)
" "$1" "$2"
}

json_length() {
  python3 -c "
import json, sys
with open(sys.argv[1]) as f:
    d = json.load(f)
print(len(d[sys.argv[2]]))
" "$1" "$2"
}

json_item() {
  # json_item <json_file> <array_path> <index> <field>
  python3 -c "
import json, sys
with open(sys.argv[1]) as f:
    d = json.load(f)
val = d
for part in sys.argv[2].split('.'):
    val = val[part]
item = val[int(sys.argv[3])]
print(item[sys.argv[4]])
" "$1" "$2" "$3" "$4"
}

# ---------------------------------------------------------------------------
# Mode 1: Mocked (GitHub pipeline)
# ---------------------------------------------------------------------------

run_mocked() {
  echo "============================================================"
  echo " MODE: mocked — native unit tests with ArduinoFake"
  echo "============================================================"
  echo

  if pio test -e native; then
    echo "PASS: mocked mode — all native unit tests passed"
    return 0
  else
    echo "FAIL: mocked mode — native unit tests failed" >&2
    return 1
  fi
}

# ---------------------------------------------------------------------------
# Mode 2: HIL (oscilloscope readback)
# ---------------------------------------------------------------------------

run_hil() {
  echo "============================================================"
  echo " MODE: hil — pio upload + Rigol scope SPI readback"
  echo "============================================================"
  echo

  local test_count
  test_count=$(json_length "$EXPECTED" "tests")
  echo "Total tests to verify: $test_count"
  echo

  # --- Step 1: Build + upload test_config.cpp hardware build ---
  echo "[Step 1/3] Build + upload test_config.cpp to ATmega328P"

  local bdir="${TMP_ROOT}/hil_upload"
  rm -rf "$bdir"
  mkdir -p "$bdir/src"

  cp "$TEST_FILE" "$bdir/src/"
  # Copy test.h alongside test_config.cpp so #include "test.h" resolves
  cp "$(dirname "$TEST_FILE")/test.h" "$bdir/src/"
  cp "$REPO_DIR/library.json" "$REPO_DIR/library.properties" "$REPO_DIR/platformio.ini" "$bdir/"
  cp -r "$REPO_DIR/src" "$bdir/"

  cat > "$bdir/platformio.ini" <<'PIOEOF'
[platformio]
description = HIL test upload for ATmega328P (Arduino Nano)

[env:ATMEGA328P]
platform = atmelavr
board = nanoatmega328
framework = arduino
monitor_speed = 115200
build_flags = -DTEST_ADC_TYPE=MCP3564
upload_flags = -V
PIOEOF

  if ! (pio run -e ATMEGA328P -d "$bdir" --target upload "${UPLOAD_ARG[@]:-}"); then
    echo "FAIL: build or upload failed" >&2
    return 1
  fi
  echo "PASS: build + upload succeeded"

  # --- Step 2: Configure scope SPI decoder + arm single sequence ---
  echo
  echo "[Step 2/3] Configure Rigol SPI decoder + arm single capture"

  local configure_errs
  configure_errs=$(python3 -c "
import pyvisa, sys
rm = pyvisa.ResourceManager('@py')
inst = rm.open_resource('${OSCI_RESOURCE}')
inst.timeout = 5000
inst.chunk_size = 64
errors = []
def write(cmd):
    inst.write(cmd)
def query(cmd):
    return inst.query(cmd).strip()

# Configure SPI decoder
write(':DECoder1:MODE SPI')
write(':DECoder1:SPI:CLK CHAN3')
write(':DECoder1:SPI:MOSI CHAN4')
write(':DECoder1:SPI:MISO CHAN2')
write(':DECoder1:SPI:CS CHAN1')
write(':DECoder1:SPI:WIDTh 8')
write(':DECoder1:SPI:EDGE RISE')
write(':DECoder1:SPI:ENDian MSB')
write(':DECoder1:SPI:SELect CS')
write(':CHAN1:PROB 1')  # 1x probe, direct connection

# Arm single sequence, trigger on CS falling edge
write(':TRIG:SWE SINGLE')
write(':TRIG:SOUR CHAN1')
write(':TRIG:EDGE:SLOP FALL')
write(':TRIG:LEV 1.2')
write(':SING')

# Verify settings
checks = {
    ':DECoder1:MODE?': 'SPI',
    ':DECoder1:SPI:CLK?': 'CHAN3',
    ':DECoder1:SPI:MOSI?': 'CHAN4',
    ':DECoder1:SPI:MISO?': 'CHAN2',
    ':DECoder1:SPI:CS?': 'CHAN1',
    ':CHAN1:PROB?': '1',
    ':TRIG:SWE?': 'SING',
    ':TRIG:SOUR?': 'CHAN1',
    ':TRIG:EDGE:SLOP?': 'NEG',
}
for cmd, expected in checks.items():
    try:
        actual = query(cmd)
        if actual.upper() != expected.upper():
            errors.append(f'{cmd}: expected={expected} actual={actual}')
    except Exception as exc:
        errors.append(f'{cmd}: readback failed ({exc})')

inst.close()
if errors:
    print('CONFIG_ERRORS:' + '; '.join(errors))
    sys.exit(1)
print('CONFIG_OK')
" 2>&1) || {
    echo "FAIL: SPI decoder configuration failed:" >&2
    echo "$configure_errs" >&2
    return 2
  }
  echo "$configure_errs" | grep -q "CONFIG_OK" && echo "PASS: scope SPI decoder configured and armed" || {
    echo "FAIL: scope SPI decoder configuration verification failed" >&2
    echo "$configure_errs" >&2
    return 2
  }

  # --- Step 3: Read back :ETABle1:DATA? and compare MOSI bytes ---
  echo
  echo "[Step 3/3] Read back SPI events and compare to expected values"

  local readback_out
  readback_out=$(run_decoder --mode readback --etable on --mosi CHAN4 --miso CHAN2 2>&1) || {
    echo "FAIL: decoder readback failed:" >&2
    echo "$readback_out" >&2
    return 3
  }

  local readback_ok
  readback_ok=$(echo "$readback_out" | python3 -c "import sys,json; d=json.load(sys.stdin); print('OK' if d.get('ok') else 'BAD')") || {
    echo "FAIL: could not parse readback JSON" >&2
    echo "$readback_out" >&2
    return 3
  }

  if [ "$readback_ok" != "OK" ]; then
    echo "FAIL: decoder readback returned ok=false" >&2
    echo "$readback_out" >&2
    return 3
  fi

  local hex_blob byte_count
  hex_blob=$(echo "$readback_out" | python3 -c "
import sys,json
d=json.load(sys.stdin)
rb=d.get('readback') or {}
print(rb.get('hex',''))
")
  byte_count=$(echo "$readback_out" | python3 -c "
import sys,json
d=json.load(sys.stdin)
rb=d.get('readback') or {}
print(rb.get('bytes',0))
")

  if [ "$byte_count" -le 2 ]; then
    echo "WARN: readback returned only $byte_count bytes (header only, no SPI events)" >&2
    echo "      Ensure scope CHAN1 (CS=pin10) is wired and toggling"
    echo "      Hex blob: $hex_blob"
  else
    echo "PASS: readback returned $byte_count bytes of SPI event data"
  fi

  # Decode hex blob to bytes and compare against expected
  local hex_bytes
  hex_bytes=$(echo "$hex_blob" | python3 -c "
import sys
h = sys.stdin.read().strip()
# Remove any non-hex chars
h = ''.join(c for c in h if c in '0123456789abcdefABCDEF')
print(h)
")

  local failed=0
  local total=$(json_length "$EXPECTED" "tests")
  for ((i=0; i<total; i++)); do
    local name mosi_hex mosi_count
    name=$(json_item "$EXPECTED" "tests" "$i" "name")
    mosi_hex=$(json_item "$EXPECTED" "tests" "$i" "spi_mosi_bytes" | python3 -c "
import sys
b = sys.stdin.read().strip()
vals = [int(x) for x in b.split(',')]
print(''.join(f'{v:02x}' for v in vals))
")
    mosi_count=$(python3 -c "
import sys
b = sys.stdin.read().strip()
vals = [int(x) for x in b.split(',')]
print(len(vals))
" <<< "$(json_item "$EXPECTED" "tests" "$i" "spi_mosi_bytes")")

    # Check if the expected MOSI sequence appears in the readback hex
    if echo "$hex_bytes" | grep -qi "$mosi_hex"; then
      echo "  PASS: $name — MOSI bytes $mosi_hex found in scope capture"
    else
      echo "  FAIL: $name — MOSI bytes $mosi_hex NOT found in scope capture" >&2
      failed=$((failed + 1))
    fi
  done

  if [ "$failed" -gt 0 ]; then
    echo "FAIL: HIL mode — $failed of $total tests failed byte comparison" >&2
    return 4
  fi

  echo "PASS: HIL mode — all $total tests matched expected MOSI bytes"
  return 0
}

# ---------------------------------------------------------------------------
# Mode 3: Hardware (Serial Unity assert)
# ---------------------------------------------------------------------------

run_hardware() {
  echo "============================================================"
  echo " MODE: hardware — pio upload + Serial Unity assert"
  echo "============================================================"
  echo

  # --- Step 1: Build + upload ---
  echo "[Step 1/2] Build + upload test_config.cpp to ATmega328P"

  local bdir="${TMP_ROOT}/hardware_upload"
  rm -rf "$bdir"
  mkdir -p "$bdir/src"

  cp "$TEST_FILE" "$bdir/src/"
  # Copy test.h alongside test_config.cpp so #include "test.h" resolves
  cp "$(dirname "$TEST_FILE")/test.h" "$bdir/src/"
  cp "$REPO_DIR/library.json" "$REPO_DIR/library.properties" "$REPO_DIR/platformio.ini" "$bdir/"
  cp -r "$REPO_DIR/src" "$bdir/"

  cat > "$bdir/platformio.ini" <<'PIOEOF'
[platformio]
description = Hardware test upload for ATmega328P (Arduino Nano)

[env:ATMEGA328P]
platform = atmelavr
board = nanoatmega328
framework = arduino
monitor_speed = 115200
build_flags = -DTEST_ADC_TYPE=MCP3564
upload_flags = -V
PIOEOF

  if ! (pio run -e ATMEGA328P -d "$bdir" --target upload "${UPLOAD_ARG[@]:-}"); then
    echo "FAIL: build or upload failed" >&2
    return 1
  fi
  echo "PASS: build + upload succeeded"

  # --- Step 2: Read Serial output and assert ---
  echo
  echo "[Step 2/2] Read Serial output and assert all tests pass"

  local serial_out
  serial_out=$(python3 -c "
import serial, time, sys
port = '${OSCI_UPLOAD_PORT}'
ser = serial.Serial(port, 115200, timeout=5)
time.sleep(3)
ser.reset_input_buffer()
time.sleep(5)
lines = []
deadline = time.time() + 30
while time.time() < deadline:
    while ser.in_waiting:
        line = ser.readline().decode('utf-8', errors='replace').strip()
        if line:
            lines.append(line)
            print(line)
    if any('TEST_END' in l for l in lines):
        break
    time.sleep(0.1)
ser.close()
" 2>&1) || {
    echo "FAIL: serial read failed:" >&2
    echo "$serial_out" >&2
    return 2
  }

  echo "$serial_out"

  # Check TEST_START and TEST_END markers
  if ! echo "$serial_out" | grep -q "TEST_START"; then
    echo "FAIL: TEST_START not found in Serial output" >&2
    return 2
  fi
  if ! echo "$serial_out" | grep -q "TEST_END"; then
    echo "FAIL: TEST_END not found in Serial output" >&2
    return 2
  fi

  # Check all expected test assertions
  local failed=0
  local total=$(json_length "$EXPECTED" "tests")
  for ((i=0; i<total; i++)); do
    local name assert_str
    name=$(json_item "$EXPECTED" "tests" "$i" "name")
    assert_str=$(json_item "$EXPECTED" "tests" "$i" "serial_assert")

    if echo "$serial_out" | grep -qF "$assert_str"; then
      echo "  PASS: $name — '$assert_str' found"
    else
      echo "  FAIL: $name — '$assert_str' NOT found in Serial output" >&2
      failed=$((failed + 1))
    fi
  done

  if [ "$failed" -gt 0 ]; then
    echo "FAIL: Hardware mode — $failed of $total tests failed serial assert" >&2
    return 2
  fi

  echo "PASS: Hardware mode — all $total tests passed via Serial assert"
  return 0
}

# ---------------------------------------------------------------------------
# Main dispatch
# ---------------------------------------------------------------------------

case "$MODE" in
  mocked)
    run_mocked
    ;;
  hil)
    run_hil
    ;;
  hardware)
    run_hardware
    ;;
  all)
    echo "Running all modes: mocked -> hil -> hardware"
    echo

    run_mocked || { echo "FAIL: mocked mode failed, stopping" >&2; exit 1; }
    echo

    run_hil || { echo "FAIL: hil mode failed, stopping" >&2; exit 1; }
    echo

    run_hardware || { echo "FAIL: hardware mode failed, stopping" >&2; exit 1; }
    echo

    echo "============================================================"
    echo " ALL MODES PASSED"
    echo "============================================================"
    ;;
  *)
    echo "Unknown mode: $MODE (use mocked, hil, hardware, or all)" >&2
    exit 2
    ;;
esac
