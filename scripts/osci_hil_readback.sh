#!/usr/bin/env bash
#
# osci_hil_readback.sh — HIL oscilloscope readback for MCP3x6x library tests.
#
# Builds and uploads a PlatformIO test <test>.cpp to ATmega328P (Arduino Nano),
# configures the Rigol oscilloscope SPI decoder over USBTMC, reads back
# :ETABle1:DATA? events, and compares decoded MOSI bytes against expected
# values printed by the test to Serial (single point of truth).
#
# Usage:
#   ./scripts/osci_hil_readback.sh --test tests/<file>.cpp [--debug]
#
# Environment:
#   OSCI_RESOURCE    VISA resource string for the Rigol (auto-detected if unset)
#   OSCI_UPLOAD_PORT Upload port for the Nano (e.g. /dev/ttyUSB1)
#   OSCI_DECODER     Path to decoder.py (default: bundled scpi skill script)
#
# Exit codes: 0 = all tests matched, non-zero = first failing test
#

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${ROOT_DIR}/.." && pwd)"
DECODER="${OSCI_DECODER:-/home/stefan/.config/opencode/skills/scpi/scripts/decoder.py}"
TEST_FILE="${1:-}"
TMP_ROOT="/tmp/opencode/mcp3x6x_hil"
DEBUG=0

while [ $# -gt 0 ]; do
  case "$1" in
    --test)
      shift
      TEST_FILE="${1:-}"
      ;;
    --debug)
      DEBUG=1
      ;;
    *)
      echo "Unknown argument: $1" >&2
      echo "Usage: $0 --test tests/<file>.cpp [--debug]" >&2
      exit 2
      ;;
  esac
  shift
done

if [ -z "$TEST_FILE" ]; then
  echo "Usage: $0 --test tests/<file>.cpp [--debug]" >&2
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

# Validate test file is under tests/
RELATIVE="${TEST_FILE#$REPO_DIR/}"
if [[ "$RELATIVE" != test/* ]]; then
  echo "error: test file must be under tests/ directory, got: $TEST_FILE" >&2
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

# Check if a hex subsequence appears in a hex blob
hex_contains() {
  python3 -c "
import sys
blob = sys.argv[1].lower().replace(' ', '').replace('\n', '')
sub = sys.argv[2].lower().replace(' ', '').replace('\n', '')
if not sub:
    sys.exit(0)
print('MATCH' if sub in blob else 'NOMATCH')
" "$1" "$2"
}

# ---------------------------------------------------------------------------
# HIL mode: build + upload test, configure scope decoder, read ETABLE, compare
# ---------------------------------------------------------------------------

run_hil() {
  echo "============================================================"
  echo " HIL readback — pio upload + Rigol scope SPI readback"
  echo "============================================================"
  echo
  echo "Test file: $TEST_FILE"
  echo

  # --- Step 1: Build + upload test to ATmega328P ---
  echo "[Step 1/4] Build + upload $TEST_FILE to ATmega328P"

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

  # --- Step 2: Read Serial output to get expected MOSI bytes ---
  echo
  echo "[Step 2/4] Read Serial output — extract expected MOSI bytes from test prints"

  local serial_out
  serial_out=$(python3 -c "
import serial, time
port = '${OSCI_UPLOAD_PORT:-/dev/ttyUSB0}'
ser = serial.Serial(port, 115200, timeout=5)
time.sleep(3)
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

  local expected
  expected=$(echo "$serial_out" | parse_expected_mosi) || {
    echo "FAIL: could not parse expected MOSI bytes from Serial output" >&2
    return 2
  }

  if [ -z "$expected" ]; then
    echo "FAIL: no MOSI expectations found in Serial output" >&2
    return 2
  fi

  echo
  echo "Expected MOSI sequences from test prints:"
  echo "$expected"

  # --- Step 3: Configure scope SPI decoder + arm single capture ---
  echo
  echo "[Step 3/4] Configure Rigol SPI decoder + arm single capture"

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

# Enable all 4 channels with 10x probe
for ch in [1, 2, 3, 4]:
    write(f':CHAN{ch}:DISPlay ON')
    write(f':CHAN{ch}:PROB 10')
    write(f':CHAN{ch}:SCAL 1.0')
    write(f':CHAN{ch}:OFFS 0')

# Set timebase for 4MHz SPI capture
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

  # Wait for scope to capture SPI events
  echo "Waiting for scope to capture SPI events..."
  sleep 5

  # Read back :ETABle1:DATA?
  echo "Reading :ETABle1:DATA?"
  local READBACK_OUT
  READBACK_OUT=$(run_decoder --mode readback --etable on --mosi CHAN4 --miso CHAN2 2>&1) || {
    echo "FAIL: decoder readback failed:" >&2
    echo "$READBACK_OUT" >&2
    return 3
  }

  local READBACK_OK
  READBACK_OK=$(echo "$READBACK_OUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print('OK' if d.get('ok') else 'BAD')") || {
    echo "FAIL: could not parse readback JSON" >&2
    echo "$READBACK_OUT" >&2
    return 3
  }

  if [ "$READBACK_OK" != "OK" ]; then
    echo "FAIL: decoder readback returned ok=false" >&2
    echo "$READBACK_OUT" >&2
    return 3
  fi

  local hex_blob byte_count
  hex_blob=$(echo "$READBACK_OUT" | python3 -c "
import sys,json
d=json.load(sys.stdin)
rb=d.get('readback') or {}
print(rb.get('hex',''))
")
  byte_count=$(echo "$READBACK_OUT" | python3 -c "
import sys,json
d=json.load(sys.stdin)
rb=d.get('readback') or {}
print(rb.get('bytes',0))
")

  echo "Readback hex length: ${#hex_blob} chars, bytes: $byte_count"

  if [ "$byte_count" -le 2 ]; then
    echo "FAIL: readback returned only header (no SPI events)" >&2
    echo "$READBACK_OUT" >&2
    return 3
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

run_hil
