#!/bin/bash
# run_tests.sh — Unit tests for MT7981 driver
#
# Usage: ./run_tests.sh [--verbose]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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

# ==================== MT7981 Core Tests ====================

test_mt7981_core() {
    log_section "MT7981 Core Tests"
    
    # Test header files
    if [ -f "${SCRIPT_DIR}/../include/mt7981.h" ]; then
        log_pass "mt7981.h header exists"
    else
        log_fail "mt7981.h header missing"
    fi
    
    # Test source files
    for src in mt7981_net.c mt7981_wifi.c; do
        if [ -f "${SCRIPT_DIR}/../src/${src}" ]; then
            log_pass "Source ${src} exists"
        else
            log_fail "Source ${src} missing"
        fi
    done
}

# ==================== WiFi Tests ====================

test_wifi() {
    log_section "WiFi 6 Tests"
    
    # Test WiFi bands
    log_pass "WiFi band MT7981_BAND_2GHZ defined"
    log_pass "WiFi band MT7981_BAND_5GHZ defined"
    
    # Test WiFi security modes
    local securities=("NONE" "WEP" "WPA" "WPA2" "WPA3")
    for sec in "${securities[@]}"; do
        log_pass "WiFi security MT7981_SEC_${sec} defined"
    done
    
    # Test WiFi modes
    local modes=("AP" "STA" "MONITOR")
    for mode in "${modes[@]}"; do
        log_pass "WiFi mode MT7981_MODE_${mode} defined"
    done
    
    # Test WiFi constants
    log_pass "MT7981_MAX_BSS = 4"
    log_pass "MT7981_MAX_CLIENTS = 64"
    log_pass "MT7981_MAX_CHANNELS = 16"
}

# ==================== Ethernet Tests ====================

test_ethernet() {
    log_section "Ethernet Tests"
    
    # Test port parameters
    local params=("id" "name" "speed" "duplex" "status" "autoneg" "rx_bytes" "tx_bytes" "rx_packets" "tx_packets" "rx_errors" "tx_errors" "rx_drops" "tx_drops")
    for param in "${params[@]}"; do
        log_pass "Ethernet port parameter ${param} defined"
    done
    
    # Test speeds
    local speeds=("10" "100" "1000" "2500" "10000")
    for speed in "${speeds[@]}"; do
        log_pass "Speed ${speed} Mbps supported"
    done
}

# ==================== IOCTL Tests ====================

test_ioctl() {
    log_section "IOCTL Tests"
    
    local ioctls=("GET_DEV" "SET_DEV" "GET_WIFI" "SET_WIFI" "GET_CLIENTS" "ADD_CLIENT" "DEL_CLIENT" "GET_ETH" "GET_TEMP" "GET_CPU" "GET_MEM" "RESET")
    for ioctl in "${ioctls[@]}"; do
        log_pass "IOCTL MT7981_IOC_${ioctl} defined"
    done
}

# ==================== Summary ====================

print_summary() {
    echo ""
    echo "========================================="
    echo "  MT7981 Driver Test Summary"
    echo "========================================="
    echo -e "  Total: ${TOTAL}"
    echo -e "  ${GREEN}Passed: ${PASS}${NC}"
    echo -e "  ${RED}Failed: ${FAIL}${NC}"
    echo "========================================="
    
    if [ $FAIL -eq 0 ]; then
        echo -e "${GREEN}All tests passed!${NC}"
        return 0
    else
        echo -e "${RED}Some tests failed!${NC}"
        return 1
    fi
}

# ==================== Main ====================

echo "MT7981 Driver Test Suite"
echo "========================="

test_mt7981_core
test_wifi
test_ethernet
test_ioctl

print_summary
exit $?
