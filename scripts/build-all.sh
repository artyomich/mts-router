#!/bin/bash
# MTS Router — Master build script
# Builds all device API implementations
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
LOG_DIR="${BUILD_DIR}/logs"
RESULTS_FILE="${BUILD_DIR}/build-results.json"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

mkdir -p "${BUILD_DIR}" "${LOG_DIR}"

# Initialize results
echo '{"timestamp": "'$(date -u +%Y-%m-%dT%H:%M:%SZ)'", "devices": {}, "total": 0, "success": 0, "failed": 0}' > "${RESULTS_FILE}"

log() {
    echo -e "${1}${2}${NC}"
}

log_status() {
    log "${YELLOW}[INFO]${NC} $1"
}

log_success() {
    log "${GREEN}[OK]${NC} $1"
}

log_error() {
    log "${RED}[FAIL]${NC} $1"
}

update_results() {
    local device="$1"
    local status="$2"
    local duration="$3"
    local log_file="$4"
    
    python3 -c "
import json, sys
with open('${RESULTS_FILE}', 'r') as f:
    data = json.load(f)
data['devices']['${device}'] = {
    'status': '${status}',
    'duration_seconds': ${duration},
    'log': '${log_file}'
}
data['total'] = data.get('total', 0) + 1
if '${status}' == 'success':
    data['success'] = data.get('success', 0) + 1
else:
    data['failed'] = data.get('failed', 0) + 1
with open('${RESULTS_FILE}', 'w') as f:
    json.dump(data, f, indent=2)
"
}

build_device() {
    local device_name="$1"
    local device_dir="$2"
    local build_script="$3"
    
    local start_time=$(date +%s)
    local log_file="${LOG_DIR}/${device_name}.log"
    
    log_status "Building ${device_name}..."
    
    if [ ! -d "${device_dir}" ]; then
        log_error "Directory not found: ${device_dir}"
        update_results "${device_name}" "skipped" 0 ""
        return 1
    fi
    
    if [ ! -f "${build_script}" ]; then
        log_error "Build script not found: ${build_script}"
        update_results "${device_name}" "skipped" 0 ""
        return 1
    fi
    
    if bash "${build_script}" 2>&1 | tee "${log_file}"; then
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        log_success "${device_name} built in ${duration}s"
        update_results "${device_name}" "success" "${duration}" "${log_file}"
        return 0
    else
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        log_error "${device_name} failed after ${duration}s"
        update_results "${device_name}" "failed" "${duration}" "${log_file}"
        return 1
    fi
}

build_all() {
    log_status "=== MTS Router — Full Build ==="
    log_status "Start time: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    
    local failed=0
    
    # Core Router (MTS-CR-9000)
    if build_device "mts-cr9000" "${PROJECT_DIR}/core-router-api" "${PROJECT_DIR}/scripts/build-core-router.sh"; then
        :
    else
        failed=$((failed + 1))
    fi
    
    # Mobile Core (MTS-MC-5000)
    if build_device "mts-mc5000" "${PROJECT_DIR}/mobile-core-api" "${PROJECT_DIR}/scripts/build-mobile-core.sh"; then
        :
    else
        failed=$((failed + 1))
    fi
    
    # Mobile Backhaul (MTS-MB-3000)
    if build_device "mts-mb3000" "${PROJECT_DIR}/mts-mb3000-api" "${PROJECT_DIR}/scripts/build-mobile-backhaul.sh"; then
        :
    else
        failed=$((failed + 1))
    fi
    
    # OLT GPON (MTS-OLT-2000)
    if build_device "mts-olt2000" "${PROJECT_DIR}/olt-gpon-api" "${PROJECT_DIR}/scripts/build-olt-gpon.sh"; then
        :
    else
        failed=$((failed + 1))
    fi
    
    # Enterprise Router (MTS-ER-1000)
    if build_device "mts-er1000" "${PROJECT_DIR}/enterprise-router-api" "${PROJECT_DIR}/scripts/build-enterprise.sh"; then
        :
    else
        failed=$((failed + 1))
    fi
    
    # Residential Gateway (MTS-RG-500)
    if build_device "mts-rg500" "${PROJECT_DIR}/residential-gateway-api" "${PROJECT_DIR}/scripts/build-residential.sh"; then
        :
    else
        failed=$((failed + 1))
    fi
    
    log_status "=== Build Complete ==="
    log_status "Total: $((6 - failed))/6 succeeded"
    
    if [ ${failed} -gt 0 ]; then
        log_error "${failed} device(s) failed"
        exit 1
    fi
}

# Parse arguments
case "${1:-all}" in
    all)
        build_all
        ;;
    core-router|mts-cr9000)
        build_device "mts-cr9000" "${PROJECT_DIR}/core-router-api" "${PROJECT_DIR}/scripts/build-core-router.sh"
        ;;
    mobile-core|mts-mc5000)
        build_device "mts-mc5000" "${PROJECT_DIR}/mobile-core-api" "${PROJECT_DIR}/scripts/build-mobile-core.sh"
        ;;
    mobile-backhaul|mts-mb3000)
        build_device "mts-mb3000" "${PROJECT_DIR}/mts-mb3000-api" "${PROJECT_DIR}/scripts/build-mobile-backhaul.sh"
        ;;
    olt-gpon|mts-olt2000)
        build_device "mts-olt2000" "${PROJECT_DIR}/olt-gpon-api" "${PROJECT_DIR}/scripts/build-olt-gpon.sh"
        ;;
    enterprise|mts-er1000)
        build_device "mts-er1000" "${PROJECT_DIR}/enterprise-router-api" "${PROJECT_DIR}/scripts/build-enterprise.sh"
        ;;
    residential|mts-rg500)
        build_device "mts-rg500" "${PROJECT_DIR}/residential-gateway-api" "${PROJECT_DIR}/scripts/build-residential.sh"
        ;;
    status)
        if [ -f "${RESULTS_FILE}" ]; then
            python3 -c "
import json
with open('${RESULTS_FILE}') as f:
    data = json.load(f)
print(f'Timestamp: {data[\"timestamp\"]}')
print(f'Total: {data[\"total\"]}, Success: {data[\"success\"]}, Failed: {data[\"failed\"]}')
print()
for name, info in data['devices'].items():
    status_icon = '✓' if info['status'] == 'success' else ('✗' if info['status'] == 'failed' else '-')
    print(f'  {status_icon} {name}: {info[\"status\"]} ({info[\"duration_seconds\"]}s)')
"
        else
            echo "No build results found. Run './scripts/build-all.sh' first."
        fi
        ;;
    clean)
        rm -rf "${BUILD_DIR}"
        echo "Build directory cleaned."
        ;;
    *)
        echo "Usage: $0 {all|core-router|mobile-core|mobile-backhaul|olt-gpon|enterprise|residential|status|clean}"
        exit 1
        ;;
esac
