#!/usr/bin/env bash
# ==============================================================================
# Project Ascendant - Automated QA & Headless Test Runner
# Runs Python validators, backend postgres tests, and UE automation tests.
# ==============================================================================

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
ENGINE_BIN="${UE_EDITOR_PATH:-/mnt/Data/Engine/Binaries/Linux/UnrealEditor}"
UPROJECT="${PROJECT_ROOT}/ProjectAscendant.uproject"

RUN_UE=0
UE_ONLY=0
TEST_FILTER="ProjectAscendant."

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --ue)
            RUN_UE=1
            shift
            ;;
        --filter=*)
            TEST_FILTER="${1#*=}"
            RUN_UE=1
            shift
            ;;
        --all)
            RUN_UE=1
            shift
            ;;
        --ue-only)
            # UE gate only (used by the CI ue-tests job; gates 1-2 run in the CI 'gates' job).
            RUN_UE=1
            UE_ONLY=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: ./run_headless_tests.sh [--ue] [--ue-only] [--all] [--filter=ProjectAscendant.Network]"
            exit 1
            ;;
    esac
done

echo "============================================================"
echo " Project Ascendant: Automated QA Test Suite"
echo " Project Root: ${PROJECT_ROOT}"
echo "============================================================"

FAILED_GATES=0

# 1. GDD Consistency Validator
if [ "${UE_ONLY}" -eq 1 ]; then
    echo -e "\n[GATE 1/3] GDD Consistency Validator skipped (--ue-only)."
    echo -e "\n[GATE 2/3] Backend Postgres Test Suite skipped (--ue-only)."
else
echo -e "\n[GATE 1/3] Running GDD Consistency Validator..."
if python3 "${PROJECT_ROOT}/Tools/QA/validate_gdd_consistency.py"; then
    echo ">> [PASS] GDD Consistency Gate"
else
    echo ">> [FAIL] GDD Consistency Gate"
    FAILED_GATES=$((FAILED_GATES + 1))
fi

# 2. Backend Postgres & Anti-Dupe Transaction Suite
echo -e "\n[GATE 2/3] Running Backend Postgres & Anti-Dupe Test Suite..."
set +e
python3 "${PROJECT_ROOT}/Tools/QA/test_backend_postgres.py"
PG_STATUS=$?
set -e

# Exit code 3 from test_backend_postgres.py == server genuinely unreachable (nothing listening).
# Only that case may be skipped, and only outside CI. Any other non-zero code is a real failure.
if [ "${PG_STATUS}" -eq 0 ]; then
    echo ">> [PASS] Backend Database & Anti-Dupe Gate"
elif [ "${PG_STATUS}" -eq 3 ] && [ "${CI:-}" != "true" ]; then
    echo ">> [WARN] Backend Database & Anti-Dupe Gate skipped locally (PostgreSQL server not reachable). Run 'docker compose up -d postgres' for local DB tests. Verified strictly in CI."
else
    echo ">> [FAIL] Backend Database & Anti-Dupe Gate (ExitCode=${PG_STATUS})"
    FAILED_GATES=$((FAILED_GATES + 1))
fi
fi

# 3. Unreal Engine Headless Automation Suite
if [ "${RUN_UE}" -eq 1 ]; then
    echo -e "\n[GATE 3/3] Running Unreal Engine Headless Automation Tests (${TEST_FILTER})..."
    if [ ! -f "${ENGINE_BIN}" ]; then
        echo "[ERROR] UnrealEditor binary not found at: ${ENGINE_BIN}"
        FAILED_GATES=$((FAILED_GATES + 1))
    else
        LOG_FILE="${PROJECT_ROOT}/Saved/Logs/AutomationTest_Headless.log"
        mkdir -p "${PROJECT_ROOT}/Saved/Logs"

        # Rotate any stale log so a previous run can never be parsed as the current result.
        if [ -f "${LOG_FILE}" ]; then
            mv -f "${LOG_FILE}" "${LOG_FILE%.log}-prev.log"
        fi

        # Prime render offload & headless flags
        export __NV_PRIME_RENDER_OFFLOAD=1
        export __GLX_VENDOR_LIBRARY_NAME=nvidia
        export SDL_VIDEODRIVER="offscreen"

        echo "[INFO] Executing headless tests in UnrealEditor..."
        set +e
        "${ENGINE_BIN}" "${UPROJECT}" \
            -nullrhi -nosound -unattended -nopause -ForceLogFlush \
            -ExecCmds="Automation RunTests ${TEST_FILTER}" \
            -TestExit="Automation Test Queue Empty" \
            -log="AutomationTest_Headless.log" > /dev/null 2>&1
        UE_EXIT=$?
        set -e

        # Parse test results from log if available
        if [ -f "${LOG_FILE}" ]; then
            TOTAL_DISCOVERED=$(grep -oP "Found \K[0-9]+(?= automation tests based on)" "${LOG_FILE}" | head -n 1 || true)
            TOTAL_PASS=$(grep -c -E "Result={Success}|Automation Test Succeeded" "${LOG_FILE}" || true)
            TOTAL_FAIL=$(grep -c -E "Result={Fail}|Automation Test Failed" "${LOG_FILE}" || true)
            QUEUE_EMPTY=$(grep -c -E "\.\.\.Automation Test Queue Empty [0-9]+ tests performed" "${LOG_FILE}" || true)
            TESTS_PERFORMED=$(grep -oP "\.\.\.Automation Test Queue Empty \K[0-9]+(?= tests performed)" "${LOG_FILE}" | tail -n 1 || true)
            ERROR_COUNT=$(grep -c -E "\]Log[a-zA-Z0-9_]+: (Error|Fatal):" "${LOG_FILE}" || true)

            echo "UE Automation Summary: Discovered=${TOTAL_DISCOVERED:-unknown}, Passed=${TOTAL_PASS}, Failed=${TOTAL_FAIL}, Errors=${ERROR_COUNT}, QueueFinished=${QUEUE_EMPTY}, ExitCode=${UE_EXIT}"

            # Validate that tests executed and all discovered tests completed.
            # Note: On Linux, UE5's -TestExit calls FPlatformMisc::RequestExit(true) which terminates via _exit(1).
            # When TOTAL_FAIL == 0, ERROR_COUNT == 0, QUEUE_EMPTY > 0, and all discovered tests passed,
            # ExitCode 1 is the expected Linux clean exit signal.
            if [ "${TOTAL_FAIL}" -gt 0 ] || [ "${TOTAL_PASS}" -le 0 ]; then
                echo ">> [FAIL] UE Automation Gate (Passed=${TOTAL_PASS}, Failed=${TOTAL_FAIL}, ExitCode=${UE_EXIT})"
                FAILED_GATES=$((FAILED_GATES + 1))
            elif [ "${ERROR_COUNT}" -gt 0 ]; then
                echo ">> [FAIL] UE Automation Gate: ${ERROR_COUNT} Error/Fatal log entries detected"
                FAILED_GATES=$((FAILED_GATES + 1))
            elif [ "${UE_EXIT}" -ne 0 ] && [ "${UE_EXIT}" -ne 1 ]; then
                echo ">> [FAIL] UE Automation Gate (Process crashed or terminated abnormally with ExitCode=${UE_EXIT})"
                FAILED_GATES=$((FAILED_GATES + 1))
            elif [ -z "${TOTAL_DISCOVERED}" ] || [ "${TOTAL_PASS}" -ne "${TOTAL_DISCOVERED}" ]; then
                echo ">> [FAIL] UE Automation Gate: Passed (${TOTAL_PASS}) does not equal Discovered (${TOTAL_DISCOVERED:-unknown})"
                FAILED_GATES=$((FAILED_GATES + 1))
            elif [ "${QUEUE_EMPTY}" -eq 0 ]; then
                echo ">> [FAIL] UE Automation Gate: Automation Test Queue did not complete fully."
                FAILED_GATES=$((FAILED_GATES + 1))
            elif [ -z "${TESTS_PERFORMED}" ] || [ "${TESTS_PERFORMED}" -ne "${TOTAL_DISCOVERED}" ]; then
                echo ">> [FAIL] UE Automation Gate: tests performed (${TESTS_PERFORMED:-unknown}) does not equal Discovered (${TOTAL_DISCOVERED})"
                FAILED_GATES=$((FAILED_GATES + 1))
            else
                if [ "${UE_EXIT}" -eq 1 ]; then
                    echo ">> [PASS] UE Automation Gate (${TOTAL_PASS}/${TOTAL_DISCOVERED:-${TOTAL_PASS}} passed, queue finished completely; ExitCode=1 confirmed as Linux UE5 -TestExit FPlatformMisc::RequestExit(true) clean exit with 0 errors/fatals)"
                else
                    echo ">> [PASS] UE Automation Gate (${TOTAL_PASS}/${TOTAL_DISCOVERED:-${TOTAL_PASS}} passed, queue finished completely)"
                fi
            fi
        else
            if [ "${UE_EXIT}" -ne 0 ]; then
                echo ">> [FAIL] UE Process exited with code ${UE_EXIT} (Log file missing)"
                FAILED_GATES=$((FAILED_GATES + 1))
            else
                echo ">> [FAIL] UE Automation log file not generated at ${LOG_FILE}"
                FAILED_GATES=$((FAILED_GATES + 1))
            fi
        fi
    fi
else
    echo -e "\n[GATE 3/3] UE Automation Tests skipped for fast check (run with --ue or --all for full PR merge verification)."
fi

echo -e "\n============================================================"
if [ "${FAILED_GATES}" -eq 0 ]; then
    echo ">> ALL AUTOMATED GATES PASSED SUCCESSFULLY (Exit 0) <<"
    echo "============================================================"
    exit 0
else
    echo ">> AUTOMATED GATES FAILED (${FAILED_GATES} failures) (Exit 1) <<"
    echo "============================================================"
    exit 1
fi
