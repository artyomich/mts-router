#!/bin/bash
# test-integration.sh — Интеграционное тестирование всех устройств
#
# Usage:
#   ./test-integration.sh [--device <device>] [--verbose]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Test results
PASS=0
FAIL=0
SKIP=0
TOTAL=0

# Parse arguments
DEVICE=""
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --device)
            DEVICE="$2"
            shift 2
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--device <device>] [--verbose]"
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

# Logging functions
log_pass() {
    PASS=$((PASS + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "${GREEN}[PASS]${NC} $*"
}

log_fail() {
    FAIL=$((FAIL + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "${RED}[FAIL]${NC} $*"
}

log_skip() {
    SKIP=$((SKIP + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "${YELLOW}[SKIP]${NC} $*"
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $*"
}

log_section() {
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════${NC}"
}

# Check if a service is responding
check_service() {
    local name="$1"
    local host="$2"
    local port="$3"
    local timeout="${4:-2}"
    
    if nc -z -w "$timeout" "$host" "$port" 2>/dev/null; then
        return 0
    fi
    return 1
}

# Test Core Router integration
test_core_router() {
    log_section "Core Router (MTS-CR-9000) Integration Tests"
    
    # Check API server
    if check_service "core-router-api" "localhost" 50051; then
        log_pass "API server running on port 50051"
    else
        log_skip "API server not running on port 50051"
    fi
    
    # Check Yocto build files
    if [[ -f "${PROJECT_DIR}/core-router/linux/yocto-layer.md" ]]; then
        log_pass "Yocto layer spec exists"
    else
        log_fail "Yocto layer spec missing"
    fi
    
    # Check device tree
    if [[ -f "${PROJECT_DIR}/core-router/linux/device-tree.dts" ]]; then
        log_pass "Device tree exists"
    else
        log_fail "Device tree missing"
    fi
    
    # Check driver spec
    if [[ -f "${PROJECT_DIR}/core-router/firmware/driver-spec.md" ]]; then
        log_pass "Driver spec exists"
    else
        log_fail "Driver spec missing"
    fi
    
    # Check API implementation
    if [[ -d "${PROJECT_DIR}/core-router-api" ]]; then
        if [[ -f "${PROJECT_DIR}/core-router-api/CMakeLists.txt" ]]; then
            log_pass "CMakeLists.txt exists"
        else
            log_fail "CMakeLists.txt missing"
        fi
        if [[ -f "${PROJECT_DIR}/core-router-api/proto/mts_core_router.proto" ]]; then
            log_pass "Protobuf spec exists"
        else
            log_fail "Protobuf spec missing"
        fi
    else
        log_fail "API directory missing"
    fi
}

# Test Mobile Core integration
test_mobile_core() {
    log_section "Mobile Core (MTS-MC-5000) Integration Tests"
    
    # Check API server
    if check_service "mobile-core-api" "localhost" 50052; then
        log_pass "API server running on port 50052"
    else
        log_skip "API server not running on port 50052"
    fi
    
    # Check Yocto+K3s build files
    if [[ -f "${PROJECT_DIR}/mobile-core/linux/yocto-k3s-layer.md" ]]; then
        log_pass "Yocto+K3s layer spec exists"
    else
        log_fail "Yocto+K3s layer spec missing"
    fi
    
    # Check device tree
    if [[ -f "${PROJECT_DIR}/mobile-core/linux/device-tree.dts" ]]; then
        log_pass "Device tree exists"
    else
        log_fail "Device tree missing"
    fi
    
    # Check driver spec
    if [[ -f "${PROJECT_DIR}/mobile-core/firmware/driver-spec.md" ]]; then
        log_pass "Driver spec exists"
    else
        log_fail "Driver spec missing"
    fi
    
    # Check API implementation
    if [[ -d "${PROJECT_DIR}/mobile-core-api" ]]; then
        if [[ -f "${PROJECT_DIR}/mobile-core-api/CMakeLists.txt" ]]; then
            log_pass "CMakeLists.txt exists"
        else
            log_fail "CMakeLists.txt missing"
        fi
        if [[ -f "${PROJECT_DIR}/mobile-core-api/proto/mts_mobile_core.proto" ]]; then
            log_pass "Protobuf spec exists"
        else
            log_fail "Protobuf spec missing"
        fi
    else
        log_fail "API directory missing"
    fi
}

# Test Mobile Backhaul integration
test_mobile_backhaul() {
    log_section "Mobile Backhaul (MTS-MB-3000) Integration Tests"
    
    # Check API server
    if check_service "mobile-backhaul-api" "localhost"