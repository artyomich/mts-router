#!/bin/bash
# setup.sh — Подготовка DPDK окружения для тестирования на целевом оборудовании
#
# Usage:
#   ./setup.sh [--device <device>] [--hugepages <num>] [--bind] [--help]
#
# Настраивает hugepages, binding NIC к vfio-pci и проверяет готовность DPDK.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
RESULTS_DIR="${BUILD_DIR}/test-results/dpdk"

DEVICE=""
HUGEPAGES=2048
DO_BIND=false

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
log_success() { echo -e "${GREEN}[OK]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Setup DPDK environment for MTS Router hardware testing.

Options:
  --device <device>      Target device (cr9000, mc5000, mb3000, er1000)
  --hugepages <num>      Number of 2MB hugepages (default: 2048)
  --bind                 Bind NICs to vfio-pci driver
  --help                 Show this help message

Devices:
  cr9000                 MTS-CR-9000 (Tofino 2, 100G ports)
  mc5000                 MTS-MC-5000 (ThunderX3, 100G ports)
  mb3000                 MTS-MB-3000 (S32G3, 10G ports)
  er1000                 MTS-ER-1000 (S32G3, 10G ports)

Examples:
  $(basename "$0") --device cr9000 --hugepages 4096
  $(basename "$0") --device mb3000 --bind
  $(basename "$0") --device er1000
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --device)
            DEVICE="$2"
            shift 2
            ;;
        --hugepages)
            HUGEPAGES="$2"
            shift 2
            ;;
        --bind)
            DO_BIND=true
            shift
            ;;
        --help|-h)
            usage
            ;;
        *)
            log_error "Unknown option: $1"
            usage
            ;;
    esac
done

# Create results directory
mkdir -p "${RESULTS_DIR}"

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    log_warn "Not running as root - some operations may fail"
    log_warn "Run with: sudo $(basename "$0") $*"
fi

# Step 1: Configure hugepages
log_info "Configuring hugepages: ${HUGEPAGES} pages (${HUGEPAGES} * 2MB = $((HUGEPAGES * 2 / 1024))GB)..."

HUGEPAGE_DIR="/sys/kernel/mm/hugepages/hugepages-2048kB"
if [ -d "${HUGEPAGE_DIR}" ]; then
    CURRENT_HUGEPAGES=$(cat "${HUGEPAGE_DIR}/nr_hugepages" 2>/dev/null || echo "0")
    log_info "Current hugepages: ${CURRENT_HUGEPAGES}"
    
    if [ "${CURRENT_HUGEPAGES}" -lt "${HUGEPAGES}" ]; then
        if echo "${HUGEPAGES}" > "${HUGEPAGE_DIR}/nr_hugepages" 2>/dev/null; then
            log_success "Hugepages set to ${HUGEPAGES}"
        else
            log_error "Failed to set hugepages - try: echo ${HUGEPAGES} > ${HUGEPAGE_DIR}/nr_hugepages"
        fi
    else
        log_success "Sufficient hugepages available: ${CURRENT_HUGEPAGES}"
    fi
else
    log_error "Hugepages directory not found: ${HUGEPAGE_DIR}"
    log_error "Check kernel config: CONFIG_HUGETLBFS=y CONFIG_HUGETLB_PAGE=y"
fi

# Step 2: Mount hugetlbfs
log_info "Checking hugetlbfs mount..."
if ! mount | grep -q hugetlbfs; then
    if [ -d "/dev/hugepages" ]; then
        sudo mount -t hugetlbfs hugetlbfs /dev/hugepages 2>/dev/null || true
        log_warn "Attempted to mount hugetlbfs"
    else
        sudo mkdir -p /dev/hugepages
        sudo mount -t hugetlbfs hugetlbfs /dev/hugepages 2>/dev/null || true
        log_warn "Attempted to mount hugetlbfs to /dev/hugepages"
    fi
else
    log_success "hugetlbfs is mounted"
fi

# Step 3: Check and bind NICs to vfio-pci
if [ "${DO_BIND}" = true ]; then
    log_info "Binding NICs to vfio-pci driver..."
    
    # Load required modules
    sudo modprobe vfio-pci 2>/dev/null || true
    sudo modprobe uio 2>/dev/null || true
    sudo modprobe uio_pci_generic 2>/dev/null || true
    
    # Check if dpdk-devbind.py is available
    DPDK_DEVBIND=""
    for path in /usr/share/dpdk/usertools/dpdk-devbind.py /usr/local/share/dpdk/usertools/dpdk-devbind.py; do
        if [ -f "${path}" ]; then
            DPDK_DEVBIND="${path}"
            break
        fi
    done
    
    if [ -n "${DPDK_DEVBIND}" ] && command -v "${DPDK_DEVBIND}" &>/dev/null; then
        # Show current NIC status
        log_info "Current NIC status:"
        "${DPDK_DEVBIND}" --status 2>/dev/null || log_warn "Could not get NIC status"
        
        # Bind Ethernet devices to vfio-pci
        log_info "Binding Ethernet devices to vfio-pci..."
        "${DPDK_DEVBIND}" --bind=vfio-pci net 2>/dev/null || log_warn "Could not bind NICs"
        
        log_success "NIC binding completed"
    else
        log_warn "dpdk-devbind.py not found at standard locations"
        log_warn "Install DPDK or set DPDK_DEVBIND environment variable"
    fi
fi

# Step 4: Check DPDK installation
log_info "Checking DPDK installation..."
DPDK_FOUND=false

if command -v dpdk-devbind.py &>/dev/null || [ -f "/usr/share/dpdk/usertools/dpdk-devbind.py" ]; then
    DPDK_FOUND=true
    log_success "DPDK tools found"
fi

if command -v test-pmd &>/dev/null || ls /usr/lib/dpdk/*test-pmd* &>/dev/null || ls /usr/local/lib/dpdk/*test-pmd* &>/dev/null; then
    DPDK_FOUND=true
    log_success "DPDK test-pmd found"
fi

if ! ${DPDK_FOUND}; then
    log_warn "DPDK not found - install from https://doc.dpdk.org/guides/linux/guide_src/installation.html"
    log_warn "Or use package manager: apt-get install dpdk"
fi

# Step 5: Generate test configuration
TEST_CONFIG="${RESULTS_DIR}/dpdk-test-config.json"
cat > "${TEST_CONFIG}" <<EOF
{
  "hugepages": {
    "requested": ${HUGEPAGES},
    "page_size_kb": 2048,
    "total_mb": $((HUGEPAGES * 2 / 1024))
  },
  "devices": {
    "cr9000": {
      "description": "MTS-CR-9000 Core Router",
      "asic": "Tofino 2",
      "ports": "8x 100G",
      "expected_perf_mpps": 50,
      "test_pmd_args": "-l 0-7 -n 4 -- -i --portmask=0xFF"
    },
    "mc5000": {
      "description": "MTS-MC-5000 Mobile Core",
      "asic": "ThunderX3",
      "ports": "8x 100G",
      "expected_perf_mpps": 50,
      "test_pmd_args": "-l 0-7 -n 4 -- -i --portmask=0xFF"
    },
    "mb3000": {
      "description": "MTS-MB-3000 Mobile Backhaul",
      "asic": "S32G3",
      "ports": "8x 10G",
      "expected_perf_mpps": 10,
      "test_pmd_args": "-l 0-3 -n 4 -- -i --portmask=0x3F"
    },
    "er1000": {
      "description": "MTS-ER-1000 Enterprise Router",
      "asic": "S32G3",
      "ports": "4x 10G",
      "expected_perf_mpps": 10,
      "test_pmd_args": "-l 0-3 -n 4 -- -i --portmask=0xF"
    }
  }
}
EOF
log_success "Test configuration created at ${TEST_CONFIG}"

# Step 6: Display summary
echo ""
echo "========================================================"
echo "  DPDK Environment Setup Summary"
echo "========================================================"
echo ""
echo "  Hugepages:     ${HUGEPAGES} (2MB pages = $((HUGEPAGES * 2 / 1024))GB)"
echo "  hugetlbfs:     $(mount | grep -c hugetlbfs || echo '0') mounted"
echo "  DPDK found:    ${DPDK_FOUND}"
echo "  NIC binding:   ${DO_BIND}"
echo ""

if [ -n "${DEVICE}" ]; then
    log_info "Target device: ${DEVICE}"
    case "${DEVICE}" in
        cr9000|mc5000)
            log_info "Expected: 100G ports, >= 50 Mpps forwarding"
            ;;
        mb3000|er1000)
            log_info "Expected: 10G ports, >= 10 Mpps forwarding"
            ;;
    esac
fi

echo ""
echo "Next steps:"
echo "  1. Verify NICs are bound to vfio-pci: dpdk-devbind.py --status"
echo "  2. Run benchmark: scripts/dpdk/run-benchmark.sh --device <device>"
echo ""
