#!/bin/bash
# deploy.sh — Деплой MTS Router образов
#
# Usage:
#   ./deploy.sh <device> [--target <host>] [--verify]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info()  { echo -e "${GREEN}[deploy] $*${NC}"; }
log_warn()  { echo -e "${YELLOW}[deploy] $*${NC}"; }
log_error() { echo -e "${RED}[deploy] $*${NC}" >&2; }

# Available devices
DEVICES=(
    "core-router"
    "mobile-core"
    "mobile-backhaul"
    "olt-gpon"
    "enterprise"
    "residential"
)

# Parse arguments
DEVICE=""
TARGET=""
VERIFY=false

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <device> [--target <host>] [--verify]"
    echo ""
    echo "Available devices:"
    for d in "${DEVICES[@]}"; do
        echo "  - ${d}"
    done
    exit 1
fi

DEVICE="$1"
shift

# Validate device
valid=false
for d in "${DEVICES[@]}"; do
    if [[ "$d" == "$DEVICE" ]]; then
        valid=true
        break
    fi
done

if [[ "$valid" != "true" ]]; then
    log_error "Unknown device: ${DEVICE}"
    exit 1
fi

# Parse options
while [[ $# -gt 0 ]]; do
    case $1 in
        --target)
            TARGET="$2"
            shift 2
            ;;
        --verify)
            VERIFY=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 <device> [--target <host>] [--verify]"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Deploy functions
deploy_core_router() {
    log_info "Deploying Core Router (MTS-CR-9000)..."
    
    if [[ -n "$TARGET" ]]; then
        log_info "  Target