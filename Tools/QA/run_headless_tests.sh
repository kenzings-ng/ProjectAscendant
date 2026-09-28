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
        *)
            echo "Unknown option: $1"
            echo "Usage: ./run_headless_tests.sh [--ue] [--all] [--filter=ProjectAscendant.Network]"
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
echo -e "\n[GATE 1/3] Running GDD Consistency Validator..."
if python3 "${PROJECT_ROOT}/Tools/QA/validate_gdd_consistency.py"; then
    echo ">> [PASS] GDD Consistency Gate"
else
    echo ">> [FAIL] GDD Consistency Gate"
    FAILED_GATES=$((FAILED_GATES + 1))
fi

# 2. Backend Postgres & Anti-Dupe Transaction Suite
echo -e "\n[GATE 2/3] Running Backend Postgres & Anti-Dupe Test Suite..."
if python3 "${PROJECT_ROOT}/Tools/QA/test_backend_postgres.py"; then
    echo ">> [PASS] Backend Database & Anti-Dupe Gate"
else
    echo ">> [FAIL] Backend Database & Anti-Dupe Gate"
    FAILED_GATES=$((FAILED_GATES + 1))
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

        # Prime render offload & headless flags
        export __NV_PRIME_RENDER_OFFLOAD=1
        export __GLX_VENDOR_LIBRARY_NAME=nvidia
        export SDL_VIDEODRIVER="offscreen"

        echo "[INFO] Executing headless tests in UnrealEditor..."
        set +e
        "${ENGINE_BIN}" "${UPROJECT}" \
            -nullrhi -nosound -unattended -nopause \
            -ExecCmds="Automation RunTests ${TEST_FILTER}; Quit" \
            -log="AutomationTest_Headless.log" > /dev/null 2>&1
        UE_EXIT=$?
        set -e

        # Parse test results from log if available
        if [ -f "${LOG_FILE}" ]; then
            TOTAL_PASS=$(grep -c "Automation Test Succeeded" "${LOG_FILE}" || true)
            TOTAL_FAIL=$(grep -c "Automation Test Failed" "${LOG_FILE}" || true)
            echo "UE Automation Summary: Passed=${TOTAL_PASS}, Failed=${TOTAL_FAIL}, ExitCode=${UE_EXIT}"
            if [ "${TOTAL_FAIL}" -gt 0 ] || [ "${UE_EXIT}" -ne 0 ]; then
                echo ">> [FAIL] UE Automation Gate"
                FAILED_GATES=$((FAILED_GATES + 1))
            else
                echo ">> [PASS] UE Automation Gate (${TOTAL_PASS} passed)"
            fi
        else
            if [ "${UE_EXIT}" -ne 0 ]; then
                echo ">> [FAIL] UE Process exited with code ${UE_EXIT}"
                FAILED_GATES=$((FAILED_GATES + 1))
            else
                echo ">> [PASS] UE Process completed successfully"
            fi
        fi
    fi
else
    echo -e "\n[GATE 3/3] Skipping Unreal Engine Automation Tests (use --ue or --all to run)."
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
