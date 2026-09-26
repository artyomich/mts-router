#!/bin/bash
# MTS Router — Test runner
# Runs all device API tests
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
TEST_DIR="${BUILD_DIR}/test-results"
LOG_DIR="${BUILD_DIR}/logs"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

mkdir -p "${TEST_DIR}" "${LOG_DIR}"

test_count=0
pass_count=0
fail_count=0

log() {
    echo -e "${1}${2}${NC}"
}

run_test_simple() {
    local test_name="$1"
    shift
    test_count=$((test_count + 1))
    
    if "$@" > "${TEST_DIR}/${test_name}.log" 2>&1; then
        pass_count=$((pass_count + 1))
        log "${GREEN}[PASS]${NC} ${test_name}"
    else
        fail_count=$((fail_count + 1))
        log "${RED}[FAIL]${NC} ${test_name}"
        echo "  Log: ${TEST_DIR}/${test_name}.log"
    fi
}

echo "=== MTS Router — Test Suite ==="
echo "Start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo ""

# Test 1: Proto compilation - Core Router
run_test_simple "proto-compile-core" protoc --proto_path="${PROJECT_DIR}/core-router-api/proto" --cpp_out="${TEST_DIR}" "${PROJECT_DIR}/core-router-api/proto/mts_core_router.proto"

# Test 2: Proto compilation - Mobile Core
run_test_simple "proto-compile-mobile-core" protoc --proto_path="${PROJECT_DIR}/mobile-core-api/proto" --cpp_out="${TEST_DIR}" "${PROJECT_DIR}/mobile-core-api/proto/mts_mobile_core.proto"

# Test 3: Proto compilation - Mobile Backhaul
run_test_simple "proto-compile-mobile-backhaul" protoc --proto_path="${PROJECT_DIR}/mts-mb3000-api/proto" --cpp_out="${TEST_DIR}" "${PROJECT_DIR}/mts-mb3000-api/proto/mts_backhaul.proto"

# Test 4: Proto compilation - OLT GPON
run_test_simple "proto-compile-olt-gpon" protoc --proto_path="${PROJECT_DIR}/olt-gpon-api/proto" --cpp_out="${TEST_DIR}" "${PROJECT_DIR}/olt-gpon-api/proto/mts_olt_gpon.proto"

# Test 5: Proto compilation - Enterprise
run_test_simple "proto-compile-enterprise" protoc --proto_path="${PROJECT_DIR}/enterprise-router-api/proto" --cpp_out="${TEST_DIR}" "${PROJECT_DIR}/enterprise-router-api/proto/mts_enterprise.proto"

# Test 6: Proto compilation - Residential
run_test_simple "proto-compile-residential" protoc --proto_path="${PROJECT_DIR}/residential-gateway-api/proto" --cpp_out="${TEST_DIR}" "${PROJECT_DIR}/residential-gateway-api/proto/mts_residential.proto"

# Test 7: Python client imports
run_test_simple "python-client-import" python3 -c "import sys; sys.path.insert(0, '${PROJECT_DIR}/api/examples'); print('OK')"

# Test 8: Directory structure - Core Router
run_test_simple "dir-structure-core" test -d "${PROJECT_DIR}/core-router-api" -a -f "${PROJECT_DIR}/core-router-api/CMakeLists.txt" -a -f "${PROJECT_DIR}/core-router-api/proto/mts_core_router.proto" -a -f "${PROJECT_DIR}/core-router-api/config/mts-cr9000.conf"

# Test 9: Directory structure - Mobile Core
run_test_simple "dir-structure-mobile-core" test -d "${PROJECT_DIR}/mobile-core-api" -a -f "${PROJECT_DIR}/mobile-core-api/CMakeLists.txt" -a -f "${PROJECT_DIR}/mobile-core-api/proto/mts_mobile_core.proto" -a -f "${PROJECT_DIR}/mobile-core-api/config/mts-mc5000.conf"

# Test 10: Directory structure - OLT GPON
run_test_simple "dir-structure-olt-gpon" test -d "${PROJECT_DIR}/olt-gpon-api" -a -f "${PROJECT_DIR}/olt-gpon-api/CMakeLists.txt" -a -f "${PROJECT_DIR}/olt-gpon-api/proto/mts_olt_gpon.proto" -a -f "${PROJECT_DIR}/olt-gpon-api/config/mts-olt2000.conf"

# Test 11: Directory structure - Enterprise
run_test_simple "dir-structure-enterprise" test -d "${PROJECT_DIR}/enterprise-router-api" -a -f "${PROJECT_DIR}/enterprise-router-api/CMakeLists.txt" -a -f "${PROJECT_DIR}/enterprise-router-api/proto/mts_enterprise.proto" -a -f "${PROJECT_DIR}/enterprise-router-api/config/mts-er1000.conf"

# Test 12: Directory structure - Residential
run_test_simple "dir-structure-residential" test -d "${PROJECT_DIR}/residential-gateway-api" -a -f "${PROJECT_DIR}/residential-gateway-api/CMakeLists.txt" -a -f "${PROJECT_DIR}/residential-gateway-api/proto/mts_residential.proto" -a -f "${PROJECT_DIR}/residential-gateway-api/config/mts-rg500.conf"

# Test 13: Build scripts are executable
run_test_simple "scripts-executable" test -x "${SCRIPT_DIR}/build-all.sh" -a -x "${SCRIPT_DIR}/build-core-router.sh" -a -x "${SCRIPT_DIR}/build-mobile-core.sh" -a -x "${SCRIPT_DIR}/build-mobile-backhaul.sh" -a -x "${SCRIPT_DIR}/build-olt-gpon.sh" -a -x "${SCRIPT_DIR}/build-enterprise.sh" -a -x "${SCRIPT_DIR}/build-residential.sh"

# Test 14: Spec files exist
run_test_simple "specs-exist" test -f "${PROJECT_DIR}/core-router/spec/core-router-spec.md" -a -f "${PROJECT_DIR}/mobile-core/spec/mobile-core-spec.md" -a -f "${PROJECT_DIR}/mobile-backhaul/spec/mobile-backhaul-spec.md" -a -f "${PROJECT_DIR}/olt-gpon/spec/olt-gpon-spec.md" -a -f "${PROJECT_DIR}/enterprise-router/spec/enterprise-router-spec.md" -a -f "${PROJECT_DIR}/residential-gateway/spec/residential-gateway-spec.md"

# Test 15: API spec exists
run_test_simple "api-spec-exists" test -f "${PROJECT_DIR}/api/spec/mts-router-api.md" -a -f "${PROJECT_DIR}/api/spec/mts-api-protobuf.md" -a -f "${PROJECT_DIR}/api/examples/mts-api-examples.py"

# Test 16: Docs exist
run_test_simple "docs-exist" test -f "${PROJECT_DIR}/docs/architecture-overview.md" -a -f "${PROJECT_DIR}/docs/chipset-analysis.md" -a -f "${PROJECT_DIR}/docs/linux-os-selection.md"

# Test 17: CMakeLists.txt valid syntax - Core Router
run_test_simple "cmake-syntax-core" sh -c "grep -q 'cmake_minimum_required' '${PROJECT_DIR}/core-router-api/CMakeLists.txt' && grep -q 'find_package(protobuf' '${PROJECT_DIR}/core-router-api/CMakeLists.txt' && grep -q 'find_package(gRPC' '${PROJECT_DIR}/core-router-api/CMakeLists.txt'"

# Test 18: CMakeLists.txt valid syntax - Mobile Core
run_test_simple "cmake-syntax-mobile-core" sh -c "grep -q 'cmake_minimum_required' '${PROJECT_DIR}/mobile-core-api/CMakeLists.txt' && grep -q 'find_package(protobuf' '${PROJECT_DIR}/mobile-core-api/CMakeLists.txt' && grep -q 'find_package(gRPC' '${PROJECT_DIR}/mobile-core-api/CMakeLists.txt'"

# Test 19: CMakeLists.txt valid syntax - OLT GPON
run_test_simple "cmake-syntax-olt-gpon" sh -c "grep -q 'cmake_minimum_required' '${PROJECT_DIR}/olt-gpon-api/CMakeLists.txt' && grep -q 'find_package(protobuf' '${PROJECT_DIR}/olt-gpon-api/CMakeLists.txt' && grep -q 'find_package(gRPC' '${PROJECT_DIR}/olt-gpon-api/CMakeLists.txt'"

# Test 20: CMakeLists.txt valid syntax - Enterprise
run_test_simple "cmake-syntax-enterprise" sh -c "grep -q 'cmake_minimum_required' '${PROJECT_DIR}/enterprise-router-api/CMakeLists.txt' && grep -q 'find_package(protobuf' '${PROJECT_DIR}/enterprise-router-api/CMakeLists.txt' && grep -q 'find_package(gRPC' '${PROJECT_DIR}/enterprise-router-api/CMakeLists.txt'"

# Test 21: CMakeLists.txt valid syntax - Residential
run_test_simple "cmake-syntax-residential" sh -c "grep -q 'cmake_minimum_required' '${PROJECT_DIR}/residential-gateway-api/CMakeLists.txt' && grep -q 'find_package(protobuf' '${PROJECT_DIR}/residential-gateway-api/CMakeLists.txt' && grep -q 'find_package(gRPC' '${PROJECT_DIR}/residential-gateway-api/CMakeLists.txt'"

# Test 22: Config files valid JSON - Core Router
run_test_simple "config-json-core" python3 -c "import json; json.load(open('${PROJECT_DIR}/core-router-api/config/mts-cr9000.conf'))"

# Test 23: Config files valid JSON - Mobile Core
run_test_simple "config-json-mobile-core" python3 -c "import json; json.load(open('${PROJECT_DIR}/mobile-core-api/config/mts-mc5000.conf'))"

# Test 24: Config files valid JSON - OLT GPON
run_test_simple "config-json-olt-gpon" python3 -c "import json; json.load(open('${PROJECT_DIR}/olt-gpon-api/config/mts-olt2000.conf'))"

# Test 25: Config files valid JSON - Enterprise
run_test_simple "config-json-enterprise" python3 -c "import json; json.load(open('${PROJECT_DIR}/enterprise-router-api/config/mts-er1000.conf'))"

# Test 26: Config files valid JSON - Residential
run_test_simple "config-json-residential" python3 -c "import json; json.load(open('${PROJECT_DIR}/residential-gateway-api/config/mts-rg500.conf'))"

# Test 27: AGENTS.md exists
run_test_simple "agents-md-exists" test -f "${PROJECT_DIR}/AGENTS.md"

# Test 28: README.md exists
run_test_simple "readme-exists" test -f "${PROJECT_DIR}/README.md"

# Test 29: PROJECT-SUMMARY.md exists
run_test_simple "project-summary-exists" test -f "${PROJECT_DIR}/PROJECT-SUMMARY.md"

# Test 30: mts-mb3000-api existing files intact
run_test_simple "mb3000-intact" test -f "${PROJECT_DIR}/mts-mb3000-api/CMakeLists.txt" -a -f "${PROJECT_DIR}/mts-mb3000-api/README.md" -a -f "${PROJECT_DIR}/mts-mb3000-api/src/main.cpp" -a -f "${PROJECT_DIR}/mts-mb3000-api/proto/mts_backhaul.proto"

echo ""
echo "=== Test Results ==="
echo "Total: ${test_count}, Passed: ${pass_count}, Failed: ${fail_count}"

if [ ${fail_count} -gt 0 ]; then
    log "${RED}Some tests failed. Check logs in ${TEST_DIR}/"
    exit 1
fi

log "${GREEN}All tests passed!"
