#!/usr/bin/env bash
#
# osci_hil_readback.sh — 3-mode test orchestrator for MCP3x6x library.
#
# Modes:
#   mocked   Run native unit tests with ArduinoFake mocks (GitHub pipeline)
#   hil      Upload test_config.cpp to Nano, configure Rigol scope SPI decoder,
#            read back :ETABle1:DATA? events, compare MOSI bytes against expected
#            values printed by the test to Serial (single point of truth)
#   hardware Upload test_config.cpp to Nano, read Serial output, assert all tests pass
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
#
# Exit codes: 0 = all modes pass, non-zero = first failing mode/test
#

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${ROOT_DIR}/.." && pwd)"
DECODER="${OSCI_DECODER:-/home/stefan/.config/opencode/skills/scpi/scripts/decoder.py}"
TEST_FILE="${OSCI_TEST_FILE:-${REPO_DIR}/test/test_MCP3x6x/test_config.cpp}"
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

# Parse expected MOSI bytes from Serial output lines like:
#   TEST: begin ... MOSI: 78 79 PASS
parse_expected_mosi() {
  # parse_expected_mosi <serial_output>
  python3 -c "
import sys, re
text = sys.stdin.read()
tests = []
for line in text.splitlines():
    m = re.match(r'TEST:\s+(.+?)\s+\.\.\.\s+MOSI:\s+([0-9a-fA-F ]+)\s+PASS', line)
    if m:
        name = m.group(1)
        hexstr = m.group(2).replace(' ', '')
        tests.append((name, hexstr))
for name, hexstr in tests:
    print(f'{name}\t{hexstr}')
"
}

# Extract just the test names + PASS status from Serial output
parse_serial_tests() {
  python3 -c "
import sys, re
text = sys.stdin.read()
for line in text.splitlines():
    m = re.match(r'TEST:\s+(.+?)\s+\.\.\.\s+(PASS|FAIL)', line)
    if m:
        name = m.group(1)
        status = m.group(2)
        print(f'{name}\t{status}')
"
}

run_decoder_hex() {
  run_decoder --mode readback --etable on --mosi CHAN4 --miso CHAN2 2>&1 | python3 -c "
import sys, json
try:
    d = json.load(sys.stdin)
    rb = d.get('readback') or {}
    print(rb.get('hex', ''))
except Exception:
    print('')
"
}

# Check if a hex subsequence appears in a hex blob
hex_contains() {
  # hex_contains <blob_hex> <subseq_hex>
  python3 -c "
import sys
blob = sys.argv[1].lower().replace(' ', '').replace('\n', '')
sub = sys.argv[2].lower().replace(' ', '').replace('\n', '')
if not sub:
    sys.exit(0)  # empty subsequence matches
print('MATCH' if sub in blob else 'NOMATCH')
" "$1" "$2"
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

  # --- Step 1: Build + upload test_config.cpp hardware build ---
  echo "[Step 1/3] Build + upload test_config.cpp to ATmega328P"

  local bdir="${TMP_ROOT}/hil_upload"
  rm -rf "$bdir"
  mkdir -p "$bdir/src"

  cp "$TEST_FILE" "$bdir/src/"
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

  # --- Step 2: Configure scope SPI decoder + arm single capture ---
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

# Enable all 4 channels with 10x probe (standard scope probe)
# and appropriate vertical scale for 3.3V/5V logic signals (1V/div with 10x probe = 10V effective)
for ch in [1, 2, 3, 4]:
    write(f':CHAN{ch}:DISPlay ON')
    write(f':CHAN{ch}:PROB 10')
    write(f':CHAN{ch}:SCAL 1.0')
    write(f':CHAN{ch}:OFFS 0')

# Set timebase for 4MHz SPI capture (100ns/div = captures 4 clock cycles across screen)
write(':TIM:SCAL 100e-9')
write(':TIM:OFFS 0')

# Arm single sequence, trigger on CS falling edge
write(':TRIG:SWE SINGLE')
write(':TRIG:SOUR CHAN1')
write(':TRIG:EDGE:SLOP FALL')
write(':TRIG:LEV 1.2')
write(':SING')

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

  # --- Step 3: Read Serial output to get expected MOSI bytes ---
  echo
  echo "[Step 3a/4] Read Serial output — extract expected MOSI bytes from test prints"

  local serial_out
  serial_out=$(python3 -c "
import serial, time
port = '${OSCI_UPLOAD_PORT:-/dev/ttyUSB0}'
ser = serial.Serial(port, 115200, timeout=5)
time.sleep(8)
lines = []
deadline = time.time() + 30
while time.time() < deadline:
    while ser.in_waiting:
        line = ser.readline().decode('utf-8', errors='replace').strip()
        if line:
            lines.append(line)
            print(line, flush=True)
    if any('TEST_END' in l for l in lines):
        break
    time.sleep(0.1)
ser.close()
" 2>&1) || {
    echo "FAIL: serial read failed:" >&2
    echo "$serial_out" >&2
    return 3
  }

  echo "$serial_out"

  if ! echo "$serial_out" | grep -q "TEST_START"; then
    echo "FAIL: TEST_START not found in Serial output" >&2
    return 3
  fi
  if ! echo "$serial_out" | grep -q "TEST_END"; then
    echo "FAIL: TEST_END not found in Serial output" >&2
    return 3
  fi

  # Parse expected MOSI bytes from Serial
  local expected
  expected=$(echo "$serial_out" | parse_expected_mosi) || {
    echo "FAIL: could not parse expected MOSI bytes from Serial output" >&2
    return 3
  }

  if [ -z "$expected" ]; then
    echo "FAIL: no MOSI expectations found in Serial output" >&2
    return 3
  fi

  echo
  echo "Expected MOSI sequences from test prints:"
  echo "$expected"

  # --- Step 4: Read back :ETABle1:DATA? and compare MOSI bytes ---
  echo
  echo "[Step 4/4] Read back SPI events and compare to expected MOSI bytes"

  local hex_blob
  hex_blob=$(run_decoder_hex) || {
    echo "FAIL: decoder readback failed" >&2
    return 4
  }

  if [ -z "$hex_blob" ]; then
    echo "WARN: readback returned empty data" >&2
  else
    echo "Readback hex length: ${#hex_blob} chars"
  fi

  local failed=0
  local total=0
  while IFS=$'\t' read -r name hexstr; do
    [ -z "$name" ] && continue
    total=$((total + 1))
    if hex_contains "$hex_blob" "$hexstr"; then
      echo "  PASS: $name — MOSI $hexstr found in scope capture"
    else
      echo "  FAIL: $name — MOSI $hexstr NOT found in scope capture" >&2
      failed=$((failed + 1))
    fi
  done <<< "$expected"

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
import serial, time
port = '${OSCI_UPLOAD_PORT:-/dev/ttyUSB0}'
ser = serial.Serial(port, 115200, timeout=5)
time.sleep(8)
lines = []
deadline = time.time() + 30
while time.time() < deadline:
    while ser.in_waiting:
        line = ser.readline().decode('utf-8', errors='replace').strip()
        if line:
            lines.append(line)
            print(line, flush=True)
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

  if ! echo "$serial_out" | grep -q "TEST_START"; then
    echo "FAIL: TEST_START not found in Serial output" >&2
    return 2
  fi
  if ! echo "$serial_out" | grep -q "TEST_END"; then
    echo "FAIL: TEST_END not found in Serial output" >&2
    return 2
  fi

  # Parse test results from Serial
  local failed=0
  local total=0
  while IFS=$'\t' read -r name status; do
    [ -z "$name" ] && continue
    total=$((total + 1))
    if [ "$status" = "PASS" ]; then
      echo "  PASS: $name"
    else
      echo "  FAIL: $name — reported FAIL" >&2
      failed=$((failed + 1))
    fi
  done <<< "$(echo "$serial_out" | parse_serial_tests)"

  if [ "$failed" -gt 0 ]; then
    echo "FAIL: Hardware mode — $failed of $total tests failed" >&2
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
