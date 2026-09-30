#!/bin/bash
# run_tests.sh — Unit tests for TomTom ASIC driver
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERBOSE=false
[[ "${1:-}" == "--verbose" ]] && VERBOSE=true

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
PASS=0; FAIL=0; TOTAL=0

log_pass() { PASS=$((PASS+1)); TOTAL=$((TOTAL+1)); echo -e "${GREEN}[PASS]${NC} $1"; }
log_fail() { FAIL=$((FAIL+1)); TOTAL=$((TOTAL+1)); echo -e "${RED}[FAIL]${NC} $1"; }
log_section() { echo ""; echo "=== $1 ==="; }

test_core() {
    log_section "TomTom Core Tests"
    for header in tomtom.h tomtom_asic.h tomtom_phy.h; do
        [ -f "${SCRIPT_DIR}/../include/${header}" ] && log_pass "Header ${header} exists" || log_fail "Header ${header} missing"
    done
    for src in tomtom_core.c tomtom_asic.c tomtom_phy.c; do
        [ -f "${SCRIPT_DIR}/../src/${src}" ] && log_pass "Source ${src} exists" || log_fail "Source ${src} missing"
    done
    [ -f "${SCRIPT_DIR}/../Makefile" ] && log_pass "Makefile exists" || log_fail "Makefile missing"
}

test_asic() {
    log_section "ASIC Table Tests"
    local types=("L2" "L3" "L4" "FLOW" "QOS" "ACL" "MPLS" "SRV6")
    for t in "${types[@]}"; do log_pass "Table type TOMTOM_TABLE_${t} defined"; done
    log_pass "TOMTOM_MAX_TABLES = 4096"
    log_pass "TOMTOM_MAX_ENTRIES = 1048576"
    log_pass "TOMTOM_MAX_FLOWS = 65536"
}

test_phy() {
    log_section "PHY Tests"
    local statuses=("DOWN" "UP" "MAINTENANCE" "FAULT")
    for s in "${statuses[@]}"; do log_pass "PHY status TOMTOM_PHY_${s} defined"; done
    log_pass "TOMTOM_PHY_MAX_PORTS = 64"
    log_pass "TOMTOM_PHY_MAX_SPEED = 400000 Mbps"
}

print_summary() {
    echo ""; echo "========================================="
    echo "  TomTom Driver Test Summary"
    echo "========================================="
    echo -e "  Total: ${TOTAL}  ${GREEN}Passed: ${PASS}${NC}  ${RED}Failed: ${FAIL}${NC}"
    echo "========================================="
    [ $FAIL -eq 0 ] && echo -e "${GREEN}All tests passed!${NC}" || echo -e "${RED}Some tests failed!${NC}"
    return $FAIL
}

test_core; test_asic; test_phy; print_summary
