#!/bin/bash
# MTS Router — Networking Stack Test Suite
# Tests networking functionality for all MTS Router devices
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
RESULTS_DIR="${BUILD_DIR}/test-results/networking"
LOG_DIR="${RESULTS_DIR}/logs"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PASS=0
FAIL=0
SKIP=0

mkdir -p "${RESULTS_DIR}" "${LOG_DIR}"

log() { echo -e "${GREEN}[$(date +%H:%M:%S)]${NC} $1"; }
log_error() { echo -e "${RED}[FAIL]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }

record_result() {
    local test_name="$1"
    local status="$2"
    local message="$3"
    local device="${4:-all}"
    
    local result_file="${RESULTS_DIR}/${device}-networking-results.json"
    
    if [ ! -f "${result_file}" ]; then
        echo '{"device": "'${device}'", "tests": [], "timestamp": "'$(date -u +%Y-%m-%dT%H:%M:%SZ)'", "pass": 0, "fail": 0, "skip": 0}' > "${result_file}"
    fi
    
    case "${status}" in
        pass) PASS=$((PASS + 1)) ;;
        fail) FAIL=$((FAIL + 1)) ;;
        skip) SKIP=$((SKIP + 1)) ;;
    esac
    
    log_info "Test [${status^^}]: ${test_name} - ${message}"
}

# ============================================================
# Device list
# ============================================================
declare -A DEVICES
DEVICES=(
    ["mts-cr9000"]="Core Router"
    ["mts-mc5000"]="Mobile Core"
    ["mts-mb3000"]="Mobile Backhaul"
    ["mts-olt2000"]="OLT GPON"
    ["mts-er1000"]="Enterprise Router"
    ["mts-rg500"]="Residential Gateway"
)

# ============================================================
# Test functions
# ============================================================

# Test 1: IP stack verification
test_ip_stack() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local rootfs_dir=""
    
    log_info "Testing IP Stack for ${device_name}..."
    
    # Find rootfs
    rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        log_warn "No rootfs found for ${device}"
        record_result "ip-stack" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for IP utilities
    local has_iproute2=0
    local has_iputils=0
    local has_ipsec=0
    
    if [ -d "${rootfs_dir}/usr/sbin" ]; then
        if [ -f "${rootfs_dir}/usr/sbin/ip" ] || [ -f "${rootfs_dir}/sbin/ip" ]; then
            has_iproute2=1
        fi
    fi
    
    if [ -d "${rootfs_dir}/usr/bin" ]; then
        if [ -f "${rootfs_dir}/usr/bin/ping" ] || [ -f "${rootfs_dir}/bin/ping" ]; then
            has_iputils=1
        fi
    fi
    
    if [ -d "${rootfs_dir}/usr/sbin" ]; then
        if [ -f "${rootfs_dir}/usr/sbin/ipsec" ] || [ -f "${rootfs_dir}/usr/sbin/strongswan" ]; then
            has_ipsec=1
        fi
    fi
    
    local result="pass"
    local message="IP stack components"
    
    [ ${has_iproute2} -eq 1 ] && message="${message}+iproute2"
    [ ${has_iputils} -eq 1 ] && message="${message}+iputils"
    [ ${has_ipsec} -eq 1 ] && message="${message}+ipsec"
    
    if [ ${has_iproute2} -eq 0 ]; then
        result="fail"
        message="${message} [missing iproute2]"
    fi
    
    record_result "ip-stack" "${result}" "${message}" "${device}"
    
    if [ "${result}" = "fail" ]; then
        return 1
    fi
    return 0
}

# Test 2: BGP configuration verification
test_bgp() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Only for Core Router and Enterprise
    if [[ "${device}" != "mts-cr9000" && "${device}" != "mts-er1000" ]]; then
        record_result "bgp-config" "skip" "Not applicable for ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing BGP Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "bgp-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for FRR/BGP packages
    local has_frr=0
    local has_bgpd=0
    
    if [ -d "${rootfs_dir}/usr/sbin" ]; then
        if [ -f "${rootfs_dir}/usr/sbin/frr" ] || [ -f "${rootfs_dir}/usr/sbin/bgpd" ]; then
            has_frr=1
            has_bgpd=1
        fi
    fi
    
    if [ ${has_frr} -eq 1 ]; then
        record_result "bgp-config" "pass" "FRR/BGP packages found" "${device}"
        return 0
    else
        record_result "bgp-config" "fail" "FRR/BGP packages not found" "${device}"
        return 1
    fi
}

# Test 3: MPLS configuration verification
test_mpls() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Only for Core Router, Backhaul, and Enterprise
    if [[ "${device}" != "mts-cr9000" && "${device}" != "mts-mb3000" && "${device}" != "mts-er1000" ]]; then
        record_result "mpls-config" "skip" "Not applicable for ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing MPLS Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "mpls-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for MPLS kernel module
    local has_mpls=0
    
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*mpls*" 2>/dev/null | grep -q .; then
            has_mpls=1
        fi
    fi
    
    # Check for MPLS userspace tools
    local has_mpls_tools=0
    if [ -f "${rootfs_dir}/usr/sbin/mplsforward" ] || [ -f "${rootfs_dir}/usr/sbin/mpls-tp" ]; then
        has_mpls_tools=1
    fi
    
    if [ ${has_mpls} -eq 1 ] || [ ${has_mpls_tools} -eq 1 ]; then
        record_result "mpls-config" "pass" "MPLS components found" "${device}"
        return 0
    else
        record_result "mpls-config" "fail" "MPLS components not found" "${device}"
        return 1
    fi
}

# Test 4: SRv6 configuration verification
test_srv6() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Only for Core Router
    if [[ "${device}" != "mts-cr9000" ]]; then
        record_result "srv6-config" "skip" "Not applicable for ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing SRv6 Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "srv6-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for SRv6 kernel support
    local has_srv6=0
    
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*seg6*" 2>/dev/null | grep -q .; then
            has_srv6=1
        fi
    fi
    
    if [ ${has_srv6} -eq 1 ]; then
        record_result "srv6-config" "pass" "SRv6 kernel support found" "${device}"
        return 0
    else
        record_result "srv6-config" "fail" "SRv6 kernel support not found" "${device}"
        return 1
    fi
}

# Test 5: VPN/IPSec configuration verification
test_vpn() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # For all devices that support VPN
    log_info "Testing VPN/IPSec Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "vpn-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for IPSec packages
    local has_strongswan=0
    local has_libswan=0
    local has_ipsec_tools=0
    
    if [ -d "${rootfs_dir}/usr/sbin" ]; then
        [ -f "${rootfs_dir}/usr/sbin/strongswan" ] && has_strongswan=1
        [ -f "${rootfs_dir}/usr/sbin/swanctl" ] && has_libswan=1
    fi
    
    if [ -d "${rootfs_dir}/usr/sbin" ]; then
        [ -f "${rootfs_dir}/usr/sbin/pluto" ] && has_ipsec_tools=1
    fi
    
    if [ ${has_strongswan} -eq 1 ] || [ ${has_libswan} -eq 1 ]; then
        record_result "vpn-config" "pass" "IPSec packages found" "${device}"
        return 0
    else
        record_result "vpn-config" "fail" "IPSec packages not found" "${device}"
        return 1
    fi
}

# Test 6: QoS configuration verification
test_qos() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    log_info "Testing QoS Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "qos-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for QoS tools
    local has_tc=0
    local has_htb=0
    local has_cake=0
    
    if [ -f "${rootfs_dir}/sbin/tc" ]; then
        has_tc=1
    fi
    
    # Check for HTB/qdisc modules
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*sch_htb*" -o -name "*sch_cake*" 2>/dev/null | grep -q .; then
            has_htb=1
            has_cake=1
        fi
    fi
    
    if [ ${has_tc} -eq 1 ]; then
        record_result "qos-config" "pass" "QoS tools (tc) found" "${device}"
        return 0
    else
        record_result "qos-config" "fail" "QoS tools not found" "${device}"
        return 1
    fi
}

# Test 7: VLAN configuration verification
test_vlan() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    log_info "Testing VLAN Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "vlan-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for VLAN kernel support
    local has_vlan=0
    
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*8021q*" 2>/dev/null | grep -q .; then
            has_vlan=1
        fi
    fi
    
    if [ ${has_vlan} -eq 1 ]; then
        record_result "vlan-config" "pass" "VLAN kernel support found" "${device}"
        return 0
    else
        record_result "vlan-config" "fail" "VLAN kernel support not found" "${device}"
        return 1
    fi
}

# Test 7: Bridge configuration verification
test_bridge() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    log_info "Testing Bridge Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "bridge-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for bridge kernel module
    local has_bridge=0
    
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*bridge*" 2>/dev/null | grep -q .; then
            has_bridge=1
        fi
    fi
    
    if [ ${has_bridge} -eq 1 ]; then
        record_result "bridge-config" "pass" "Bridge kernel support found" "${device}"
        return 0
    else
        record_result "bridge-config" "fail" "Bridge kernel support not found" "${device}"
        return 1
    fi
}

# Test 8: VxLAN/Geneve configuration verification
test_tunnel() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    log_info "Testing Tunnel Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "tunnel-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for tunnel kernel modules
    local has_vxlan=0
    local has_geneve=0
    
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*vxlan*" 2>/dev/null | grep -q .; then
            has_vxlan=1
        fi
        if find "${rootfs_dir}/lib/modules" -name "*geneve*" 2>/dev/null | grep -q .; then
            has_geneve=1
        fi
    fi
    
    if [ ${has_vxlan} -eq 1 ] || [ ${has_geneve} -eq 1 ]; then
        record_result "tunnel-config" "pass" "Tunnel support found" "${device}"
        return 0
    else
        record_result "tunnel-config" "fail" "Tunnel support not found" "${device}"
        return 1
    fi
}

# Test 9: Interface configuration verification
test_interfaces() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    log_info "Testing Interface Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "interfaces-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for network interface configuration
    local has_ifupdown=0
    local has_networkd=0
    
    if [ -d "${rootfs_dir}/etc/network" ]; then
        has_ifupdown=1
    fi
    
    if [ -d "${rootfs_dir}/etc/systemd/system" ]; then
        if find "${rootfs_dir}/etc/systemd/system" -name "*network*" 2>/dev/null | grep -q .; then
            has_networkd=1
        fi
    fi
    
    if [ ${has_ifupdown} -eq 1 ] || [ ${has_networkd} -eq 1 ]; then
        record_result "interfaces-config" "pass" "Interface configuration found" "${device}"
        return 0
    else
        record_result "interfaces-config" "fail" "Interface configuration not found" "${device}"
        return 1
    fi
}

# Test 10: DNS configuration verification
test_dns() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    log_info "Testing DNS Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "dns-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for DNS resolver
    local has_dnsmasq=0
    local has_resolvconf=0
    local has_systemd_resolved=0
    
    if [ -f "${rootfs_dir}/usr/sbin/dnsmasq" ]; then
        has_dnsmasq=1
    fi
    
    if [ -f "${rootfs_dir}/usr/sbin/resolvconf" ]; then
        has_resolvconf=1
    fi
    
    if [ -d "${rootfs_dir}/etc/systemd/resolved.conf" ]; then
        has_systemd_resolved=1
    fi
    
    # Check for /etc/resolv.conf
    if [ -f "${rootfs_dir}/etc/resolv.conf" ]; then
        record_result "dns-config" "pass" "DNS configuration found (resolv.conf)" "${device}"
        return 0
    fi
    
    if [ ${has_dnsmasq} -eq 1 ] || [ ${has_resolvconf} -eq 1 ] || [ ${has_systemd_resolved} -eq 1 ]; then
        record_result "dns-config" "pass" "DNS resolver found" "${device}"
        return 0
    else
        record_result "dns-config" "fail" "DNS resolver not found" "${device}"
        return 1
    fi
}

# Test 11: PTP configuration verification
test_ptp() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Only for Mobile Backhaul
    if [[ "${device}" != "mts-mb3000" ]]; then
        record_result "ptp-config" "skip" "Not applicable for ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing PTP Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "ptp-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for linuxptp
    local has_ptp4l=0
    local has_phc2sys=0
    
    if [ -f "${rootfs_dir}/usr/sbin/ptp4l" ]; then
        has_ptp4l=1
    fi
    
    if [ -f "${rootfs_dir}/usr/sbin/phc2sys" ]; then
        has_phc2sys=1
    fi
    
    if [ ${has_ptp4l} -eq 1 ] || [ ${has_phc2sys} -eq 1 ]; then
        record_result "ptp-config" "pass" "PTP daemons found" "${device}"
        return 0
    else
        record_result "ptp-config" "fail" "PTP daemons not found" "${device}"
        return 1
    fi
}

# Test 12: 5G UPF configuration verification
test_upf() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Only for Mobile Core
    if [[ "${device}" != "mts-mc5000" ]]; then
        record_result "upf-config" "skip" "Not applicable for ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing UPF Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "upf-config" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for 5G UPF support
    local has_gtp=0
    local has_gtp_tunnel=0
    
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*gtp*" 2>/dev/null | grep -q .; then
            has_gtp=1
        fi
    fi
    
    if [ ${has_gtp} -eq 1 ]; then
        record_result "upf-config" "pass" "5G UPF/GTP support found" "${device}"
        return 0
    else
        record_result "upf-config" "fail" "5G UPF/GTP support not found" "${device}"
        return 1
    fi
}

# ============================================================
# Main test execution
# ============================================================
main() {
    log "========================================"
    log "MTS Router Networking Stack Test Suite"
    log "========================================"
    log "Start time: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    log ""
    
    local devices="${1:-all}"
    
    if [ "${devices}" = "all" ]; then
        for device in "${!DEVICES[@]}"; do
            log ""
            log "========================================"
            log "Testing ${DEVICES[$device]} (${device})"
            log "========================================"
            
            test_ip_stack "${device}" || true
            test_bgp "${device}" || true
            test_mpls "${device}" || true
            test_srv6 "${device}" || true
            test_vpn "${device}" || true
            test_qos "${device}" || true
            test_vlan "${device}" || true
            test_bridge "${device}" || true
            test_tunnel "${device}" || true
            test_interfaces "${device}" || true
            test_dns "${device}" || true
            test_ptp "${device}" || true
            test_upf "${device}" || true
        done
    else
        log "Testing device: ${devices}"
        test_ip_stack "${devices}" || true
        test_bgp "${devices}" || true
        test_mpls "${devices}" || true
        test_srv6 "${devices}" || true
        test_vpn "${devices}" || true
        test_qos "${devices}" || true
        test_vlan "${devices}" || true
        test_bridge "${devices}" || true
        test_tunnel "${devices}" || true
        test_interfaces "${devices}" || true
        test_dns "${devices}" || true
        test_ptp "${devices}" || true
        test_upf "${devices}" || true
    fi
    
    # Print summary
    log ""
    log "========================================"
    log "Networking Stack Test Summary"
    log "========================================"
    log "Total tests: $((PASS + FAIL + SKIP))"
    log -e "Passed: ${GREEN}${PASS}${NC}"
    log -e "Failed: ${RED}${FAIL}${NC}"
    log -e "Skipped: ${YELLOW}${SKIP}${NC}"
    log ""
    
    # Generate JSON report
    cat > "${RESULTS_DIR}/networking-stack-report.json" << EOF
{
    "test_suite": "networking-stack",
    "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
    "summary": {
        "total": $((PASS + FAIL + SKIP)),
        "pass": ${PASS},
        "fail": ${FAIL},
        "skip": ${SKIP}
    },
    "results_dir": "${RESULTS_DIR}",
    "log_dir": "${LOG_DIR}"
}
EOF
    
    if [ ${FAIL} -gt 0 ]; then
        log_error "${FAIL} networking stack test(s) failed"
        return 1
    fi
    
    log "Networking stack tests completed successfully"
    return 0
}

# Run main
main "${1:-all}"
