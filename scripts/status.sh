#!/bin/bash
# status.sh — Показание статуса всех MTS Router компонентов
#
# Usage:
#   ./status.sh [--json] [--agents agent1,agent2,...]

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

# Check if file exists and is non-empty
check_file() {
    local file="$1"
    if [[ -f "${PROJECT_DIR}/${file}" ]]; then
        if [[ -s "${PROJECT_DIR}/${file}" ]]; then
            echo -e "${GREEN}✓${NC}"
            return 0
        else
            echo -e "${YELLOW}○${NC}"
            return 1
        fi
    else
        echo -e "${RED}✗${NC}"
        return 1
    fi
}

# Check directory exists
check_dir() {
    local dir="$1"
    if [[ -d "${PROJECT_DIR}/${dir}" ]]; then
        local count=$(find "${PROJECT_DIR}/${dir}" -type f | wc -l)
        echo -e "${GREEN}✓${NC} (${count} files)"
        return 0
    else
        echo -e "${RED}✗${NC}"
        return 1
    fi
}

# Check command exists
check_command() {
    if command -v "$1" &>/dev/null; then
        local version=$("$1" --version 2>&1 | head -1)
        echo -e "${GREEN}✓${NC} (${version})"
        return 0
    else
        echo -e "${RED}✗${NC}"
        return 1
    fi
}

# Print section header
print_header() {
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
}

# Print status line
print_status() {
    local name="$1"
    local status="$2"
    local detail="${3:-}"
    
    printf "  %-40s " "$name"
    echo -e "$status"
    if [[ -n "$detail" ]]; then
        printf "  %-40s %s\n" "" "$detail"
    fi
}

# Check Core Router
check_core_router() {
    print_header "Core Router (MTS-CR-9000)"
    check_file "core-router/spec/core-router-spec.md" && print_status "Spec" "ready"
    check_file "core-router/chip/board-trace.md" && print_status "Board trace" "ready"
    check_file "core-router/linux/yocto-layer.md" && print_status "Yocto layer" "ready"
    check_file "core-router/linux/device-tree.dts" && print_status "Device tree" "ready"
    check_file "core-router/firmware/driver-spec.md" && print_status "Driver spec" "ready"
    check_dir "core-router-api" && print_status "API" "implemented"
}

# Check Mobile Core
check_mobile_core() {
    print_header "Mobile Core (MTS-MC-5000)"
    check_file "mobile-core/spec/mobile-core-spec.md" && print_status "Spec" "ready"
    check_file "mobile-core/linux/yocto-k3s-layer.md" && print_status "Yocto+K3s" "ready"
    check_file "mobile-core/linux/device-tree.dts" && print_status "Device tree" "ready"
    check_file "mobile-core/firmware/driver-spec.md" && print_status "Driver spec" "ready"
    check_dir "mobile-core-api" && print_status "API" "implemented"
}

# Check Mobile Backhaul
check_mobile_backhaul() {
    print_header "Mobile Backhaul (MTS-MB-3000)"
    check_file "mobile-backhaul/spec/mobile-backhaul-spec.md" && print_status "Spec" "ready"
    check_file "mobile-backhaul/linux/buildroot-config.md" && print_status "Buildroot" "ready"
    check_file "mobile-backhaul/linux/device-tree.dts" && print_status "Device tree" "ready"
    check_file "mobile-backhaul/firmware/driver-spec.md" && print_status "Driver spec" "ready"
    check_dir "mts-mb3000-api" && print_status "API" "implemented"
}

# Check OLT GPON
check_olt_gpon() {
    print_header "OLT GPON (MTS-OLT-2000)"
    check_file "olt-gpon/spec/olt-gpon-spec.md" && print_status "Spec" "ready"
    check_file "olt-gpon/linux/openwrt-layer.md" && print_status "OpenWrt" "ready"
    check_file "olt-gpon/linux/device-tree.dts" && print_status "Device tree" "ready"
    check_file "olt-gpon/firmware/driver-spec.md" && print_status "Driver spec" "ready"
    check_dir "olt-gpon-api" && print_status "API" "implemented"
}

# Check Enterprise Router
check_enterprise() {
    print_header "Enterprise Router (MTS-ER-1000)"
    check_file "enterprise-router/spec/enterprise-router-spec.md" && print_status "Spec" "ready"
    check_file "enterprise-router/linux/openwrt-layer.md" && print_status "OpenWrt" "ready"
    check_file "enterprise-router/linux/device-tree.dts" && print_status "Device tree" "ready"
    check_file "enterprise-router/firmware/driver-spec.md" && print_status "Driver spec" "ready"
    check_dir "enterprise-router-api" && print_status "API" "implemented"
}

# Check Residential Gateway
check_residential() {
    print_header "Residential Gateway (MTS-RG-500)"
    check_file "residential-gateway/spec/residential-gateway-spec.md" && print_status "Spec" "ready"
    check_file "residential-gateway/linux/openwrt-layer.md" && print_status "OpenWrt" "ready"
    check_file "residential-gateway/linux/device-tree.dts" && print_status "Device tree" "ready"
    check_file "residential-gateway/firmware/driver-spec.md" && print_status "Driver spec" "ready"
    check_dir "residential-gateway-api" && print_status "API" "implemented"
}

# Check Linux meta-mts
check_linux() {
    print_header "Linux meta-mts Yocto Layer"
    check_file "linux/meta-mts/README.md" && print_status "README" "ready"
    check_file "linux/meta-mts/conf/layer.conf" && print_status "layer.conf" "ready"
    check_file "linux/meta-mts/classes/mts-board.bbclass" && print_status "bbclass" "ready"
    check_file "linux/meta-mts/recipes-kernel/linux/mts-kernel_%.bbappend" && print_status "kernel recipe" "ready"
    check_file "linux/meta-mts/recipes-core/images/mts-image.bbappend" && print_status "image recipe" "ready"
}

# Check API/SDK
check_api() {
    print_header "API/SDK"
    check_file "api/spec/mts-router-api.md" && print_status "API spec" "ready"
    check_file "api/spec/mts-api-protobuf.md" && print_status "Protobuf spec" "ready"
    check_file "api/spec/mts-router-openapi.yaml" && print_status "OpenAPI" "ready"
    check_dir "api/sdk/python" && print_status "Python SDK" "ready"
    check_dir "api/sdk/go" && print_status "Go SDK" "ready"
    check_dir "api/sdk/java" && print_status "Java SDK" "ready"
    check_dir "api/examples" && print_status "Examples" "ready"
    check_file "api/spec/mts-common.yang" && print_status "YANG: common" "ready"
    check_file "api/spec/mts-interface.yang" && print_status "YANG: interface" "ready"
    check_file "api/spec/mts-routing.yang" && print_status "YANG: routing" "ready"
    check_file "api/spec/mts-bgp.yang" && print_status "YANG: bgp" "ready"
    check_file "api/spec/mts-qos.yang" && print_status "YANG: qos" "ready"
    check_file "api/spec/mts-telemetry.yang" && print_status "YANG: telemetry" "ready"
    check_file "api/spec/mts-ha.yang" && print_status "YANG: ha" "ready"
    check_file "api/spec/mts-firmware.yang" && print_status "YANG: firmware" "ready"
    check_file "api/spec/mts-users.yang" && print_status "YANG: users" "ready"
    check_file "api/spec/mts-cr.yang" && print_status "YANG: core-router" "ready"
    check_file "api/spec/mts-mc.yang" && print_status "YANG: mobile-core" "ready"
    check_file "api/spec/mts-mb.yang" && print_status "YANG: mobile-backhaul" "ready"
    check_file "api/spec/mts-olt.yang" && print_status "YANG: olt-gpon" "ready"
    check_file "api/spec/mts-er.yang" && print_status "YANG: enterprise" "ready"
    check_file "api/spec/mts-rg.yang" && print_status "YANG: residential" "ready"
}

# Check CI/CD
check_cicd() {
    print_header "CI/CD"
    check_file ".github/workflows/ci-cd.yml" && print_status "GitHub Actions" "ready"
    check_file "scripts/build-all.sh" && print_status "Build script" "ready"
    check_file "scripts/test-all.sh" && print_status "Test script" "ready"
    check_file "scripts/test-all.py" && print_status "Python test" "ready"
    check_file "scripts/deploy.sh" && print_status "Deploy script" "ready"
}

# Check dependencies
check_dependencies() {
    print_header "Dependencies"
    printf "  %-40s " "gcc"
    check_command "gcc" 2>/dev/null || echo -e "${RED}✗${NC}"
    
    printf "  %-40s " "cmake"
    check_command "cmake" 2>/dev/null || echo -e "${RED}✗${NC}"
    
    printf "  %-40s " "protoc"
    check_command "protoc" 2>/dev/null || echo -e "${RED}✗${NC}"
    
    printf "  %-40s " "python3"
    check_command "python3" 2>/dev/null || echo -e "${RED}✗${NC}"
    
    printf "  %-40s " "git"
    check_command "git" 2>/dev/null || echo -e "${RED}✗${NC}"
}

# Main
main() {
    echo -e "${BLUE}"
    echo "  ╔═══════════════════════════════════════════════════════════╗"
    echo "  ║           MTS Router Development Status                   ║"
    echo "  ║           $(date +%Y-%m-%d\ %H:%M:%S)                        ║"
    echo "  ╚═══════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    check_dependencies
    check_core_router
    check_mobile_core
    check_mobile_backhaul
    check_olt_gpon
    check_enterprise
    check_residential
    check_linux
    check_api
    check_cicd
    
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}  Summary${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo ""
    echo -e "  Run ${YELLOW}./scripts/status.sh --json${NC} for machine-readable output"
    echo -e "  Run ${YELLOW}./scripts/run-agents.sh --parallel${NC} to start all agents"
    echo -e "  Run ${YELLOW}./scripts/test-all.sh${NC} to run all tests"
    echo ""
}

main
