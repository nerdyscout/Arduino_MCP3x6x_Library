#!/usr/bin/env bash
#
# upload.sh — Build and upload a single example to a board.
#
# Runs PlatformIO in place against the existing example files — no copying,
# no temporary config. Uses a temporary symlink in src/ so the example .ino
# is found by the normal src_dir = src/ mechanism.
#
# Usage:
#   ./upload.sh <example> [env]
#
# Examples:
#   ./upload.sh MCP3564_DMA             # build+upload to default env (ATMEGA328P)
#   ./upload.sh MCP3564_DMA SAMD21G18A  # build+upload to SAMD21
#
# Upload logs are written to logs/upload_<example>_<env>.log for analysis.
#

set -euo pipefail

EXAMPLES_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${EXAMPLES_DIR}/.." && pwd)"
PLATFORMIO_INI="${REPO_DIR}/platformio.ini"
LOGS_DIR="${REPO_DIR}/logs"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
now() { date '+%Y-%m-%d %H:%M:%S'; }
log() { echo "[$(now)] $*"; }

# ---------------------------------------------------------------------------
# Discover environments from platformio.ini
# ---------------------------------------------------------------------------
discover_envs() {
    grep -E '^\[env:' "${PLATFORMIO_INI}" | sed -E 's/^\[env:(.*)\]/\1/' | tr '\n' ' '
}

# ---------------------------------------------------------------------------
# Parse arguments
# ---------------------------------------------------------------------------
if [ $# -lt 1 ]; then
    echo "Usage: $0 <example> [env]" >&2
    echo "Available examples:" >&2
    ls -1d "${EXAMPLES_DIR}"/*/ | xargs -n1 basename | sed 's/^/\t/' >&2
    echo "Available environments:" >&2
    printf '\t%s\n' $(discover_envs) >&2
    exit 1
fi

EXAMPLE="$1"
shift

ENV=""
if [ $# -gt 0 ]; then
    ENV="$1"
fi

# Default env from platformio.ini default_envs
if [ -z "${ENV}" ]; then
    ENV="$(grep '^default_envs' "${PLATFORMIO_INI}" | sed -E 's/^default_envs *= *(.*)/\1/')"
fi

# Validate example
if [ ! -d "${EXAMPLES_DIR}/${EXAMPLE}" ]; then
    echo "ERROR: example not found: ${EXAMPLE}" >&2
    echo "Available examples:" >&2
    ls -1d "${EXAMPLES_DIR}"/*/ | xargs -n1 basename | sed 's/^/\t/' >&2
    exit 1
fi

# Validate example has .ino files
INO_COUNT=$(ls -1 "${EXAMPLES_DIR}/${EXAMPLE}"/*.ino 2>/dev/null | wc -l)
if [ "${INO_COUNT}" -eq 0 ]; then
    echo "ERROR: no .ino files in example: ${EXAMPLE}" >&2
    exit 1
fi

# Validate env
AVAILABLE_ENVS="$(discover_envs)"
if ! echo " ${AVAILABLE_ENVS} " | grep -q " ${ENV} "; then
    echo "ERROR: environment not found: ${ENV}" >&2
    echo "Available environments:" >&2
    printf '\t%s\n' ${AVAILABLE_ENVS} >&2
    exit 1
fi

# Find the .ino file (should match example dir name)
INO_FILE="${EXAMPLES_DIR}/${EXAMPLE}/${EXAMPLE}.ino"
if [ ! -f "${INO_FILE}" ]; then
    INO_FILE=$(ls "${EXAMPLES_DIR}/${EXAMPLE}"/*.ino 2>/dev/null | head -1)
    if [ -z "${INO_FILE}" ]; then
        echo "ERROR: no .ino file found in example: ${EXAMPLE}" >&2
        exit 1
    fi
fi
INO_BASENAME="$(basename "${INO_FILE}")"

# Create temporary symlink in src/ so PlatformIO finds the example .ino
SYMLINK="${REPO_DIR}/src/${INO_BASENAME}"
ln -sf "${INO_FILE}" "${SYMLINK}"

# Ensure symlink is removed on exit
trap 'rm -f "${SYMLINK}"' EXIT

mkdir -p "$LOGS_DIR"
LOGFILE="${LOGS_DIR}/upload_${EXAMPLE}_${ENV}.log"

log "Uploading example: ${EXAMPLE} to env: ${ENV}"
log "Symlink: src/${INO_BASENAME} -> ${INO_FILE}"
log "Log file: ${LOGFILE}"
log ">>> PLATFORMIO_EXTRA_SCRIPTS= pio run -c ${PLATFORMIO_INI} -e ${ENV} --target upload"

# Run pio run from repo root
# Override extra_scripts to empty (examples don't need HIL scripts)
set +e
PLATFORMIO_EXTRA_SCRIPTS= pio run -c "${PLATFORMIO_INI}" -e "${ENV}" --target upload 2>&1 | tee "$LOGFILE"
upload_status=$?
set -e

if [ "$upload_status" -eq 0 ]; then
    log "=== Upload complete: ${EXAMPLE} ==="
else
    log "=== Upload FAILED: ${EXAMPLE} (exit ${upload_status}) ==="
    exit "$upload_status"
fi
