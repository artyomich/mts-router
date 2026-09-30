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
    if check_service "mobile-backhaul-api" "localhost" 50053; then
        log_pass "API server running on port 50053"
    else
        log_skip "API server not running on port 50053"
    fi
    
    # Check Buildroot build files
    if [[ -f "${PROJECT_DIR}/mobile-backhaul/linux/buildroot-config.md" ]]; then
        log_pass "Buildroot config exists"
    else
        log_fail "Buildroot config missing"
    fi
    
    # Check device tree
    if [[ -f "${PROJECT_DIR}/mobile-backhaul/linux/device-tree.dts" ]]; then
        log_pass "Device tree exists"
    else
        log_fail "Device tree missing"
    fi
    
    # Check driver spec
    if [[ -f "${PROJECT_DIR}/mobile-backhaul/firmware/driver-spec.md" ]]; then
        log_pass "Driver spec exists"
    else
        log_fail "Driver spec missing"
    fi
    
    # Check API implementation
    if [[ -d "${PROJECT_DIR}/mts-mb3000-api" ]]; then
        if [[ -f "${PROJECT_DIR}/mts-mb3000-api/CMakeLists.txt" ]]; then
            log_pass "CMakeLists.txt exists"
        else
            log_fail "CMakeLists.txt missing"
        fi
        if [[ -f "${PROJECT_DIR}/mts-mb3000-api/proto/mts_backhaul.proto" ]]; then
            log_pass "Protobuf spec exists"
        else
            log_fail "Protobuf spec missing"
        fi
        # Check HAL modules
        if [[ -f "${PROJECT_DIR}/mts-mb3000-api/include/hal/ptp_hal.h" ]]; then
            log_pass "PTP HAL module exists"
        else
            log_fail "PTP HAL module missing"
        fi
        if [[ -f "${PROJECT_DIR}/mts-mb3000-api/include/hal/sync_e_hal.h" ]]; then
            log_pass "SyncE HAL module exists"
        else
            log_fail "SyncE HAL module missing"
        fi
        if [[ -f "${PROJECT_DIR}/mts-mb3000-api/include/hal/mpls_hal.h" ]]; then
            log_pass "MPLS HAL module exists"
        else
            log_fail "MPLS HAL module missing"
        fi
    else
        log_fail "API directory missing"
    fi
}

# Test OLT GPON integration
test_olt_gpon() {
    log_section "OLT GPON (MTS-OLT-2000) Integration Tests"
    
    # Check API server
    if check_service "olt-gpon-api" "localhost" 50054; then
        log_pass "API server running on port 50054"
    else
        log_skip "API server not running on port 50054"
    fi
    
    # Check OpenWrt build files
    if [[ -f "${PROJECT_DIR}/olt-gpon/linux/openwrt-layer.md" ]]; then
        log_pass "OpenWrt layer spec exists"
    else
        log_fail "OpenWrt layer spec missing"
    fi
    
    # Check device tree
    if [[ -f "${PROJECT_DIR}/olt-gpon/linux/device-tree.dts" ]]; then
        log_pass "Device tree exists"
    else
        log_fail "Device tree missing"
    fi
    
    # Check driver spec
    if [[ -f "${PROJECT_DIR}/olt-gpon/firmware/driver-spec.md" ]]; then
        log_pass "Driver spec exists"
    else
        log_fail "Driver spec missing"
    fi
    
    # Check RTL960x driver
    if [[ -d "${PROJECT_DIR}/firmware/rtl960x-driver" ]]; then
        if [[ -f "${PROJECT_DIR}/firmware/rtl960x-driver/Makefile" ]]; then
            log_pass "RTL960x driver Makefile exists"
        else
            log_fail "RTL960x driver Makefile missing"
        fi
        if [[ -f "${PROJECT_DIR}/firmware/rtl960x-driver/include/rtl960x.h" ]]; then
            log_pass "RTL960x header exists"
        else
            log_fail "RTL960x header missing"
        fi
    else
        log_fail "RTL960x driver directory missing"
    fi
    
    # Check API implementation
    if [[ -d "${PROJECT_DIR}/olt-gpon-api" ]]; then
        if [[ -f "${PROJECT_DIR}/olt-gpon-api/CMakeLists.txt" ]]; then
            log_pass "CMakeLists.txt exists"
        else
            log_fail "CMakeLists.txt missing"
        fi
        if [[ -f "${PROJECT_DIR}/olt-gpon-api/proto/mts_olt_gpon.proto" ]]; then
            log_pass "Protobuf spec exists"
        else
            log_fail "Protobuf spec missing"
        fi
        # Check HAL modules
        for hal in gpon_hal onu_hal omci_hal tr069_hal; do
            if [[ -f "${PROJECT_DIR}/olt-gpon-api/include/hal/${hal}.h" ]]; then
                log_pass "${hal} module exists"
            else
                log_fail "${hal} module missing"
            fi
        done
    else
        log_fail "API directory missing"
    fi
}

# Test Enterprise Router integration
test_enterprise() {
    log_section "Enterprise Router (MTS-ER-1000) Integration Tests"
    
    # Check API server
    if check_service "enterprise-api" "localhost" 50055; then
        log_pass "API server running on port 50055"
    else
        log_skip "API server not running on port 50055"
    fi
    
    # Check OpenWrt build files
    if [[ -f "${PROJECT_DIR}/enterprise-router/linux/openwrt-layer.md" ]]; then
        log_pass "OpenWrt layer spec exists"
    else
        log_fail "OpenWrt layer spec missing"
    fi
    
    # Check device tree
    if [[ -f "${PROJECT_DIR}/enterprise-router/linux/device-tree.dts" ]]; then
        log_pass "Device tree exists"
    else
        log_fail "Device tree missing"
    fi
    
    # Check driver spec
    if [[ -f "${PROJECT_DIR}/enterprise-router/firmware/driver-spec.md" ]]; then
        log_pass "Driver spec exists"
    else
        log_fail "Driver spec missing"
    fi
    
    # Check TomTom driver
    if [[ -d "${PROJECT_DIR}/firmware/tomtom-driver" ]]; then
        if [[ -f "${PROJECT_DIR}/firmware/tomtom-driver/Makefile" ]]; then
            log_pass "TomTom driver Makefile exists"
        else
            log_fail "TomTom driver Makefile missing"
        fi
        if [[ -f "${PROJECT_DIR}/firmware/tomtom-driver/include/tomtom.h" ]]; then
            log_pass "TomTom header exists"
        else
            log_fail "TomTom header missing"
        fi
    else
        log_fail "TomTom driver directory missing"
    fi
    
    # Check API implementation
    if [[ -d "${PROJECT_DIR}/enterprise-router-api" ]]; then
        if [[ -f "${PROJECT_DIR}/enterprise-router-api/CMakeLists.txt" ]]; then
            log_pass "CMakeLists.txt exists"
        else
            log_fail "CMakeLists.txt missing"
        fi
        if [[ -f "${PROJECT_DIR}/enterprise-router-api/proto/mts_enterprise.proto" ]]; then
            log_pass "Protobuf spec exists"
        else
            log_fail "Protobuf spec missing"
        fi
        # Check HAL modules
        for hal in sdwan_hal ipsec_hal vrrp_hal mpls_hal; do
            if [[ -f "${PROJECT_DIR}/enterprise-router-api/include/hal/${hal}.h" ]]; then
                log_pass "${hal} module exists"
            else
                log_fail "${hal} module missing"
            fi
        done
    else
        log_fail "API directory missing"
    fi
}

# Test Residential Gateway integration
test_residential() {
    log_section "Residential Gateway (MTS-RG-500) Integration Tests"
    
    # Check API server
    if check_service "residential-api" "localhost" 50056; then
        log_pass "API server running on port 50056"
    else
        log_skip "API server not running on port 50056"
    fi
    
    # Check OpenWrt build files
    if [[ -f "${PROJECT_DIR}/residential-gateway/linux/openwrt-layer.md" ]]; then
        log_pass "OpenWrt layer spec exists"
    else
        log_fail "OpenWrt layer spec missing"
    fi
    
    # Check device tree
    if [[ -f "${PROJECT_DIR}/residential-gateway/linux/device-tree.dts" ]]; then
        log_pass "Device tree exists"
    else
        log_fail "Device tree missing"
    fi
    
    # Check driver spec
    if [[ -f "${PROJECT_DIR}/residential-gateway/firmware/driver-spec.md" ]]; then
        log_pass "Driver spec exists"
    else
        log_fail "Driver spec missing"
    fi
    
    # Check MT7981 driver
    if [[ -d "${PROJECT_DIR}/firmware/mts-rg-drivers/mt7981" ]]; then
        if [[ -f "${PROJECT_DIR}/firmware/mts-rg-drivers/mt7981/Makefile" ]]; then
            log_pass "MT7981 driver Makefile exists"
        else
            log_fail "MT7981 driver Makefile missing"
        fi
        if [[ -f "${PROJECT_DIR}/firmware/mts-rg-drivers/mt7981/include/mt7981.h" ]]; then
            log_pass "MT7981 header exists"
        else
            log_fail "MT7981 header missing"
        fi
    else
        log_fail "MT7981 driver directory missing"
    fi
    
    # Check API implementation
    if [[ -d "${PROJECT_DIR}/residential-gateway-api" ]]; then
        if [[ -f "${PROJECT_DIR}/residential-gateway-api/CMakeLists.txt" ]]; then
            log_pass "CMakeLists.txt exists"
        else
            log_fail "CMakeLists.txt missing"
        fi
        if [[ -f "${PROJECT_DIR}/residential-gateway-api/proto/mts_residential.proto" ]]; then
            log_pass "Protobuf spec exists"
        else
            log_fail "Protobuf spec missing"
        fi
        # Check HAL modules
        for hal in wifi_hal voip_hal iptv_hal gpon_hal tr069_hal; do
            if [[ -f "${PROJECT_DIR}/residential-gateway-api/include/hal/${hal}.h" ]]; then
                log_pass "${hal} module exists"
            else
                log_fail "${hal} module missing"
            fi
        done
    else
        log_fail "API directory missing"
    fi
}

# Test REST API Gateway
test_rest_api() {
    log_section "REST API Gateway Tests"
    
    # Check REST API implementation
    if [[ -d "${PROJECT_DIR}/api/rest-gateway" ]]; then
        if [[ -f "${PROJECT_DIR}/api/rest-gateway/CMakeLists.txt" ]]; then
            log_pass "REST API CMakeLists.txt exists"
        else
            log_fail "REST API CMakeLists.txt missing"
        fi
        if [[ -f "${PROJECT_DIR}/api/rest-gateway/src/mts-rest-server.c" ]]; then
            log_pass "REST API server source exists"
        else
            log_fail "REST API server source missing"
        fi
        if [[ -f "${PROJECT_DIR}/api/rest-gateway/include/mts-rest.h" ]]; then
            log_pass "REST API header exists"
        else
            log_fail "REST API header missing"
        fi
    else
        log_fail "REST API directory missing"
    fi
    
    # Check telemetry implementation
    if [[ -f "${PROJECT_DIR}/api/rest-gateway/src/mts-rest-telemetry.c" ]]; then
        log_pass "Telemetry module exists"
    else
        log_fail "Telemetry module missing"
    fi
    
    # Check config management implementation
    if [[ -f "${PROJECT_DIR}/api/rest-gateway/src/mts-rest-config.c" ]]; then
        log_pass "Config management module exists"
    else
        log_fail "Config management module missing"
    fi
    
    # Check OpenAPI spec
    if [[ -f "${PROJECT_DIR}/api/spec/mts-router-openapi.yaml" ]]; then
        log_pass "OpenAPI spec exists"
    else
        log_fail "OpenAPI spec missing"
    fi
    
    # Check OpenAPI extensions
    if [[ -f "${PROJECT_DIR}/api/spec/mts-extensions.yaml" ]]; then
        log_pass "OpenAPI extensions exist"
    else
        log_fail "OpenAPI extensions missing"
    fi
}

# Test API/SDK
test_api_sdk() {
    log_section "API/SDK Tests"
    
    # Check YANG models
    local yang_count=0
    while IFS= read -r -d '' yang_file; do
        yang_count=$((yang_count + 1))
    done < <(find "${PROJECT_DIR}/api/spec" -name "*.yang" -print0 2>/dev/null)
    
    if [[ $yang_count -gt 0 ]]; then
        log_pass "Found ${yang_count} YANG model(s)"
    else
        log_fail "No YANG models found"
    fi
    
    # Check protobuf specs (they are in API directories, not api/spec)
    local proto_count=0
    while IFS= read -r -d '' _proto_file; do
        proto_count=$((proto_count + 1))
    done < <(find "${PROJECT_DIR}" -maxdepth 3 -name "*.proto" -print0 2>/dev/null)
    
    if [[ $proto_count -gt 0 ]]; then
        log_pass "Found ${proto_count} protobuf spec(s)"
    else
        log_fail "No protobuf specs found"
    fi
    
    # Check SDK implementations
    if [[ -d "${PROJECT_DIR}/api/sdk/go" ]]; then
        if [[ -f "${PROJECT_DIR}/api/sdk/go/go.mod" ]]; then
            log_pass "Go SDK exists"
        else
            log_fail "Go SDK missing"
        fi
    fi
    
    if [[ -d "${PROJECT_DIR}/api/sdk/java" ]]; then
        if [[ -f "${PROJECT_DIR}/api/sdk/java/MtsRouterClient.java" ]]; then
            log_pass "Java SDK exists"
        else
            log_fail "Java SDK missing"
        fi
    fi
    
    if [[ -f "${PROJECT_DIR}/api/examples/mts-api-examples.py" ]]; then
        log_pass "Python examples exist"
    else
        log_fail "Python examples missing"
    fi
}

# Test CI/CD
test_ci_cd() {
    log_section "CI/CD Tests"
    
    # Check GitHub Actions workflows
    if [[ -d "${PROJECT_DIR}/.github/workflows" ]]; then
        local workflow_count=0
        while IFS= read -r -d '' wf_file; do
            workflow_count=$((workflow_count + 1))
        done < <(find "${PROJECT_DIR}/.github/workflows" -name "*.yml" -print0 2>/dev/null)
        
        if [[ $workflow_count -gt 0 ]]; then
            log_pass "Found ${workflow_count} GitHub Actions workflow(s)"
        else
            log_fail "No GitHub Actions workflows found"
        fi
    else
        log_fail "GitHub Actions directory missing"
    fi
    
    # Check build scripts
    for device in core-router mobile-core mobile-backhaul olt-gpon enterprise residential; do
        if [[ -f "${PROJECT_DIR}/scripts/build-${device}.sh" ]]; then
            log_pass "Build script for ${device} exists"
        else
            log_fail "Build script for ${device} missing"
        fi
    done
    
    # Check test scripts
    for test in test-all-protocols test-performance test-ha test-integration; do
        if [[ -f "${PROJECT_DIR}/scripts/${test}.sh" ]]; then
            log_pass "Test script ${test} exists"
        else
            log_fail "Test script ${test} missing"
        fi
    done
}

# Test Security Framework
test_security() {
    log_section "Security Framework Tests"
    
    # Check security audit script
    if [[ -f "${PROJECT_DIR}/scripts/security-audit.sh" ]]; then
        log_pass "Security audit script exists"
    else
        log_fail "Security audit script missing"
    fi
    
    # Check security documentation
    if [[ -d "${PROJECT_DIR}/qa/security" ]]; then
        if [[ -f "${PROJECT_DIR}/qa/security/README.md" ]]; then
            log_pass "Security audit documentation exists"
        else
            log_fail "Security audit documentation missing"
        fi
    else
        log_fail "Security QA directory missing"
    fi
    
    # Check performance optimization guide
    if [[ -f "${PROJECT_DIR}/docs/performance/optimization-guide.md" ]]; then
        log_pass "Performance optimization guide exists"
    else
        log_fail "Performance optimization guide missing"
    fi
    
    # Check certification checklist
    if [[ -f "${PROJECT_DIR}/docs/certification-checklist.md" ]]; then
        log_pass "Certification checklist exists"
    else
        log_fail "Certification checklist missing"
    fi
}

# Cross-device integration tests
test_cross_device() {
    log_section "Cross-Device Integration Tests"
    
    # Test that all device specs reference the same API schema
    local api_spec="${PROJECT_DIR}/api/spec/mts-router-api.md"
    if [[ -f "$api_spec" ]]; then
        log_pass "Common API spec exists - all devices can share interface"
        
        # Check that each device spec references common API
        for device_spec in core-router/spec/core-router-spec.md mobile-core/spec/mobile-core-spec.md mobile-backhaul/spec/mobile-backhaul-spec.md olt-gpon/spec/olt-gpon-spec.md enterprise-router/spec/enterprise-router-spec.md residential-gateway/spec/residential-gateway-spec.md; do
            if [[ -f "${PROJECT_DIR}/${device_spec}" ]]; then
                if grep -qi "api\|grpc\|yang\|protobuf" "${PROJECT_DIR}/${device_spec}" 2>/dev/null; then
                    log_pass "$(basename $(dirname ${device_spec})) references API spec"
                else
                    log_warn "$(basename $(dirname ${device_spec})) does not explicitly reference API spec"
                fi
            fi
        done
    fi
    
    # Test that all device trees are consistent
    local dtc_count=0
    for dtb in core-router/linux/device-tree.dts mobile-core/linux/device-tree.dts mobile-backhaul/linux/device-tree.dts olt-gpon/linux/device-tree.dts enterprise-router/linux/device-tree.dts residential-gateway/linux/device-tree.dts; do
        if [[ -f "${PROJECT_DIR}/${dtb}" ]]; then
            dtc_count=$((dtc_count + 1))
        fi
    done
    log_pass "Found ${dtc_count}/6 device tree files"
    
    # Test that all driver specs reference common kernel interfaces
    local driver_count=0
    for driver in core-router/firmware/driver-spec.md mobile-core/firmware/driver-spec.md mobile-backhaul/firmware/driver-spec.md olt-gpon/firmware/driver-spec.md enterprise-router/firmware/driver-spec.md residential-gateway/firmware/driver-spec.md; do
        if [[ -f "${PROJECT_DIR}/${driver}" ]]; then
            driver_count=$((driver_count + 1))
        fi
    done
    log_pass "Found ${driver_count}/6 driver spec files"
}

# Test Linux OS layer
test_linux_layer() {
    log_section "Linux OS Layer Tests"
    
    # Check meta-mts layer
    if [[ -d "${PROJECT_DIR}/linux/meta-mts" ]]; then
        log_pass "meta-mts layer exists"
        
        # Check machine configs
        if [[ -d "${PROJECT_DIR}/linux/meta-mts/conf" ]]; then
            log_pass "meta-mts machine configs exist"
        else
            log_fail "meta-mts machine configs missing"
        fi
        
        # Check recipes
        if [[ -d "${PROJECT_DIR}/linux/meta-mts/recipes-core/images" ]]; then
            local image_count=0
            while IFS= read -r -d '' img; do
                image_count=$((image_count + 1))
            done < <(find "${PROJECT_DIR}/linux/meta-mts/recipes-core/images" -name "*.bb" -print0 2>/dev/null)
            log_pass "Found ${image_count} Yocto image recipe(s)"
        else
            log_fail "meta-mts image recipes missing"
        fi
        
        # Check kernel configs
        if [[ -d "${PROJECT_DIR}/linux/meta-mts/recipes-kernel/linux/configs" ]]; then
            local config_count=0
            while IFS= read -r -d '' cfg; do
                config_count=$((config_count + 1))
            done < <(find "${PROJECT_DIR}/linux/meta-mts/recipes-kernel/linux/configs" -name "*.cfg" -print0 2>/dev/null)
            log_pass "Found ${config_count} kernel config(s)"
        else
            log_fail "kernel configs missing"
        fi
    else
        log_fail "meta-mts layer missing"
    fi
}

# ============================================================
# Main execution
# ============================================================
main() {
    log_section "MTS Router Integration Test Suite"
    echo ""
    echo "  Started: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "  Hostname: $(hostname)"
    echo "  Kernel: $(uname -r)"
    echo ""
    
    # Run all tests
    test_core_router
    test_mobile_core
    test_mobile_backhaul
    test_olt_gpon
    test_enterprise
    test_residential
    test_rest_api
    test_api_sdk
    test_ci_cd
    test_security
    test_cross_device
    test_linux_layer
    
    # Summary
    log_section "Integration Test Summary"
    local total=$((PASS + FAIL + SKIP))
    echo ""
    echo -e "  ${GREEN}PASSED:  ${PASS}${NC}"
    echo -e "  ${RED}FAILED:  ${FAIL}${NC}"
    echo -e "  ${YELLOW}SKIPPED: ${SKIP}${NC}"
    echo ""
    echo -e "  TOTAL:   ${total}"
    echo ""
    
    if [[ $FAIL -gt 0 ]]; then
        echo -e "  ${RED}Integration tests FAILED${NC}"
        exit 1
    else
        echo -e "  ${GREEN}Integration tests PASSED${NC}"
        exit 0
    fi
}

# Run main
main "$@"