#!/bin/bash
# run-agent.sh — Запуск конкретного MTS Router development agent
#
# Usage:
#   ./run-agent.sh <agent-name> [--setup] [--build] [--test]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Available agents
AGENTS=(
    "core-router"
    "mobile-core"
    "mobile-backhaul"
    "olt-gpon"
    "enterprise"
    "residential"
    "linux"
    "firmware"
    "api"
    "qa"
)

# Parse arguments
AGENT=""
ACTION="all"

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <agent-name> [--setup|--build|--test|--all]"
    echo ""
    echo "Available agents:"
    for agent in "${AGENTS[@]}"; do
        echo "  - ${agent}"
    done
    exit 1
fi

AGENT="$1"
shift

# Validate agent
valid=false
for a in "${AGENTS[@]}"; do
    if [[ "$a" == "$AGENT" ]]; then
        valid=true
        break
    fi
done

if [[ "$valid" != "true" ]]; then
    echo -e "${RED}Error: Unknown agent '${AGENT}'${NC}" >&2
    exit 1
fi

# Parse action arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --setup) ACTION="setup"; shift ;;
        --build) ACTION="build"; shift ;;
        --test)  ACTION="test"; shift ;;
        --all)   ACTION="all"; shift ;;
        --help|-h)
            echo "Usage: $0 <agent-name> [--setup|--build|--test|--all]"
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

# Colors for output
log_info()  { echo -e "${GREEN}[$AGENT] $*${NC}"; }
log_warn()  { echo -e "${YELLOW}[$AGENT] $*${NC}"; }
log_error() { echo -e "${RED}[$AGENT] $*${NC}" >&2; }

# Agent-specific setup functions
setup_core_router() {
    log_info "Setting up Core Router (MTS-CR-9000)..."
    log_info "  - Tofino 2 driver spec: core-router/firmware/driver-spec.md"
    log_info "  - Board spec: core-router/chip/board-trace.md"
    log_info "  - Linux/yocto: core-router/linux/yocto-layer.md"
    log_info "  - Device tree: core-router/linux/device-tree.dts"
    log_info "  - API: core-router-api/"
    log_info "  - Spec: core-router/spec/core-router-spec.md"
    log_warn "Run ./scripts/build-core-router.sh to build"
}

setup_mobile_core() {
    log_info "Setting up Mobile Core (MTS-MC-5000)..."
    log_info "  - ThunderX3 driver spec: mobile-core/firmware/driver-spec.md"
    log_info "  - Linux/yocto+k3s: mobile-core/linux/yocto-k3s-layer.md"
    log_info "  - Device tree: mobile-core/linux/device-tree.dts"
    log_info "  - API: mobile-core-api/"
    log_info "  - Spec: mobile-core/spec/mobile-core-spec.md"
    log_warn "Run ./scripts/build-mobile-core.sh to build"
}

setup_mobile_backhaul() {
    log_info "Setting up Mobile Backhaul (MTS-MB-3000)..."
    log_info "  - S32G3 driver spec: mobile-backhaul/firmware/driver-spec.md"
    log_info "  - Linux/buildroot: mobile-backhaul/linux/buildroot-config.md"
    log_info "  - Device tree: mobile-backhaul/linux/device-tree.dts"
    log_info "  - API: mts-mb3000-api/"
    log_info "  - Spec: mobile-backhaul/spec/mobile-backhaul-spec.md"
    log_warn "Run ./scripts/build-mobile-backhaul.sh to build"
}

setup_olt_gpon() {
    log_info "Setting up OLT GPON (MTS-OLT-2000)..."
    log_info "  - RTL960x driver spec: olt-gpon/firmware/driver-spec.md"
    log_info "  - Linux/openwrt: olt-gpon/linux/openwrt-layer.md"
    log_info "  - Device tree: olt-gpon/linux/device-tree.dts"
    log_info "  - API: olt-gpon-api/"
    log_info "  - Spec: olt-gpon/spec/olt-gpon-spec.md"
    log_warn "Run ./scripts/build-olt-gpon.sh to build"
}

setup_enterprise() {
    log_info "Setting up Enterprise Router (MTS-ER-1000)..."
    log_info "  - S32G3 driver spec: enterprise-router/firmware/driver-spec.md"
    log_info "  - Linux/openwrt: enterprise-router/linux/openwrt-layer.md"
    log_info "  - Device tree: enterprise-router/linux/device-tree.dts"
    log_info "  - API: enterprise-router-api/"
    log_info "  - Spec: enterprise-router/spec/enterprise-router-spec.md"
    log_warn "Run ./scripts/build-enterprise.sh to build"
}

setup_residential() {
    log_info "Setting up Residential Gateway (MTS-RG-500)..."
    log_info "  - MT7981 driver spec: residential-gateway/firmware/driver-spec.md"
    log_info "  - Linux/openwrt: residential-gateway/linux/openwrt-layer.md"
    log_info "  - Device tree: residential-gateway/linux/device-tree.dts"
    log_info "  - API: residential-gateway-api/"
    log_info "  - Spec: residential-gateway/spec/residential-gateway-spec.md"
    log_warn "Run ./scripts/build-residential.sh to build"
}

setup_linux() {
    log_info "Setting up meta-mts Yocto layer..."
    log_info "  - Layer: linux/meta-mts/"
    log_info "  - Classes: linux/meta-mts/classes/"
    log_info "  - Recipes: linux/meta-mts/recipes-*/"
    log_info "  - README: linux/meta-mts/README.md"
    log_warn "Configure with: source oe-init-build-env && bitbake-layers add-layer meta-mts"
}

setup_firmware() {
    log_info "Setting up firmware drivers..."
    local drivers=(
        "core-router/firmware/driver-spec.md"
        "mobile-core/firmware/driver-spec.md"
        "mobile-backhaul/firmware/driver-spec.md"
        "olt-gpon/firmware/driver-spec.md"
        "enterprise-router/firmware/driver-spec.md"
        "residential-gateway/firmware/driver-spec.md"
    )
    for driver in "${drivers[@]}"; do
        if [[ -f "${PROJECT_DIR}/${driver}" ]]; then
            log_info "  ✓ ${driver}"
        else
            log_warn "  ✗ ${driver} (not found)"
        fi
    done
}

setup_api() {
    log_info "Setting up API/SDK..."
    log_info "  - API spec: api/spec/mts-router-api.md"
    log_info "  - Protobuf: api/spec/mts-api-protobuf.md"
    log_info "  - OpenAPI: api/spec/mts-router-openapi.yaml"
    log_info "  - Python SDK: api/sdk/python/"
    log_info "  - Go SDK: api/sdk/go/"
    log_info "  - Java SDK: api/sdk/java/"
    log_info "  - Examples: api/examples/"
}

setup_qa() {
    log_info "Setting up QA/CI/CD..."
    log_info "  - CI/CD: .github/workflows/ci-cd.yml"
    log_info "  - Build scripts: scripts/build-*.sh"
    log_info "  - Test scripts: scripts/test-*.py"
    log_warn "Run ./scripts/test-all.sh for full test suite"
}

# Agent-specific build functions
build_core_router()   { bash "${PROJECT_DIR}/scripts/build-core-router.sh"; }
build_mobile_core()   { bash "${PROJECT_DIR}/scripts/build-mobile-core.sh"; }
build_mobile_backhaul() { bash "${PROJECT_DIR}/scripts/build-mobile-backhaul.sh"; }
build_olt_gpon()      { bash "${PROJECT_DIR}/scripts/build-olt-gpon.sh"; }
build_enterprise()    { bash "${PROJECT_DIR}/scripts/build-enterprise.sh"; }
build_residential()   { bash "${PROJECT_DIR}/scripts/build-residential.sh"; }

# Agent-specific test functions
test_core_router()   { bash "${PROJECT_DIR}/scripts/test-all.sh" --agent core-router; }
test_mobile_core()   { bash "${PROJECT_DIR}/scripts/test-all.sh" --agent mobile-core; }
test_mobile_backhaul() { bash "${PROJECT_DIR}/scripts/test-all.sh" --agent mobile-backhaul; }
test_olt_gpon()      { bash "${PROJECT_DIR}/scripts/test-all.sh" --agent olt-gpon; }
test_enterprise()    { bash "${PROJECT_DIR}/scripts/test-all.sh" --agent enterprise; }
test_residential()   { bash "${PROJECT_DIR}/scripts/test-all.sh" --agent residential; }

# Main
case "$ACTION" in
    setup)
        case "$AGENT" in
            core-router)       setup_core_router ;;
            mobile-core)       setup_mobile_core ;;
            mobile-backhaul)   setup_mobile_backhaul ;;
            olt-gpon)          setup_olt_gpon ;;
            enterprise)        setup_enterprise ;;
            residential)       setup_residential ;;
            linux)             setup_linux ;;
            firmware)          setup_firmware ;;
            api)               setup_api ;;
            qa)                setup_qa ;;
        esac
        ;;
    build)
        case "$AGENT" in
            core-router)       build_core_router ;;
            mobile-core)       build_mobile_core ;;
            mobile-backhaul)   build_mobile_backhaul ;;
            olt-gpon)          build_olt_gpon ;;
            enterprise)        build_enterprise ;;
            residential)       build_residential ;;
            *) log_error "Build not supported for agent: ${AGENT}" ;;
        esac
        ;;
    test)
        case "$AGENT" in
            core-router)       test_core_router ;;
            mobile-core)       test_mobile_core ;;
            mobile-backhaul)   test_mobile_backhaul ;;
            olt-gpon)          test_olt_gpon ;;
            enterprise)        test_enterprise ;;
            residential)       test_residential ;;
            *) log_error "Test not supported for agent: ${AGENT}" ;;
        esac
        ;;
    all)
        case "$AGENT" in
            core-router)       setup_core_router; build_core_router ;;
            mobile-core)       setup_mobile_core; build_mobile_core ;;
            mobile-backhaul)   setup_mobile_backhaul; build_mobile_backhaul ;;
            olt-gpon)          setup_olt_gpon; build_olt_gpon ;;
            enterprise)        setup_enterprise; build_enterprise ;;
            residential)       setup_residential; build_residential ;;
            linux)             setup_linux ;;
            firmware)          setup_firmware ;;
            api)               setup_api ;;
            qa)                setup_qa ;;
        esac
        ;;
esac

log_info "Agent ${AGENT} ${ACTION} completed."
