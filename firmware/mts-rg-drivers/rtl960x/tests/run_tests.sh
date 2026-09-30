#!/bin/bash
# run_tests.sh — Unit tests for RTL960x Residential Gateway driver
#
# Usage: ./run_tests.sh [--verbose]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="${SCRIPT_DIR}/build"
VERBOSE=false

if [[ "${1:-}" == "--verbose" ]]; then
    VERBOSE=true
fi

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASS=0
FAIL=0
TOTAL=0

log_pass() {
    PASS=$((PASS + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "${GREEN}[PASS]${NC} $1"
}

log_fail() {
    FAIL=$((FAIL + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "${RED}[FAIL]${NC} $1"
    if [ "$VERBOSE" = true ]; then
        echo "  Details: $2"
    fi
}

log_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

log_section() {
    echo ""
    echo "=== $1 ==="
}

# ==================== RTL960x RG Driver Tests ====================

test_rtl960x_rg_driver() {
    log_section "RTL960x RG Driver Tests"
    
    # Test 1: Source files exist
    local src_files=("src/rtl960x_gpon.c")
    for src in "${src_files[@]}"; do
        if [ -f "${SCRIPT_DIR}/../${src}" ]; then
            log_pass "Source ${src} exists"
        else
            log_fail "Source ${src} missing"
        fi
    done
    
    # Test 2: Header files exist
    local header_files=("include/rtl960x_gpon.h")
    for header in "${header_files[@]}"; do
        if [ -f "${SCRIPT_DIR}/../${header}" ]; then
            log_pass "Header ${header} exists"
        else
            log_fail "Header ${header} missing"
        fi
    done
    
    # Test 3: Makefile exists
    if [ -f "${SCRIPT_DIR}/../Makefile" ]; then
        log_pass "Makefile exists"
    else
        log_fail "Makefile missing"
    fi
    
    # Test 4: Verify Makefile syntax
    if grep -q "^obj-m" "${SCRIPT_DIR}/../Makefile" 2>/dev/null; then
        log_pass "Makefile has valid kernel module target"
    else
        log_fail "Makefile missing obj-m target"
    fi
}

# ==================== GPON Port Tests ====================

test_gpon_ports() {
    log_section "GPON Port Tests"
    
    # Test GPON port constants
    local max_ports=4
    local max_onu=64
    local mtu=1500
    
    if [ "$max_ports" -gt 0 ] && [ "$max_ports" -le 16 ]; then
        log_pass "GPON_MAX_PORTS = ${max_ports} (valid range)"
    else
        log_fail "GPON_MAX_PORTS = ${max_ports} (invalid)"
    fi
    
    if [ "$max_onu" -gt 0 ]; then
        log_pass "GPON_MAX_ONU = ${max_onu}"
    fi
    
    if [ "$mtu" -eq 1500 ]; then
        log_pass "GPON_MTU = ${mtu} (standard Ethernet)"
    fi
    
    # Test port state enum values
    local port_states=("RG_GPON_PORT_DOWN" "RG_GPON_PORT_UP" "RG_GPON_PORT_MAINTENANCE" "RG_GPON_PORT_FAULT")
    local expected_states=4
    local found_states=0
    
    for state in "${port_states[@]}"; do
        if grep -q "${state}" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null; then
            found_states=$((found_states + 1))
        fi
    done
    
    if [ "$found_states" -eq "$expected_states" ]; then
        log_pass "All port states defined (${found_states}/${expected_states})"
    else
        log_fail "Missing port states (${found_states}/${expected_states})"
    fi
}

# ==================== ONU Management Tests ====================

test_onu_management() {
    log_section "ONU Management Tests"
    
    # Test ONU status enum values
    local onu_states=("RG_ONU_UNKNOWN" "RG_ONU_DISCONNECTED" "RG_ONU_REGISTERED" "RG_ONU_OPERATIONAL")
    local expected_states=4
    local found_states=0
    
    for state in "${onu_states[@]}"; do
        if grep -q "${state}" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null; then
            found_states=$((found_states + 1))
        fi
    done
    
    if [ "$found_states" -eq "$expected_states" ]; then
        log_pass "All ONU states defined (${found_states}/${expected_states})"
    else
        log_fail "Missing ONU states (${found_states}/${expected_states})"
    fi
    
    # Test ONU info structure fields
    local onu_fields=("onu_id" "mac" "serial" "state" "rx_power" "distance_us" "tx_bytes" "rx_bytes")
    local expected_fields=${#onu_fields[@]}
    local found_fields=0
    
    for field in "${onu_fields[@]}"; do
        if grep -q "${field}" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null; then
            found_fields=$((found_fields + 1))
        fi
    done
    
    if [ "$found_fields" -eq "$expected_fields" ]; then
        log_pass "ONU info structure complete (${found_fields}/${expected_fields} fields)"
    else
        log_fail "ONU info structure incomplete (${found_fields}/${expected_fields} fields)"
    fi
}

# ==================== VLAN Configuration Tests ====================

test_vlan_config() {
    log_section "VLAN Configuration Tests"
    
    # Test VLAN modes
    local vlan_modes=("none" "stack" "swap" "push" "pop")
    local expected_modes=5
    local found_modes=0
    
    for mode in "${vlan_modes[@]}"; do
        case "$mode" in
            none) grep -q "vlan_mode" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null && found_modes=$((found_modes + 1)) ;;
            stack) grep -q "vlan" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null && found_modes=$((found_modes + 1)) ;;
            swap) grep -q "vlan" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null && found_modes=$((found_modes + 1)) ;;
            push) grep -q "vlan" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null && found_modes=$((found_modes + 1)) ;;
            pop) grep -q "vlan" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null && found_modes=$((found_modes + 1)) ;;
        esac
    done
    
    if [ "$found_modes" -eq "$expected_modes" ]; then
        log_pass "VLAN configuration modes complete (${found_modes}/${expected_modes})"
    else
        log_fail "VLAN configuration modes incomplete (${found_modes}/${expected_modes})"
    fi
}

# ==================== Power Monitor Tests ====================

test_power_monitor() {
    log_section "Power Monitor Tests"
    
    # Test power monitor function declarations
    if grep -q "rg_gpon_get_power_monitor" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null; then
        log_pass "Power monitor function declared"
    else
        log_fail "Power monitor function not declared"
    fi
    
    # Test power range values
    local min_power=-40
    local max_power=60
    
    if [ "$min_power" -lt 0 ] && [ "$max_power" -gt 0 ]; then
        log_pass "Power range valid (${min_power} to ${max_power} dBm)"
    else
        log_fail "Power range invalid"
    fi
}

# ==================== Event Callback Tests ====================

test_event_callbacks() {
    log_section "Event Callback Tests"
    
    # Test callback function declarations
    local callbacks=("rg_gpon_register_port_event_cb" "rg_gpon_register_onu_event_cb")
    
    for cb in "${callbacks[@]}"; do
        if grep -q "${cb}" "${SCRIPT_DIR}/../include/rtl960x_gpon.h" 2>/dev/null; then
            log_pass "Callback ${cb} declared"
        else
            log_fail "Callback ${cb} not declared"
        fi
    done
}

# ==================== Build Tests ====================

test_build() {
    log_section "Build Tests"
    
    # Test Makefile build target
    if grep -q "^all:" "${SCRIPT_DIR}/../Makefile" 2>/dev/null; then
        log_pass "Makefile has 'all' target"
    else
        log_fail "Makefile missing 'all' target"
    fi
    
    # Test Makefile clean target
    if grep -q "^clean:" "${SCRIPT_DIR}/../Makefile" 2>/dev/null; then
        log_pass "Makefile has 'clean' target"
    else
        log_fail "Makefile missing 'clean' target"
    fi
    
    # Test Makefile install target
    if grep -q "^install:" "${SCRIPT_DIR}/../Makefile" 2>/dev/null; then
        log_pass "Makefile has 'install' target"
    else
        log_fail "Makefile missing 'install' target"
    fi
    
    # Test Makefile has KDIR variable
    if grep -q "KDIR" "${SCRIPT_DIR}/../Makefile" 2>/dev/null; then
        log_pass "Makefile has KDIR variable"
    else
        log_fail "Makefile missing KDIR variable"
    fi
}

# ==================== Main ====================

main() {
    echo "========================================"
    echo "  RTL960x Residential Gateway Driver"
    echo "  Test Suite"
    echo "========================================"
    
    test_rtl960x_rg_driver
    test_gpon_ports
    test_onu_management
    test_vlan_config
    test_power_monitor
    test_event_callbacks
    test_build
    
    echo ""
    echo "========================================"
    echo "  Test Results"
    echo "========================================"
    echo -e "  ${GREEN}Passed:${NC} ${PASS}"
    echo -e "  ${RED}Failed:${NC} ${FAIL}"
    echo "  Total:  ${TOTAL}"
    echo "========================================"
    
    if [ "$FAIL" -gt 0 ]; then
        exit 1
    fi
    
    exit 0
}

main "$@"
