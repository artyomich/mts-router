#!/bin/bash
# test-sdk.sh — Тестирование SDK для всех языков
#
# Usage:
#   ./test-sdk.sh [--lang python|go|java|all] [--verbose]

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
LANG=""
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --lang)
            LANG="$2"
            shift 2
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--lang python|go|java|all] [--verbose]"
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

# Test Python SDK
test_python_sdk() {
    log_info "Testing Python SDK..."
    
    local sdk_dir="${PROJECT_DIR}/api/sdk/python"
    local sdk_file="${sdk_dir}/mts_router_sdk.py"
    
    if [[ ! -f "$sdk_file" ]]; then
        log_skip "Python SDK not found at ${sdk_file}"
        return 1
    fi
    
    # Check syntax
    if python3 -m py_compile "$sdk_file" 2>/dev/null; then
        log_pass "Python SDK syntax check"
    else
        log_fail "Python SDK syntax check"
        return 1
    fi
    
    # Check imports
    if python3 -c "
import sys
sys.path.insert(0, '${sdk_dir}')
import mts_router_sdk
print('import ok')
" 2>/dev/null; then
        log_pass "Python SDK import check"
    else
        log_fail "Python SDK import check"
        return 1
    fi
    
    # Check class definitions
    if python3 -c "
import sys
sys.path.insert(0, '${sdk_dir}')
import mts_router_sdk
assert hasattr(mts_router_sdk, 'MtsRouterClient')
print('class ok')
" 2>/dev/null; then
        log_pass "Python SDK class check"
    else
        log_fail "Python SDK class check"
        return 1
    fi
    
    return 0
}

# Test Go SDK
test_go_sdk() {
    log_info "Testing Go SDK..."
    
    local sdk_dir="${PROJECT_DIR}/api/sdk/go"
    local go_mod="${sdk_dir}/go.mod"
    
    if [[ ! -f "$go_mod" ]]; then
        log_skip "Go SDK go.mod not found at ${go_mod}"
        return 1
    fi
    
    # Check go.mod syntax
    if grep -q "module " "$go_mod" 2>/dev/null; then
        log_pass "Go SDK go.mod check"
    else
        log_fail "Go SDK go.mod check"
        return 1
    fi
    
    # Check client file exists
    local client_file="${sdk_dir}/mtsrouter/client.go"
    if [[ -f "$client_file" ]]; then
        if grep -q "type MtsRouterClient struct" "$client_file" 2>/dev/null; then
            log_pass "Go SDK client struct check"
        else
            log_fail "Go SDK client struct check"
            return 1
        fi
    else
        log_fail "Go SDK client file not found"
        return 1
    fi
    
    return 0
}

# Test Java SDK
test_java_sdk() {
    log_info "Testing Java SDK..."
    
    local sdk_dir="${PROJECT_DIR}/api/sdk/java"
    local client_file="${sdk_dir}/MtsRouterClient.java"
    
    if [[ ! -f "$client_file" ]]; then
        log_skip "Java SDK not found at ${client_file}"
        return 1
    fi
    
    # Check class definition
    if grep -q "public class MtsRouterClient" "$client_file" 2>/dev/null; then
        log_pass "Java SDK class check"
    else
        log_fail "Java SDK class check"
        return 1
    fi
    
    # Check constructor
    if grep -q "public MtsRouterClient" "$client_file" 2>/dev/null; then
        log_pass "Java SDK constructor check"
    else
        log_fail "Java SDK constructor check"
        return 1
    fi
    
    return 0
}

# Test all SDKs
test_all() {
    log_info "Testing MTS Router SDKs..."
    echo ""
    
    case "$LANG" in
        python|Python)
            test_python_sdk
            ;;
        go|Go)
            test_go_sdk
            ;;
        java|Java)
            test_java_sdk
            ;;
        all|All|"")
            test_python_sdk
            echo ""
            test_go_sdk
            echo ""
            test_java_sdk
            ;;
        *)
            log_fail "Unknown language: ${LANG}"
            exit 1
            ;;
    esac
}

# Print summary
print_summary() {
    echo ""
    echo -e "═══════════════════════════════════════════════════"
    echo -e "  SDK Test Summary"
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
    log_info "MTS Router SDK Tests"
    log_info "Project: ${PROJECT_DIR}"
    log_info "Date: $(date +%Y-%m-%d\ %H:%M:%S)"
    echo ""
    
    test_all
    print_summary
}

main
