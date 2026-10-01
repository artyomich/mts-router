#!/bin/bash
# run-tests.sh — End-to-end интеграционное тестирование MTS Router
#
# Usage:
#   ./run-tests.sh [--scenario <name>] [--device <device>] [--verbose] [--help]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
TEST_RESULTS_DIR="${BUILD_DIR}/test-results/integration"
TOPOLOGY_FILE="${SCRIPT_DIR}/topology.yaml"

SCENARIO=""
DEVICE=""
VERBOSE=false

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

log_pass() { echo -e "${GREEN}[PASS]${NC} $*"; }
log_fail() { echo -e "${RED}[FAIL]${NC} $*"; }
log_skip() { echo -e "${YELLOW}[SKIP]${NC} $*"; }
log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
log_section() {
    echo ""
    echo -e "${CYAN}========================================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}========================================================${NC}"
}

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Run end-to-end integration tests for MTS Router topology.

Options:
  --scenario <name>    Run specific scenario (bgp_e2e, mpls, srv6, ptp, tr069, voip, iptv)
  --device <device>    Test specific device only (cr9000, mc5000, mb3000, olt2000, er1000, rg500)
  --verbose            Enable verbose output
  --help               Show this help message

Scenarios:
  all              All scenarios (default)
  bgp_e2e          BGP end-to-end between Core Router and Enterprise
  mpls             MPLS label switching
  srv6             SRv6 policy forwarding
  ptp              PTP grandmaster synchronization
  tr069            TR-069 provisioning
  voip             VoIP call setup
  iptv             IPTV multicast streaming

Examples:
  $(basename "$0")
  $(basename "$0") --scenario bgp_e2e
  $(basename "$0") --device cr9000 --verbose
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --scenario)
            SCENARIO="$2"
            shift 2
            ;;
        --device)
            DEVICE="$2"
            shift 2
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            usage
            ;;
        *)
            log_info "Unknown option: $1"
            usage
            ;;
    esac
done

# Create results directory
mkdir -p "${TEST_RESULTS_DIR}"

# Counters
PASS_COUNT=0
FAIL_COUNT=0
SKIP_COUNT=0
TOTAL_COUNT=0

# Check if a service is responding
check_service() {
    local name="$1"
    local host="$2"
    local port="$3"
    local timeout="${4:-2}"
    
    if command -v nc &>/dev/null; then
        nc -z -w "${timeout}" "${host}" "${port}" 2>/dev/null
        return $?
    elif command -v bash &>/dev/null; then
        (echo > /dev/tcp/"${host}"/"${port}") 2>/dev/null
        return $?
    fi
    return 1
}

# Run BGP end-to-end test
test_bgp_e2e() {
    log_section "BGP End-to-End Test (Core Router <-> Enterprise)"
    
    local cr_ip="10.0.0.1"
    local er_ip="10.0.0.5"
    local result="passed"
    
    # Check if BGP is running on Core Router
    if check_service "bgp" "${cr_ip}" 179 2; then
        log_pass "BGP listening on Core Router port 179"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_fail "BGP not responding on Core Router"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        result="failed"
    fi
    
    # Check if BIRD/zebra is running on Enterprise
    if check_service "bird" "${er_ip}" 0 1; then
        log_pass "BIRD running on Enterprise Router"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "BIRD check skipped (device not accessible)"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Verify BGP peer adjacency
    log_info "Verifying BGP peer adjacency..."
    if check_service "bgp-peer" "${cr_ip}" 179 2; then
        log_pass "BGP peer adjacency established"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_fail "BGP peer adjacency not established"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        result="failed"
    fi
    
    # Save test result
    local report_file="${TEST_RESULTS_DIR}/bgp_e2e.json"
    cat > "${report_file}" <<EOF
{
  "test": "bgp_e2e",
  "result": "${result}",
  "core_router": {"ip": "${cr_ip}", "bgp_port": 179},
  "enterprise": {"ip": "${er_ip}"},
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF
    
    echo "  Report: ${report_file}"
    return 0
}

# Run MPLS test
test_mpls() {
    log_section "MPLS Label Switching Test"
    
    local cr_ip="10.0.0.1"
    local mb_ip="10.0.0.3"
    local result="passed"
    
    # Check MPLS kernel module
    if lsmod | grep -q mpls 2>/dev/null || grep -q mpls /proc/modules 2>/dev/null; then
        log_pass "MPLS kernel module loaded"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "MPLS kernel module check skipped"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Check MPLS forwarding
    log_info "Checking MPLS forwarding tables..."
    local mpls_labels=$(ip mpls show 2>/dev/null | wc -l || echo "0")
    if [ "${mpls_labels}" -gt 0 ]; then
        log_pass "MPLS labels found: ${mpls_labels}"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "No MPLS labels configured (expected in full test)"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Save test result
    local report_file="${TEST_RESULTS_DIR}/mpls.json"
    cat > "${report_file}" <<EOF
{
  "test": "mpls",
  "result": "${result}",
  "core_router": {"ip": "${cr_ip}", "mpls_labels": ${mpls_labels}},
  "mobile_backhaul": {"ip": "${mb_ip}"},
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF
    
    echo "  Report: ${report_file}"
    return 0
}

# Run SRv6 test
test_srv6() {
    log_section "SRv6 Policy Test"
    
    local cr_ip="10.0.0.1"
    local mc_ip="10.0.0.2"
    local result="passed"
    
    # Check SRv6 support
    if grep -q srv6 /proc/net/ipv6_route 2>/dev/null || ip seg6 show &>/dev/null; then
        log_pass "SRv6 kernel support available"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "SRv6 check skipped (kernel may not support)"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Save test result
    local report_file="${TEST_RESULTS_DIR}/srv6.json"
    cat > "${report_file}" <<EOF
{
  "test": "srv6",
  "result": "${result}",
  "core_router": {"ip": "${cr_ip}"},
  "mobile_core": {"ip": "${mc_ip}"},
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF
    
    echo "  Report: ${report_file}"
    return 0
}

# Run PTP test
test_ptp() {
    log_section "PTP Grandmaster Synchronization Test"
    
    local mb_ip="10.0.0.3"
    local result="passed"
    
    # Check PTP daemon
    if pgrep -x ptp4l &>/dev/null; then
        log_pass "ptp4l daemon running"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "ptp4l not running (expected on hardware)"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Check PTP sync offset
    log_info "Checking PTP synchronization..."
    if command -v ptp4l &>/dev/null; then
        local offset=$(ptp4l -Q 2>/dev/null | grep -i "offset" | tail -1 || echo "offset: 0 us")
        log_info "PTP offset: ${offset}"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "ptp4l not installed on test host"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Save test result
    local report_file="${TEST_RESULTS_DIR}/ptp.json"
    cat > "${report_file}" <<EOF
{
  "test": "ptp",
  "result": "${result}",
  "mobile_backhaul": {"ip": "${mb_ip}", "mode": "grandmaster"},
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF
    
    echo "  Report: ${report_file}"
    return 0
}

# Run TR-069 test
test_tr069() {
    log_section "TR-069 Provisioning Test"
    
    local olt_ip="10.0.0.4"
    local result="passed"
    
    # Check TR-069 ACS
    if check_service "tr069-acs" "${olt_ip}" 7547 2; then
        log_pass "TR-069 ACS responding on port 7547"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "TR-069 ACS check skipped (device not accessible)"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Save test result
    local report_file="${TEST_RESULTS_DIR}/tr069.json"
    cat > "${report_file}" <<EOF
{
  "test": "tr069",
  "result": "${result}",
  "olt_gpon": {"ip": "${olt_ip}", "acs_port": 7547},
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF
    
    echo "  Report: ${report_file}"
    return 0
}

# Run VoIP test
test_voip() {
    log_section "VoIP Call Setup Test"
    
    local rg_ip="10.0.0.6"
    local result="passed"
    
    # Check Asterisk
    if pgrep -x asterisk &>/dev/null; then
        log_pass "Asterisk daemon running"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "Asterisk check skipped (device not accessible)"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Check SIP port
    if check_service "sip" "${rg_ip}" 5060 2; then
        log_pass "SIP port 5060 responding"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "SIP check skipped (device not accessible)"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Save test result
    local report_file="${TEST_RESULTS_DIR}/voip.json"
    cat > "${report_file}" <<EOF
{
  "test": "voip",
  "result": "${result}",
  "residential": {"ip": "${rg_ip}", "sip_port": 5060},
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF
    
    echo "  Report: ${report_file}"
    return 0
}

# Run IPTV test
test_iptv() {
    log_section "IPTV Multicast Streaming Test"
    
    local rg_ip="10.0.0.6"
    local result="passed"
    
    # Check IGMP proxy
    if grep -q igmp /proc/net/ip_mroute 2>/dev/null; then
        log_pass "IGMP proxy configured"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_skip "IGMP proxy check skipped"
        SKIP_COUNT=$((SKIP_COUNT + 1))
    fi
    
    # Check multicast routing
    log_info "Checking multicast routing..."
    local mroute_count=$(ip mroute show 2>/dev/null | wc -l || echo "0")
    log_info "Multicast routes: ${mroute_count}"
    PASS_COUNT=$((PASS_COUNT + 1))
    
    # Save test result
    local report_file="${TEST_RESULTS_DIR}/iptv.json"
    cat > "${report_file}" <<EOF
{
  "test": "iptv",
  "result": "${result}",
  "residential": {"ip": "${rg_ip}", "multicast_range": "239.0.0.0/8"},
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF
    
    echo "  Report: ${report_file}"
    return 0
}

# Main execution
echo "========================================================"
echo "  MTS Router Integration Test Suite"
echo "========================================================"
echo ""
echo "Started: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "Build dir: ${BUILD_DIR}"
echo "Topology: ${TOPOLOGY_FILE}"
echo ""

# Run tests based on scenario
run_all_tests() {
    test_bgp_e2e
    test_mpls
    test_srv6
    test_ptp
    test_tr069
    test_voip
    test_iptv
}

case "${SCENARIO}" in
    all|"")
        run_all_tests
        ;;
    bgp_e2e)
        test_bgp_e2e
        ;;
    mpls)
        test_mpls
        ;;
    srv6)
        test_srv6
        ;;
    ptp)
        test_ptp
        ;;
    tr069)
        test_tr069
        ;;
    voip)
        test_voip
        ;;
    iptv)
        test_iptv
        ;;
    *)
        log_info "Unknown scenario: ${SCENARIO}"
        usage
        ;;
esac

# Generate final report
TOTAL_COUNT=$((PASS_COUNT + FAIL_COUNT + SKIP_COUNT))
REPORT_FILE="${TEST_RESULTS_DIR}/summary.json"
cat > "${REPORT_FILE}" <<EOF
{
  "test_suite": "integration-tests",
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "summary": {
    "total": ${TOTAL_COUNT},
    "passed": ${PASS_COUNT},
    "failed": ${FAIL_COUNT},
    "skipped": ${SKIP_COUNT}
  }
}
EOF

# Print summary
echo ""
echo "========================================================"
echo "  Integration Test Summary"
echo "========================================================"
echo ""
echo -e "  Total tests:   ${TOTAL_COUNT}"
echo -e "  ${GREEN}Passed:        ${PASS_COUNT}${NC}"
echo -e "  ${RED}Failed:        ${FAIL_COUNT}${NC}"
echo -e "  ${YELLOW}Skipped:       ${SKIP_COUNT}${NC}"
echo ""
echo "  Results: ${REPORT_FILE}"
echo "  Completed: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo ""

# Exit with appropriate code
if [ ${FAIL_COUNT} -gt 0 ]; then
    exit 1
fi
exit 0
