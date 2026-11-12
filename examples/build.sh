#!/usr/bin/env bash
#
# build.sh — Build every example for every PlatformIO environment.
#
# Usage:
#   ./build.sh <env> <example> [...]        # build specified env(s) × example(s)
#   ./build.sh --upload <example> [env]     # build+upload one example
#
# Both at least one environment and at least one example must be given.
#
# The script auto-discovers:
#   * environments from [env:NAME] sections in platformio.ini
#   * examples from subdirectories of examples/
#
# Adding a new example or environment requires no script changes.
#
# Build logs are written to logs/<env>_<example>.log for later analysis.
#

set -euo pipefail

EXAMPLES_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${EXAMPLES_DIR}/.." && pwd)"
PLATFORMIO_INI="${REPO_DIR}/platformio.ini"
TMP_ROOT="${REPO_DIR}/.pio_build_scratch"
LOGS_DIR="${REPO_DIR}/logs"
mkdir -p "$LOGS_DIR"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
now() { date '+%Y-%m-%d %H:%M:%S'; }
log() { echo "[$(now)] $*"; }

# ---------------------------------------------------------------------------
# Discover environments from platformio.ini [env:NAME] sections
# ---------------------------------------------------------------------------
discover_envs() {
    grep -E '^\[env:' "${PLATFORMIO_INI}" | sed -E 's/^\[env:(.*)\]/\1/' | tr '\n' ' '
}

# ---------------------------------------------------------------------------
# Discover examples from examples/ subdirectories
# ---------------------------------------------------------------------------
discover_examples() {
    local dirs=()
    for d in "${EXAMPLES_DIR}"/*/; do
        dirs+=("$(basename "$d")")
    done
    printf '%s\n' "${dirs[@]}"
}

# ---------------------------------------------------------------------------
# Build one example for one environment using a scratch repo copy
# ---------------------------------------------------------------------------
build_one() {
    local env="$1"
    local example="$2"
    local bdir="${TMP_ROOT}/${env}_${example}"
    local logfile="${LOGS_DIR}/${env}_${example}.log"

    rm -rf "$bdir"
    mkdir -p "$bdir/src"

    # Copy library sources + manifest files into scratch build dir
    cp -r "${REPO_DIR}/src" "${REPO_DIR}/library.json" "${REPO_DIR}/library.properties" "${REPO_DIR}/platformio.ini" "$bdir/"

    # Examples don't need HIL extra_scripts (test/hil.py, test/hil_spi.py),
    # and the scratch dir has no test/ tree — drop them so builds don't fail.
    sed -i '/^extra_scripts *=/d' "${bdir}/platformio.ini"

    # Copy example .ino files into scratch src/
    local ino_count=0
    for f in "${EXAMPLES_DIR}/${example}"/*.ino; do
        [ -f "$f" ] || continue
        cp "$f" "$bdir/src/"
        ino_count=$((ino_count + 1))
    done

    if [ "$ino_count" -eq 0 ]; then
        echo "SKIP  [${env}] ${example}  (no .ino files)"
        echo "[$(now)] SKIP  [${env}] ${example}  (no .ino files)" > "$logfile"
        return 0
    fi

    log ">>> pio run -e ${env} -d ${bdir}  (example: ${example})"
    local start
    start=$(date +%s)

    local out
    if out=$(pio run -e "$env" -d "$bdir" 2>&1); then
        local elapsed
        elapsed=$(( $(date +%s) - start ))
        local status_line
        status_line=$(echo "$out" | grep -E '^\s+[0-9]+ succeeded' || true)
        echo "PASS  [${env}] ${example}  ${status_line}  (${elapsed}s)"
        echo "$out" > "$logfile"
        echo "[$(now)] PASS  [${env}] ${example}  (${elapsed}s)" >> "$logfile"
        return 0
    else
        local elapsed
        elapsed=$(( $(date +%s) - start ))
        echo "FAIL  [${env}] ${example}  (${elapsed}s)"
        echo "$out" > "$logfile"
        echo "[$(now)] FAIL  [${env}] ${example}  (${elapsed}s)" >> "$logfile"
        return 1
    fi
}

# ---------------------------------------------------------------------------
# Parse arguments
# ---------------------------------------------------------------------------
UPLOAD_EXAMPLE=""
UPLOAD_ENV=""
ENVS_REQUESTED=()
EXAMPLES_REQUESTED=()

while [ $# -gt 0 ]; do
    arg="$1"
    if [ "$arg" = "--upload" ]; then
        shift
        if [ $# -gt 0 ]; then
            UPLOAD_EXAMPLE="$1"
        fi
        shift
        if [ $# -gt 0 ] && [ "${1#-}" = "$1" ]; then
            UPLOAD_ENV="$1"
            shift
        fi
    elif [ -d "${EXAMPLES_DIR}/${arg}" ]; then
        EXAMPLES_REQUESTED+=("$arg")
        shift
    else
        ENVS_REQUESTED+=("$arg")
        shift
    fi
done

# Handle --upload mode
if [ -n "${UPLOAD_EXAMPLE}" ]; then
    log "=== Upload example: ${UPLOAD_EXAMPLE} ==="
    "${EXAMPLES_DIR}/upload.sh" "${UPLOAD_EXAMPLE}" "${UPLOAD_ENV}"
    exit $?
fi

ALL_ENVS=$(discover_envs)
ALL_EXAMPLES=$(discover_examples)

# Resolve envs
if [ ${#ENVS_REQUESTED[@]} -eq 0 ]; then
    log "ERROR: no environments specified. Available: "
    log "${ALL_ENVS}"
    exit 1
fi
ENVS=("${ENVS_REQUESTED[@]}")

# Resolve examples
if [ ${#EXAMPLES_REQUESTED[@]} -eq 0 ]; then
    log "ERROR: no examples specified. Available:"
    log "${ALL_EXAMPLES}"
    exit 1
fi
EXAMPLES=("${EXAMPLES_REQUESTED[@]}")

log "============================================================"
log " build.sh — example × environment build matrix"
log "============================================================"
log " Environments (${#ENVS[@]}):"
printf '\t%s\n' "${ENVS[@]}"
log " Examples      (${#EXAMPLES[@]}):"
printf '\t%s\n' "${EXAMPLES[@]}"
log " Logs dir: ${LOGS_DIR}"
log "============================================================"

PASS=0
FAIL=0
SKIP=0
TOTAL=0

for env in "${ENVS[@]}"; do
    if ! echo " $ALL_ENVS " | grep -q " $env "; then
        log "WARNING: environment '${env}' not found in platformio.ini — skipping"
        continue
    fi

    for example in "${EXAMPLES[@]}"; do
        if [ ! -d "${EXAMPLES_DIR}/${example}" ]; then
            log "WARNING: example '${example}' not found — skipping"
            continue
        fi

        TOTAL=$((TOTAL + 1))
        if build_one "$env" "$example"; then
            PASS=$((PASS + 1))
        else
            FAIL=$((FAIL + 1))
        fi
    done
done

log "============================================================"
log " Summary: ${TOTAL} builds, ${PASS} passed, ${FAIL} failed, ${SKIP} skipped"
log " Logs written to: ${LOGS_DIR}/"
log "============================================================"

if [ "$FAIL" -gt 0 ]; then
    log "Failed build logs are in ${LOGS_DIR}/<env>_<example>.log"
    exit 1
fi

exit 0
