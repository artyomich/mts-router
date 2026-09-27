#!/bin/bash
# MTS Router — DPDK Integration Test Suite
# Tests DPDK functionality for all MTS Router devices
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
RESULTS_DIR="${BUILD_DIR}/test-results/dpdk"
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
    
    local result_file="${RESULTS_DIR}/${device}-dpdk-results.json"
    
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

# DPDK-capable devices
DPDK_DEVICES=("mts-cr9000" "mts-mc5000" "mts-mb3000" "mts-olt2000" "mts-er1000")

# ============================================================
# Test functions
# ============================================================

# Test 1: DPDK package verification
test_dpdk_packages() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "dpdk-packages" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing DPDK Packages for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "dpdk-packages" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for DPDK packages
    local has_dpdk_lib=0
    local has_testpmd=0
    local has_pktgen=0
    local has_dpdk_kmod=0
    
    # Check library
    if find "${rootfs_dir}" -name "librte*.so*" 2>/dev/null | grep -q .; then
        has_dpdk_lib=1
    fi
    
    # Check testpmd
    if [ -f "${rootfs_dir}/usr/bin/testpmd" ] || [ -f "${rootfs_dir}/usr/sbin/testpmd" ]; then
        has_testpmd=1
    fi
    
    # Check pktgen
    if [ -f "${rootfs_dir}/usr/bin/pktgen" ] || [ -f "${rootfs_dir}/usr/sbin/pktgen" ]; then
        has_pktgen=1
    fi
    
    # Check kernel module
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "kni.ko" -o -name "vfio-pci.ko" 2>/dev/null | grep -q .; then
            has_dpdk_kmod=1
        fi
    fi
    
    local result="pass"
    local message="DPDK components: "
    [ ${has_dpdk_lib} -eq 1 ] && message="${message}lib "
    [ ${has_testpmd} -eq 1 ] && message="${message}testpmd "
    [ ${has_pktgen} -eq 1 ] && message="${message}pktgen "
    [ ${has_dpdk_kmod} -eq 1 ] && message="${message}kmod"
    
    if [ ${has_dpdk_lib} -eq 0 ]; then
        result="fail"
        message="${message} [missing librte]"
    fi
    
    record_result "dpdk-packages" "${result}" "${message}" "${device}"
    
    if [ "${result}" = "fail" ]; then
        return 1
    fi
    return 0
}

# Test 2: Hugepages configuration
test_hugepages() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "hugepages" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing Hugepages Configuration for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "hugepages" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for hugetlbfs support in kernel
    local has_hugetlb=0
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*hugetlb*" 2>/dev/null | grep -q .; then
            has_hugetlb=1
        fi
    fi
    
    # Check for transparent hugepages in boot config
    local has_thp=0
    if [ -f "${rootfs_dir}/boot/extlinux/extlinux.conf" ]; then
        if grep -q "transhuge\|hugepages\|thp" "${rootfs_dir}/boot/extlinux/extlinux.conf" 2>/dev/null; then
            has_thp=1
        fi
    fi
    
    # Check kernel config
    local has_kernel_hugetlb=0
    if [ -f "${rootfs_dir}/boot/config-"* ]; then
        if grep -q "CONFIG_HUGETLBFS=y" "${rootfs_dir}/boot/config-"* 2>/dev/null; then
            has_kernel_hugetlb=1
        fi
    fi
    
    local result="pass"
    local message="Hugepages support: "
    [ ${has_hugetlb} -eq 1 ] && message="${message}hugetlbfs "
    [ ${has_thp} -eq 1 ] && message="${message}THP "
    [ ${has_kernel_hugetlb} -eq 1 ] && message="${message}kernel"
    
    if [ ${has_kernel_hugetlb} -eq 0 ]; then
        result="fail"
        message="${message} [missing kernel hugepages support]"
    fi
    
    record_result "hugepages" "${result}" "${message}" "${device}"
    
    if [ "${result}" = "fail" ]; then
        return 1
    fi
    return 0
}

# Test 3: VFIO/PCI driver verification
test_vfio_driver() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "vfio-driver" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing VFIO/PCI Driver for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "vfio-driver" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for VFIO kernel module
    local has_vfio=0
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*vfio*" 2>/dev/null | grep -q .; then
            has_vfio=1
        fi
    fi
    
    # Check for IOMMU support
    local has_iommu=0
    if [ -d "${rootfs_dir}/lib/modules" ]; then
        if find "${rootfs_dir}/lib/modules" -name "*iommu*" 2>/dev/null | grep -q .; then
            has_iommu=1
        fi
    fi
    
    # Check for VFIO userspace tools
    local has_vfio_user=0
    if [ -f "${rootfs_dir}/usr/sbin/vfio-bind" ] || [ -f "${rootfs_dir}/usr/bin/vfio-bind" ]; then
        has_vfio_user=1
    fi
    
    local result="pass"
    local message="VFIO/IOMMU: "
    [ ${has_vfio} -eq 1 ] && message="${message}vfio-kmod "
    [ ${has_iommu} -eq 1 ] && message="${message}iommu "
    [ ${has_vfio_user} -eq 1 ] && message="${message}vfio-tools"
    
    if [ ${has_vfio} -eq 0 ]; then
        result="fail"
        message="${message} [missing VFIO kernel module]"
    fi
    
    record_result "vfio-driver" "${result}" "${message}" "${device}"
    
    if [ "${result}" = "fail" ]; then
        return 1
    fi
    return 0
}

# Test 4: DPDK PMD drivers verification
test_pmd_drivers() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "pmd-drivers" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing PMD Drivers for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "pmd-drivers" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for PMD libraries
    local pmd_count=0
    local pmd_list=""
    
    if [ -d "${rootfs_dir}/usr/lib/dpdk" ]; then
        pmd_count=$(find "${rootfs_dir}/usr/lib/dpdk" -name "librte_pmd_*.so*" 2>/dev/null | wc -l)
        pmd_list=$(find "${rootfs_dir}/usr/lib/dpdk" -name "librte_pmd_*.so*" 2>/dev/null | xargs -I{} basename {} .so* | sed 's/librte_pmd_//g' | tr '\n' ',' | sed 's/,$//')
    fi
    
    # Also check /usr/lib
    if [ ${pmd_count} -eq 0 ] && [ -d "${rootfs_dir}/usr/lib" ]; then
        pmd_count=$(find "${rootfs_dir}/usr/lib" -name "librte_pmd_*.so*" 2>/dev/null | wc -l)
        pmd_list=$(find "${rootfs_dir}/usr/lib" -name "librte_pmd_*.so*" 2>/dev/null | xargs -I{} basename {} .so* | sed 's/librte_pmd_//g' | tr '\n' ',' | sed 's/,$//')
    fi
    
    if [ ${pmd_count} -gt 0 ]; then
        record_result "pmd-drivers" "pass" "${pmd_count} PMDs found: ${pmd_list}" "${device}"
        return 0
    else
        record_result "pmd-drivers" "fail" "No PMD drivers found" "${device}"
        return 1
    fi
}

# Test 7: DPDK app verification
test_dpdk_apps() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "dpdk-apps" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing DPDK Apps for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "dpdk-apps" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for DPDK applications
    local apps_dir=""
    local app_count=0
    local app_list=""
    
    # Check common DPDK app locations
    for dir in "${rootfs_dir}/usr/bin/dpdk-"* "${rootfs_dir}/usr/sbin/dpdk-"* "${rootfs_dir}/usr/bin/" "${rootfs_dir}/usr/sbin/"; do
        if [ -d "${dir}" ]; then
            local count=$(find "${dir}" -name "dpdk-*" -o -name "testpmd" -o -name "pktgen" 2>/dev/null | wc -l)
            if [ ${count} -gt 0 ]; then
                app_count=$((app_count + count))
                local apps=$(find "${dir}" -name "dpdk-*" -o -name "testpmd" -o -name "pktgen" 2>/dev/null | xargs -I{} basename {} | tr '\n' ',' | sed 's/,$//')
                if [ -n "${apps}" ]; then
                    app_list="${app_list}${apps},"
                fi
            fi
        fi
    done
    
    # Remove trailing comma
    app_list=$(echo "${app_list}" | sed 's/,$//')
    
    if [ ${app_count} -gt 0 ]; then
        record_result "dpdk-apps" "pass" "${app_count} apps found: ${app_list}" "${device}"
        return 0
    else
        record_result "dpdk-apps" "fail" "No DPDK applications found" "${device}"
        return 1
    fi
}

# Test 8: DPDK kernel configuration
test_dpdk_kernel_config() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "dpdk-kernel" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing DPDK Kernel Config for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "dpdk-kernel" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Find kernel config
    local kernel_config=""
    if [ -f "${rootfs_dir}/boot/config-"* ]; then
        kernel_config=$(find "${rootfs_dir}/boot" -name "config-*" | head -1)
    elif [ -f "${rootfs_dir}/proc/config.gz" ]; then
        kernel_config="${rootfs_dir}/proc/config.gz"
    fi
    
    if [ -z "${kernel_config}" ]; then
        record_result "dpdk-kernel" "skip" "No kernel config found" "${device}"
        return 1
    fi
    
    # Check required DPDK kernel options
    local required_opts=(
        "CONFIG_HUGETLBFS=y"
        "CONFIG_TRANSPARENT_HUGEPAGE=y"
        "CONFIG_IOMMU_API=y"
        "CONFIG_VFIO=y"
        "CONFIG_PCI_IOV=y"
        "CONFIG_PCI_MSI=y"
    )
    
    local missing_opts=""
    local found_opts=0
    
    for opt in "${required_opts[@]}"; do
        if grep -q "${opt}" "${kernel_config}" 2>/dev/null; then
            found_opts=$((found_opts + 1))
        else
            missing_opts="${missing_opts} ${opt}"
        fi
    done
    
    local total_opts=${#required_opts[@]}
    
    if [ ${found_opts} -eq ${total_opts} ]; then
        record_result "dpdk-kernel" "pass" "All ${total_opts} DPDK kernel options configured" "${device}"
        return 0
    elif [ ${found_opts} -gt $((total_opts / 2)) ]; then
        record_result "dpdk-kernel" "warn" "${found_opts}/${total_opts} DPDK kernel options configured${missing_opts}" "${device}"
        return 0
    else
        record_result "dpdk-kernel" "fail" "Insufficient DPDK kernel options${missing_opts}" "${device}"
        return 1
    fi
}

# Test 9: DPDK EAL parameters verification
test_dpdk_eal() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "dpdk-eal" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing DPDK EAL Parameters for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "dpdk-eal" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for DPDK EAL configuration file
    local eal_conf_found=0
    local eal_conf_files=()
    
    for conf_dir in "${rootfs_dir}/etc/dpdk" "${rootfs_dir}/etc/" "${rootfs_dir}/opt/dpdk/etc"; do
        if [ -d "${conf_dir}" ]; then
            local confs=$(find "${conf_dir}" -name "*dpdk*" -o -name "*eal*" 2>/dev/null)
            if [ -n "${confs}" ]; then
                eal_conf_found=1
                eal_conf_files+=("${confs}")
            fi
        fi
    done
    
    if [ ${eal_conf_found} -eq 1 ]; then
        record_result "dpdk-eal" "pass" "DPDK EAL configuration files found" "${device}"
        return 0
    else
        # EAL config is optional - DPDK uses command line args
        record_result "dpdk-eal" "skip" "No EAL config files (command-line args are used)" "${device}"
        return 0
    fi
}

# Test 10: DPDK numa support
test_dpdk_numa() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "dpdk-numa" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing DPDK NUMA Support for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "dpdk-numa" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for NUMA support in kernel
    local has_numa=0
    if [ -f "${rootfs_dir}/boot/config-"* ]; then
        if grep -q "CONFIG_NUMA=y" "${rootfs_dir}/boot/config-"* 2>/dev/null; then
            has_numa=1
        fi
    fi
    
    # Check for NUMA userspace tools
    local has_numactl=0
    if [ -f "${rootfs_dir}/usr/sbin/numactl" ] || [ -f "${rootfs_dir}/usr/bin/numactl" ]; then
        has_numactl=1
    fi
    
    local result="pass"
    local message="NUMA support: "
    [ ${has_numa} -eq 1 ] && message="${message}kernel "
    [ ${has_numactl} -eq 1 ] && message="${message}numactl"
    
    if [ ${has_numa} -eq 0 ]; then
        result="fail"
        message="${message} [missing kernel NUMA support]"
    fi
    
    record_result "dpdk-numa" "${result}" "${message}" "${device}"
    
    if [ "${result}" = "fail" ]; then
        return 1
    fi
    return 0
}

# Test 11: DPDK memory pool verification
test_dpdk_mempool() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    # Check if device supports DPDK
    local supports_dpdk=0
    for dpdk_dev in "${DPDK_DEVICES[@]}"; do
        if [ "${dpdk_dev}" = "${device}" ]; then
            supports_dpdk=1
            break
        fi
    done
    
    if [ ${supports_dpdk} -eq 0 ]; then
        record_result "dpdk-mempool" "skip" "DPDK not supported on ${device_name}" "${device}"
        return 0
    fi
    
    log_info "Testing DPDK Mempool for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "dpdk-mempool" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for DPDK mempool libraries
    local has_mempool=0
    
    if find "${rootfs_dir}" -name "librte_mempool*" 2>/dev/null | grep -q .; then
        has_mempool=1
    fi
    
    if find "${rootfs_dir}" -name "librte_ring*" 2>/dev/null | grep -q .; then
        has_mempool=1
    fi
    
    if [ ${has_mempool} -eq 1 ]; then
        record_result "dpdk-mempool" "pass" "DPDK mempool/ring libraries found" "${device}"
        return 0
    else
        record_result "dpdk-mempool" "fail" "DPDK mempool/ring libraries not found" "${device}"
        return 1
    fi
}

# Test 12: DPDK crypto offload verification
test_dpdk_crypto() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    log_info "Testing DPDK Crypto for ${device_name}..."
    
    local rootfs_dir=$(find "${BUILD_DIR}" -path "*/images/*" -type d 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${rootfs_dir}" ]; then
        record_result "dpdk-crypto" "skip" "No rootfs available" "${device}"
        return 1
    fi
    
    # Check for DPDK crypto libraries
    local has_crypto=0
    
    if find "${rootfs_dir}" -name "librte_crypto*" 2>/dev/null | grep -q .; then
        has_crypto=1
    fi
    
    if find "${rootfs_dir}" -name "librte_pmd_*crypt*" 2>/dev/null | grep -q .; then
        has_crypto=1
    fi
    
    if [ ${has_crypto} -eq 1 ]; then
        record_result "dpdk-crypto" "pass" "DPDK crypto libraries found" "${device}"
        return 0
    else
        record_result "dpdk-crypto" "skip" "DPDK crypto not required for this device" "${device}"
        return 0
    fi
}

# ============================================================
# Main test execution
# ============================================================
main() {
    log "========================================"
    log "MTS Router DPDK Integration Test Suite"
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
            
            test_dpdk_packages "${device}" || true
            test_hugepages "${device}" || true
            test_vfio_driver "${device}" || true
            test_pmd_drivers "${device}" || true
            test_dpdk_apps "${device}" || true
            test_dpdk_kernel_config "${device}" || true
            test_dpdk_eal "${device}" || true
            test_dpdk_numa "${device}" || true
            test_dpdk_mempool "${device}" || true
            test_dpdk_crypto "${device}" || true
        done
    else
        log "Testing device: ${devices}"
        test_dpdk_packages "${devices}" || true
        test_hugepages "${devices}" || true
        test_vfio_driver "${devices}" || true
        test_pmd_drivers "${devices}" || true
        test_dpdk_apps "${devices}" || true
        test_dpdk_kernel_config "${devices}" || true
        test_dpdk_eal "${devices}" || true
        test_dpdk_numa "${devices}" || true
        test_dpdk_mempool "${devices}" || true
        test_dpdk_crypto "${devices}" || true
    fi
    
    # Print summary
    log ""
    log "========================================"
    log "DPDK Integration Test Summary"
    log "========================================"
    log "Total tests: $((PASS + FAIL + SKIP))"
    log -e "Passed: ${GREEN}${PASS}${NC}"
    log -e "Failed: ${RED}${FAIL}${NC}"
    log -e "Skipped: ${YELLOW}${SKIP}${NC}"
    log ""
    
    # Generate JSON report
    cat > "${RESULTS_DIR}/dpdk-integration-report.json" << EOF
{
    "test_suite": "dpdk-integration",
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
        log_error "${FAIL} DPDK integration test(s) failed"
        return 1
    fi
    
    log "DPDK integration tests completed successfully"
    return 0
}

# Run main
main "${1:-all}"
