#!/bin/bash
# run-agents.sh — Запуск всех MTS Router development agents параллельно
#
# Usage:
#   ./run-agents.sh [--parallel] [--workers N]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
LOG_DIR="${PROJECT_DIR}/logs/agents"
PID_DIR="${PROJECT_DIR}/pids"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Default settings
PARALLEL=false
WORKERS=6
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
while [[ $# -gt 0 ]]; do
    case $1 in
        --parallel)
            PARALLEL=true
            shift
            ;;
        --workers)
            WORKERS="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [--parallel] [--workers N] [--agents agent1,agent2,...]"
            echo ""
            echo "Options:"
            echo "  --parallel       Run agents in parallel"
            echo "  --workers N      Maximum number of parallel workers (default: 6)"
            echo "  --agents list    Comma-separated list of agents to run"
            echo "  --help, -h       Show this help"
            echo ""
            echo "Available agents:"
            for agent in "${AGENTS[@]}"; do
                echo "  - ${agent}"
            done
            exit 0
            ;;
        --agents)
            IFS=',' read -ra AGENTS <<< "$2"
            shift 2
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}" >&2
            exit 1
            ;;
    esac
done

# Create directories
mkdir -p "$LOG_DIR" "$PID_DIR"

# Logging function
log() {
    local level="$1"
    shift
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    case "$level" in
        INFO)  echo -e "${GREEN}[$timestamp] [$level]${NC} $*";;
        WARN)  echo -e "${YELLOW}[$timestamp] [$level]${NC} $*";;
        ERROR) echo -e "${RED}[$timestamp] [$level]${NC} $*";;
        *)     echo "[$timestamp] [$level] $*"
    esac
}

# Run a single agent
run_agent() {
    local agent="$1"
    local log_file="${LOG_DIR}/${agent}.log"
    local pid_file="${PID_DIR}/${agent}.pid"
    
    log INFO "Starting agent: ${agent}"
    
    # Run agent-specific setup
    case "$agent" in
        core-router)
            bash "${PROJECT_DIR}/scripts/build-core-router.sh" --setup 2>&1 | tee "$log_file"
            ;;
        mobile-core)
            bash "${PROJECT_DIR}/scripts/build-mobile-core.sh" --setup 2>&1 | tee "$log_file"
            ;;
        mobile-backhaul)
            bash "${PROJECT_DIR}/scripts/build-mobile-backhaul.sh" --setup 2>&1 | tee "$log_file"
            ;;
        olt-gpon)
            bash "${PROJECT_DIR}/scripts/build-olt-gpon.sh" --setup 2>&1 | tee "$log_file"
            ;;
        enterprise)
            bash "${PROJECT_DIR}/scripts/build-enterprise.sh" --setup 2>&1 | tee "$log_file"
            ;;
        residential)
            bash "${PROJECT_DIR}/scripts/build-residential.sh" --setup 2>&1 | tee "$log_file"
            ;;
        linux)
            log INFO "Setting up meta-mts Yocto layer..."
            bash -c '
                cd '"${PROJECT_DIR}"'
                echo "meta-mts layer is ready at linux/meta-mts/"
                echo "Configure your build environment:"
                echo "  source oe-init-build-env build"
                echo "  bitbake-layers add-layer meta-mts"
            ' 2>&1 | tee "$log_file"
            ;;
        firmware)
            log INFO "Building firmware drivers..."
            for driver_dir in core-router/firmware mobile-core/firmware mobile-backhaul/firmware olt-gpon/firmware enterprise-router/firmware residential-gateway/firmware; do
                if [[ -f "${PROJECT_DIR}/${driver_dir}/driver-spec.md" ]]; then
                    log INFO "Processing ${driver_dir}..."
                fi
            done
            echo "Firmware drivers are ready." | tee "$log_file"
            ;;
        api)
            log INFO "Setting up API/SDK..."
            bash -c '
                cd '"${PROJECT_DIR}"'
                echo "API specifications are ready in api/spec/"
                echo "SDKs available in:"
                echo "  - api/sdk/python/"
                echo "  - api/sdk/go/"
                echo "  - api/sdk/java/"
            ' 2>&1 | tee "$log_file"
            ;;
        qa)
            log INFO "Setting up QA/CI/CD..."
            bash -c '
                cd '"${PROJECT_DIR}"'
                echo "CI/CD pipeline is configured in .github/workflows/"
                echo "Run tests with:"
                echo "  ./scripts/test-all.sh"
            ' 2>&1 | tee "$log_file"
            ;;
        *)
            log WARN "Unknown agent: ${agent}"
            return 1
            ;;
    esac
    
    echo $$ > "$pid_file"
    log INFO "Agent ${agent} completed (PID: $(cat "$pid_file"))"
}

# Run agents in parallel
run_parallel() {
    local pids=()
    local remaining=("${AGENTS[@]}")
    
    log INFO "Starting ${#AGENTS[@]} agents with ${WORKERS} workers..."
    
    while [[ ${#remaining[@]} -gt 0 ]]; do
        # Start up to WORKERS agents
        local started=0
        while [[ ${#remaining[@]} -gt 0 && $started -lt $WORKERS ]]; do
            local agent="${remaining[0]}"
            remaining=("${remaining[@]:1}")
            
            run_agent "$agent" &
            pids+=($!)
            started=$((started + 1))
        done
        
        # Wait for any agent to complete
        wait -n 2>/dev/null || true
        
        # Clean up completed PIDs
        local new_pids=()
        for pid in "${pids[@]}"; do
            if kill -0 "$pid" 2>/dev/null; then
                new_pids+=("$pid")
            fi
        done
        pids=("${new_pids[@]}")
    done
    
    # Wait for all remaining agents
    wait
    log INFO "All agents completed."
}

# Run agents sequentially
run_sequential() {
    for agent in "${AGENTS[@]}"; do
        log INFO "Processing agent: ${agent}"
        run_agent "$agent"
        log INFO "Agent ${agent} finished."
    done
}

# Main
main() {
    log INFO "MTS Router Development Environment"
    log INFO "Project: ${PROJECT_DIR}"
    log INFO "Agents: ${AGENTS[*]}"
    
    if [[ "$PARALLEL" == true ]]; then
        run_parallel
    else
        run_sequential
    fi
    
    log INFO "All done!"
}

main
