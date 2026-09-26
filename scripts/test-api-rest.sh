#!/bin/bash
# test-api-rest.sh — Тестирование REST API всех устройств
#
# Usage:
#   ./test-api-rest.sh [--device <device>] [--verbose]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Test results
PASS=0
FAIL=0
SKIP=0

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
    echo -e "${GREEN}[PASS]${NC} $*"
}

log_fail() {
    FAIL=$((FAIL + 1))
    echo -e "${RED}[FAIL]${NC} $*"
}

log_skip() {
    SKIP=$((SKIP + 1))
    echo -e "${YELLOW}[SKIP]${NC} $*"
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $*"
}

# Check if curl is available
if ! command -v curl &>/dev/null; then
    log_info "curl not found, skipping REST API tests"
    exit 0
fi

# Test health endpoint
test_health() {
    local device="$1"
    local port="$2"
    local url="http://localhost:${port}/api/v1/health"
    
    if [[ "$VERBOSE" == true ]]; then
        log_info "Testing health endpoint: ${url}"
    fi
    
    if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
        if echo "$response" | grep -q '"status"'; then
            log_pass "Health check for ${device}"
            return 0
        fi
    fi
    log_fail "Health check for ${device} (unreachable)"
    return 1
}

# Test system info endpoint
test_system() {
    local device="$1"
    local port="$2"
    local url="http://localhost:${port}/api/v1/system"
    
    if [[ "$VERBOSE" == true ]]; then
        log_info "Testing system endpoint: ${url}"
    fi
    
    if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
        if echo "$response" | grep -q '"model"'; then
            log_pass "System info for ${device}"
            return 0
        fi
    fi
    log_fail "System info for ${device} (unreachable)"
    return 1
}

# Test interfaces endpoint
test_interfaces() {
    local device="$1"
    local port="$2"
    local url="http://localhost:${port}/api/v1/interfaces"
    
    if [[ "$VERBOSE" == true ]]; then
        log_info "Testing interfaces endpoint: ${url}"
    fi
    
    if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
        if echo "$response" | grep -q '"interfaces"'; then
            log_pass "Interfaces for ${device}"
            return 0
        fi
    fi
    log_fail "Interfaces for ${device} (unreachable)"
    return 1
}

# Test routing endpoint
test_routing() {
    local device="$1"
    local port="$2"
    local url="http://localhost:${port}/api/v1/routing/tables"
    
    if [[ "$VERBOSE" == true ]]; then
        log_info "Testing routing endpoint: ${url}"
    fi
    
    if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
        if echo "$response" | grep -q '"routes"'; then
            log_pass "Routing for ${device}"
            return 0
        fi
    fi
    log_fail "Routing for ${device} (unreachable)"
    return 1
}

# Test BGP endpoint (Core Router only)
test_bgp() {
    local device="$1"
    local port="$2"
    local url="http://localhost:${port}/api/v1/bgp"
    
    if [[ "$VERBOSE" == true ]]; then
        log_info "Testing BGP endpoint: ${url}"
    fi
    
    if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
        if echo "$response" | grep -q '"bgp"'; then
            log_pass "BGP for ${device}"
            return 0
        fi
    fi
    log_fail "BGP for ${device} (unreachable)"
    return 1
}

# Test device-specific endpoints
test_device_endpoints() {
    local device="$1"
    local port="$2"
    
    case "$device" in
        core-router)
            test_bgp "$device" "$port"
            ;;
        mobile-core)
            # Test UPF endpoint
            local url="http://localhost:${port}/api/v1/upf"
            if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
                log_pass "UPF for ${device}"
            else
                log_fail "UPF for ${device} (unreachable)"
            fi
            ;;
        mobile-backhaul)
            # Test PTP endpoint
            local url="http://localhost:${port}/api/v1/ptp"
            if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
                log_pass "PTP for ${device}"
            else
                log_fail "PTP for ${device} (unreachable)"
            fi
            ;;
        olt-gpon)
            # Test OLT endpoint
            local url="http://localhost:${port}/api/v1/olt"
            if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
                log_pass "OLT for ${device}"
            else
                log_fail "OLT for ${device} (unreachable)"
            fi
            ;;
        enterprise)
            # Test SD-WAN endpoint
            local url="http://localhost:${port}/api/v1/sdwan"
            if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
                log_pass "SD-WAN for ${device}"
            else
                log_fail "SD-WAN for ${device} (unreachable)"
            fi
            ;;
        residential)
            # Test WiFi endpoint
            local url="http://localhost:${port}/api/v1/wifi"
            if response=$(curl -s --connect-timeout 2 "$url" 2>/dev/null); then
                log_pass "WiFi for ${device}"
            else
                log_fail "WiFi for ${device} (unreachable)"
            fi
            ;;
    esac
}

# Test all devices
test_all() {
    log_info "Testing REST API endpoints..."
    echo ""
    
    # Core Router (port 50051)
    if [[ -z "$DEVICE" || "$DEVICE" == "core-router" ]]; then
        log_info "Core Router (MTS-CR-9000) - port 50051"
        test_health "core-router" 50051
        test_system "core-router" 50051
        test_interfaces "core-router" 50051
        test_routing "core-router" 50051
        test_device_endpoints "core-router" 50051
        echo ""
    fi
    
    # Mobile Core (port 50052)
    if [[ -z "$DEVICE" || "$DEVICE" == "mobile-core" ]]; then
        log_info "Mobile Core (MTS-MC-5000) - port 50052"
        test_health "mobile-core" 50052
        test_system "mobile-core" 50052
        test_interfaces "mobile-core" 50052
        test_routing "mobile-core" 50052
        test_device_endpoints "mobile-core" 50052
        echo ""
    fi
    
    # Mobile Backhaul (port 50053)
    if [[ -z "$DEVICE" || "$DEVICE" == "mobile-backhaul" ]]; then
        log_info "Mobile Backhaul (MTS-MB-3000) - port 50053"
        test_health "mobile-backhaul" 50053
        test_system "mobile-backhaul" 50053
        test_interfaces "mobile-backhaul" 50053
        test_routing "mobile-backhaul" 50053
        test_device_endpoints "mobile-backhaul" 50053
        echo ""
    fi
    
    # OLT GPON (port 50054)
    if [[ -z "$DEVICE" || "$DEVICE" == "olt-gpon" ]]; then
        log_info "OLT GPON (MTS-OLT-2000) - port 50054"
        test_health "olt-gpon" 50054
        test_system "olt-gpon" 50054
        test_interfaces "olt-gpon" 50054
        test_routing "olt-gpon" 50054
        test_device_endpoints "olt-gpon" 50054
        echo ""
    fi
    
    # Enterprise Router (port 50055)
    if [[ -z "$DEVICE" || "$DEVICE" == "enterprise" ]]; then
        log_info "Enterprise Router (MTS-ER-1000) - port 50055"
        test_health "enterprise" 50055
        test_system "enterprise" 50055
        test_interfaces "enterprise" 50055
        test_routing "enterprise" 50055
        test_device_endpoints "enterprise" 50055
        echo ""
    fi
    
    # Residential Gateway (port 50056)
    if [[ -z "$DEVICE" || "$DEVICE" == "residential" ]]; then
        log_info "Residential Gateway (MTS-RG-500) - port 50056"
        test_health "residential" 50056
        test_system "residential" 50056
        test_interfaces "residential" 50056
        test_routing "residential" 50056
        test_device_endpoints "residential" 50056
        echo ""
    fi
}

# Print summary
print_summary() {
    echo ""
    echo -e "═══════════════════════════════════════════════════"
    echo -e "  REST API Test Summary"
    echo -e "═══════════════════════════════════════════════════"
    echo -e "  ${GREEN}Passed:  ${PASS}${NC}"
    echo -e "  ${RED}Failed:  ${FAIL}${NC}"
    echo -e "  ${YELLOW}Skipped: ${SKIP}${NC}"
    echo -e "  Total:   $((PASS + FAIL + SKIP))"
    echo -e "═══════════════════════════════════════════════════"
    echo ""
    
    if [[ $FAIL -gt 0 ]]; then
        exit 1
    fi
}

# Main
main() {
    log_info "MTS Router REST API Tests"
    log_info "Project: ${PROJECT_DIR}"
    log_info "Date: $(date +%Y-%m-%d\ %H:%M:%S)"
    echo ""
    
    test_all
    print_summary
}

main
