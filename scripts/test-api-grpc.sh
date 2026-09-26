#!/bin/bash
# test-api-grpc.sh — Тестирование gRPC API всех устройств
#
# Usage:
#   ./test-api-grpc.sh [--device <device>] [--verbose]

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

# Check if grpc_tools_node_protoc or grpcurl is available
has_grpcurl=false
if command -v grpcurl &>/dev/null; then
    has_grpcurl=true
fi

# Check if python3 grpc is available
has_python_grpc=false
if python3 -c "import grpc" 2>/dev/null; then
    has_python_grpc=true
fi

# Test gRPC health check
test_grpc_health() {
    local device="$1"
    local port="$2"
    
    if [[ "$VERBOSE" == true ]]; then
        log_info "Testing gRPC health for ${device} on port ${port}"
    fi
    
    if [[ "$has_grpcurl" == true ]]; then
        if grpcurl -plaintext -port "${port}" list 2>/dev/null | grep -q "mts"; then
            log_pass "gRPC health for ${device}"
            return 0
        fi
    elif [[ "$has_python_grpc" == true ]]; then
        python3 -c "
import grpc
import sys
try:
    channel = grpc.insecure_channel('localhost:${port}')
    health = grpc.health.v1.health_pb2.HealthCheckRequest()
    response = grpc.channel_ready_future(channel).result(timeout=2)
    print('healthy')
    sys.exit(0)
except:
    sys.exit(1)
" 2>/dev/null && {
            log_pass "gRPC health for ${device}"
            return 0
        }
    fi
    
    log_fail "gRPC health for ${device} (unreachable)"
    return 1
}

# Test gRPC service listing
test_grpc_services() {
    local device="$1"
    local port="$2"
    local expected_service="$3"
    
    if [[ "$VERBOSE" == true ]]; then
        log_info "Testing gRPC services for ${device}"
    fi
    
    if [[ "$has_grpcurl" == true ]]; then
        if services=$(grpcurl -plaintext -port "${port}" list 2>/dev/null); then
            if echo "$services" | grep -q "${expected_service}"; then
                log_pass "gRPC service ${expected_service} for ${device}"
                return 0
            fi
        fi
    fi
    
    log_fail "gRPC service ${expected_service} for ${device}"
    return 1
}

# Test protobuf compile
test_proto_compile() {
    local proto_dir="$1"
    local proto_file="$2"
    
    if [[ "$VERBOSE" == true ]]; then
        log_info "Testing protobuf compile: ${proto_file}"
    fi
    
    if [[ -f "${PROJECT_DIR}/${proto_file}" ]]; then
        if protoc --proto_path="${PROJECT_DIR}/${proto_dir}" \
                  --python_out="/tmp" \
                  "${proto_file}" 2>/dev/null; then
            log_pass "Proto compile: ${proto_file}"
            rm -f "/tmp/${proto_file%.proto}_pb2.py" 2>/dev/null
            return 0
        fi
    fi
    
    log_fail "Proto compile: ${proto_file}"
    return 1
}

# Test all devices
test_all() {
    log_info "Testing gRPC API endpoints..."
    echo ""
    
    # Core Router
    if [[ -z "$DEVICE" || "$DEVICE" == "core-router" ]]; then
        log_info "Core Router (MTS-CR-9000) - port 50051"
        test_grpc_health "core-router" 50051
        test_grpc_services "core-router" 50051 "mts.core.router"
        test_proto_compile "core-router-api/proto" "core-router-api/proto/mts_core_router.proto"
        echo ""
    fi
    
    # Mobile Core
    if [[ -z "$DEVICE" || "$DEVICE" == "mobile-core" ]]; then
        log_info "Mobile Core (MTS-MC-5000) - port 50052"
        test_grpc_health "mobile-core" 50052
        test_grpc_services "mobile-core" 50052 "mts.mobile.core"
        test_proto_compile "mobile-core-api/proto" "mobile-core-api/proto/mts_mobile_core.proto"
        echo ""
    fi
    
    # Mobile Backhaul
    if [[ -z "$DEVICE" || "$DEVICE" == "mobile-backhaul" ]]; then
        log_info "Mobile Backhaul (MTS-MB-3000) - port 50053"
        test_grpc_health "mobile-backhaul" 50053
        test_grpc_services "mobile-backhaul" 50053 "mts.backhaul"
        test_proto_compile "mts-mb3000-api/proto" "mts-mb3000-api/proto/mts_backhaul.proto"
        echo ""
    fi
    
    # OLT GPON
    if [[ -z "$DEVICE" || "$DEVICE" == "olt-gpon" ]]; then
        log_info "OLT GPON (MTS-OLT-2000) - port 50054"
        test_grpc_health "olt-gpon" 50054
        test_grpc_services "olt-gpon" 50054 "mts.olt"
        test_proto_compile "olt-gpon-api/proto" "olt-gpon-api/proto/mts_olt_gpon.proto"
        echo ""
    fi
    
    # Enterprise Router
    if [[ -z "$DEVICE" || "$DEVICE" == "enterprise" ]]; then
        log_info "Enterprise Router (MTS-ER-1000) - port 50055"
        test_grpc_health "enterprise" 50055
        test_grpc_services "enterprise" 50055 "mts.enterprise"
        test_proto_compile "enterprise-router-api/proto" "enterprise-router-api/proto/mts_enterprise.proto"
        echo ""
    fi
    
    # Residential Gateway
    if [[ -z "$DEVICE" || "$DEVICE" == "residential" ]]; then
        log_info "Residential Gateway (MTS-RG-500) - port 50056"
        test_grpc_health "residential" 50056
        test_grpc_services "residential" 50056 "mts.residential"
        test_proto_compile "residential-gateway-api/proto" "residential-gateway-api/proto/mts_residential.proto"
        echo ""
    fi
}

# Print summary
print_summary() {
    echo ""
    echo -e "═══════════════════════════════════════════════════"
    echo -e "  gRPC API Test Summary"
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
    log_info "MTS Router gRPC API Tests"
    log_info "Project: ${PROJECT_DIR}"
    log_info "Date: $(date +%Y-%m-%d\ %H:%M:%S)"
    echo ""
    
    test_all
    print_summary
}

main
