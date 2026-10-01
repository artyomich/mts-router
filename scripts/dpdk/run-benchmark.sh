#!/bin/bash
# run-benchmark.sh — DPDK benchmark тестирование для MTS Router устройств
#
# Usage:
#   ./run-benchmark.sh --device <device> [--packet-sizes <sizes>] [--duration <seconds>] [--test-pmd <path>] [--help]
#
# Запускает test-pmd с различными размерами пакетов и собирает результаты.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
RESULTS_DIR="${BUILD_DIR}/test-results/dpdk"

DEVICE=""
PACKET_SIZES="64 128 256 512 1024 1518"
DURATION=60
TEST_PMD=""
VERBOSE=false

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

Run DPDK benchmark tests for MTS Router devices.

Required options:
  --device <device>    Target device (cr9000, mc5000, mb3000, er1000)

Optional options:
  --packet-sizes <s>   Comma-separated packet sizes (default: 64,128,256,512,1024,1518)
  --duration <seconds> Test duration per packet size (default: 60)
  --test-pmd <path>    Custom test-pmd binary path
  --verbose            Enable verbose output
  --help               Show this help message

Devices:
  cr9000                 MTS-CR-9000 (Tofino 2, 100G)
  mc5000                 MTS-MC-5000 (ThunderX3, 100G)
  mb3000                 MTS-MB-3000 (S32G3, 10G)
  er1000                 MTS-ER-1000 (S32G3, 10G)

Examples:
  $(basename "$0") --device cr9000
  $(basename "$0") --device mb3000 --packet-sizes 64,128,1518 --duration 30
  $(basename "$0") --device mc5000 --test-pmd /opt/dpdk/bin/test-pmd
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --device)
            DEVICE="$2"
            shift 2
            ;;
        --packet-sizes)
            PACKET_SIZES=$(echo "$2" | tr ',' ' ')
            shift 2
            ;;
        --duration)
            DURATION="$2"
            shift 2
            ;;
        --test-pmd)
            TEST_PMD="$2"
            shift 2
            ;;
        --verbose|-v)
            VERBOSE=true
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

# Validate required parameters
if [ -z "${DEVICE}" ]; then
    log_error "Device is required (--device)"
    exit 1
fi

# Create results directory
mkdir -p "${RESULTS_DIR}"

# Device configurations
declare -A DEVICE_PORTS
declare -A DEVICE_CORES
declare -A DEVICE_EXPECTED_MPPS

case "${DEVICE}" in
    cr9000)
        DEVICE_PORTS="0xFF"
        DEVICE_CORES="0-7"
        DEVICE_EXPECTED_MPPS=50
        ;;
    mc5000)
        DEVICE_PORTS="0xFF"
        DEVICE_CORES="0-7"
        DEVICE_EXPECTED_MPPS=50
        ;;
    mb3000)
        DEVICE_PORTS="0x3F"
        DEVICE_CORES="0-3"
        DEVICE_EXPECTED_MPPS=10
        ;;
    er1000)
        DEVICE_PORTS="0xF"
        DEVICE_CORES="0-3"
        DEVICE_EXPECTED_MPPS=10
        ;;
    *)
        log_error "Unknown device: ${DEVICE}"
        exit 1
        ;;
esac

# Find test-pmd if not specified
if [ -z "${TEST_PMD}" ]; then
    for path in \
        /usr/lib/dpdk/app/dpdk-test-pmd \
        /usr/local/lib/dpdk/app/dpdk-test-pmd \
        /opt/dpdk/app/dpdk-test-pmd \
        /usr/bin/test-pmd \
        /usr/local/bin/test-pmd; do
        if [ -x "${path}" ]; then
            TEST_PMD="${path}"
            break
        fi
    done
fi

if [ -z "${TEST_PMD}" ] || [ ! -x "${TEST_PMD}" ]; then
    log_error "test-pmd not found"
    log_error "Install DPDK from https://doc.dpdk.org/guides/linux/guide_src/installation.html"
    exit 1
fi

log_info "test-pmd found at: ${TEST_PMD}"
log_info "Device: ${DEVICE}, Ports mask: ${DEVICE_PORTS}, Cores: ${DEVICE_CORES}"
log_info "Packet sizes: ${PACKET_SIZES}"
log_info "Duration per test: ${DURATION}s"
log_info "Expected performance: >= ${DEVICE_EXPECTED_MPPS} Mpps"
echo ""

# Results array
declare -A RESULTS_MPPS
declare -A RESULTS_LATENCY
declare -A RESULTS_STATUS

# Run test-pmd for each packet size
for pkt_size in ${PACKET_SIZES}; do
    log_info "Running test for packet size: ${pkt_size} bytes..."
    
    RESULTS_FILE="${RESULTS_DIR}/${DEVICE}-${pkt_size}b.log"
    START_TIME=$(date +%s)
    
    # Run test-pmd with single stream, forwarding MAC, one second per stream
    # Using -T for burst mode and --stat-period for statistics
    test_pmd_cmd="${TEST_PMD} \
        -l ${DEVICE_CORES} \
        -n 4 \
        --proc-type auto \
        -- -i \
        --portmask=${DEVICE_PORTS} \
        --rxdesc=1024 \
        --txdesc=1024 \
        --forward-stats=rx"
    
    if [ "${VERBOSE}" = true ]; then
        log_info "Command: ${test_pmd_cmd}"
    fi
    
    # Run test-pmd interactively with commands
    (
        echo "start"
        sleep 5
        echo "show port stats all"
        sleep ${DURATION}
        echo "show port stats all"
        echo "quit"
    ) | timeout $((DURATION + 30)) ${test_pmd_cmd} > "${RESULTS_FILE}" 2>&1 || true
    
    END_TIME=$(date +%s)
    ACTUAL_DURATION=$((END_TIME - START_TIME))
    
    # Parse results from test-pmd output
    # Look for statistics lines like:
    #   RX-packets: 12345678
    # TX-packets: 12345678
    RX_PACKETS=$(grep -i "RX-packets" "${RESULTS_FILE}" 2>/dev/null | tail -1 | grep -oP '[0-9,]+' | tr -d ',' || echo "0")
    TX_PACKETS=$(grep -i "TX-packets" "${RESULTS_FILE}" 2>/dev/null | tail -1 | grep -oP '[0-9,]+' | tr -d ',' || echo "0")
    
    # Calculate Mpps (millions of packets per second)
    if [ "${ACTUAL_DURATION}" -gt 0 ]; then
        MPPS=$(echo "scale=2; ${RX_PACKETS} / ${ACTUAL_DURATION} / 1000000" | bc 2>/dev/null || echo "0")
    else
        MPPS="0"
    fi
    
    RESULTS_MPPS[${pkt_size}]="${MPPS}"
    
    # Check against expected performance
    EXPECTED_INT=$(echo "${DEVICE_EXPECTED_MPPS}" | cut -d. -f1)
    ACTUAL_INT=$(echo "${MPPS}" | cut -d. -f1)
    
    if [ "${ACTUAL_INT:-0}" -ge "${EXPECTED_INT:-0}" ]; then
        RESULTS_STATUS[${pkt_size}]="passed"
        log_success "Packet size ${pkt_size}b: ${MPPS} Mpps (expected >= ${DEVICE_EXPECTED_MPPS})"
    else
        RESULTS_STATUS[${pkt_size}]="failed"
        log_error "Packet size ${pkt_size}b: ${MPPS} Mpps (expected >= ${DEVICE_EXPECTED_MPPS})"
    fi
    
    echo ""
done

# Generate results report
REPORT_FILE="${RESULTS_DIR}/${DEVICE}-dpdk-results.json"
cat > "${REPORT_FILE}" <<EOF
{
  "test_suite": "dpdk-benchmark",
  "device": "${DEVICE}",
  "test_pmd": "${TEST_PMD}",
  "cores": "${DEVICE_CORES}",
  "port_mask": "${DEVICE_PORTS}",
  "duration_per_test_seconds": ${DURATION},
  "expected_mpps": ${DEVICE_EXPECTED_MPPS},
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "results": {
EOF

first=true
for pkt_size in ${PACKET_SIZES}; do
    if [ "$first" = true ]; then
        first=false
    else
        echo "," >> "${REPORT_FILE}"
    fi
    echo -n "    \"${pkt_size}b\": {\"mpps\": ${RESULTS_MPPS[${pkt_size}]:-0}, \"status\": \"${RESULTS_STATUS[${pkt_size}]:-unknown}\"}" >> "${REPORT_FILE}"
done

cat >> "${REPORT_FILE}" <<EOF

  },
  "summary": {
    "total_tests": $(echo ${PACKET_SIZES} | wc -w),
    "passed": $(echo ${RESULTS_STATUS[@]} | grep -c "passed" || echo "0"),
    "failed": $(echo ${RESULTS_STATUS[@]} | grep -c "failed" || echo "0")
  }
}
EOF

log_success "Results saved to ${REPORT_FILE}"

# Print summary
echo ""
echo "========================================================"
echo "  DPDK Benchmark Results for ${DEVICE}"
echo "========================================================"
echo ""
printf "  %-12s %-12s %-10s\n" "Packet Size" "Mpps" "Status"
printf "  %-12s %-12s %-10s\n" "-----------" "----------" "------"

for pkt_size in ${PACKET_SIZES}; do
    printf "  %-12s %-12s %-10s\n" "${pkt_size}b" "${RESULTS_MPPS[${pkt_size}]:-N/A}" "${RESULTS_STATUS[${pkt_size}]:-N/A}"
done

echo ""
echo "  Expected: >= ${DEVICE_EXPECTED_MPPS} Mpps (all packet sizes)"
echo "  Report: ${REPORT_FILE}"
echo ""

# Exit with appropriate code
FAILED_COUNT=$(echo ${RESULTS_STATUS[@]} | grep -c "failed" || echo "0")
if [ "${FAILED_COUNT}" -gt 0 ]; then
    exit 1
fi
exit 0
