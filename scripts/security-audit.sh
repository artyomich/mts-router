#!/bin/bash
# security-audit.sh — Security audit for MTS Router devices
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
RESULTS_DIR="${PROJECT_DIR}/build/security-results"
VERBOSE=false
AUDIT=""
DEVICE=""

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
PASS=0; FAIL=0; WARN=0; TOTAL=0

mkdir -p "${RESULTS_DIR}"

while [[ $# -gt 0 ]]; do
    case $1 in
        --audit) AUDIT="$2"; shift 2 ;;
        --device) DEVICE="$2"; shift 2 ;;
        --verbose|-v) VERBOSE=true; shift ;;
        *) shift ;;
    esac
done

log_pass() { PASS=$((PASS+1)); TOTAL=$((TOTAL+1)); echo -e "${GREEN}[PASS]${NC} $1"; }
log_fail() { FAIL=$((FAIL+1)); TOTAL=$((TOTAL+1)); echo -e "${RED}[FAIL]${NC} $1"; }
log_warn() { WARN=$((WARN+1)); TOTAL=$((TOTAL+1)); echo -e "${YELLOW}[WARN]${NC} $1"; }
log_info() { echo -e "${YELLOW}[INFO]${NC} $1"; }
log_section() { echo ""; echo -e "${GREEN}=== $1 ===${NC}"; }

# ==================== Kernel Security ====================

audit_kernel() {
    log_section "Kernel Security Audit"
    
    log_info "Checking kernel configuration..."
    
    # Check kernel hardening options
    local kernel_opts=(
        "CONFIG_SECURITY=y"
        "CONFIG_SECURITYFS=y"
        "CONFIG_SECURITY_NETWORK=y"
        "CONFIG_SECURITY_PATH=y"
        "CONFIG_SECURITY_SELINUX=y"
        "CONFIG_DEFAULT_SECURITY_SELINUX=y"
        "CONFIG_KALLSYMS=y"
        "CONFIG_STRICT_KERNEL_RWX=y"
        "CONFIG_STRICT_MODULE_RWX=y"
        "CONFIG_DEBUG_WX=y"
        "CONFIG_HIBERNATION=y"
        "CONFIG_RANDOMIZE_BASE=y"
        "CONFIG_STACKPROTECTOR=y"
        "CONFIG_STACKPROTECTOR_STRONG=y"
        "CONFIG_FORTIFY_SOURCE=y"
    )
    
    for opt in "${kernel_opts[@]}"; do
        if [[ -f "/proc/config.gz" ]]; then
            if zcat /proc/config.gz 2>/dev/null | grep -q "^${opt}=y$"; then
                log_pass "Kernel: ${opt}"
            else
                log_warn "Kernel: ${opt} not enabled"
            fi
        else
            log_warn "Kernel: Cannot read /proc/config.gz"
        fi
    done
    
    log_info "Checking kernel parameters..."
    
    # Check sysctl security parameters
    local sysctl_params=(
        "net.ipv4.conf.all.accept_redirects:0"
        "net.ipv6.conf.all.accept_redirects:0"
        "net.ipv4.conf.all.send_redirects:0"
        "net.ipv4.conf.all.accept_source_route:0"
        "net.ipv6.conf.all.accept_source_route:0"
        "net.ipv4.conf.all.log_martians:1"
        "net.ipv6.conf.all.log_martians:1"
        "net.ipv4.icmp_echo_ignore_broadcasts:1"
        "net.ipv4.icmp_ignore_bogus_error_responses:1"
        "kernel.randomize_va_space:2"
        "kernel.dmesg_restrict:1"
        "kernel.kptr_restrict:2"
        "kernel.yama.ptrace_scope:1"
    )
    
    for param in "${sysctl_params[@]}"; do
        local key="${param%%:*}"
        local expected="${param##*:}"
        local current=""
        
        if [[ -f "/proc/sys/${key//.//}" ]]; then
            current=$(cat "/proc/sys/${key//.//}" 2>/dev/null || echo "N/A")
            if [[ "$current" == "$expected" ]]; then
                log_pass "Sysctl: ${key} = ${current}"
            else
                log_warn "Sysctl: ${key} = ${current} (expected: ${expected})"
            fi
        else
            log_warn "Sysctl: ${key} not available"
        fi
    done
}

# ==================== Network Security ====================

audit_network() {
    log_section "Network Security Audit"
    
    log_info "Checking SSH configuration..."
    
    # SSH security checks
    if [[ -f "/etc/ssh/sshd_config" ]]; then
        local ssh_params=(
            "PermitRootLogin:no"
            "PasswordAuthentication:no"
            "PubkeyAuthentication:yes"
            "Protocol:2"
            "X11Forwarding:no"
            "MaxAuthTries:3"
            "ClientAliveInterval:300"
            "ClientAliveCountMax:2"
        )
        
        for param in "${ssh_params[@]}"; do
            local key="${param%%:*}"
            local expected="${param##*:}"
            local current=""
            
            current=$(grep -i "^${key}" /etc/ssh/sshd_config 2>/dev/null | tail -1 | cut -d' ' -f2 || echo "not set")
            if [[ "$current" == "$expected" ]]; then
                log_pass "SSH: ${key} = ${current}"
            else
                log_warn "SSH: ${key} = ${current} (expected: ${expected})"
            fi
        done
    else
        log_warn "SSH: sshd_config not found"
    fi
    
    log_info "Checking firewall rules..."
    
    # Firewall checks
    if command -v iptables &>/dev/null; then
        local rules_count
        rules_count=$(iptables -L -n 2>/dev/null | grep -c "^[A-Z]" || echo "0")
        if [[ "$rules_count" -gt 0 ]]; then
            log_pass "Firewall: ${rules_count} iptables rules configured"
        else
            log_warn "Firewall: No iptables rules configured"
        fi
    else
        log_warn "Firewall: iptables not available"
    fi
    
    if command -v nft &>/dev/null; then
        local nft_rules
        nft_rules=$(nft list ruleset 2>/dev/null | wc -l || echo "0")
        if [[ "$nft_rules" -gt 0 ]]; then
            log_pass "NFT: ${nft_rules} rules configured"
        else
            log_warn "NFT: No nft rules configured"
        fi
    fi
}

# ==================== Filesystem Security ====================

audit_filesystem() {
    log_section "Filesystem Security Audit"
    
    log_info "Checking file permissions..."
    
    # Check critical file permissions
    local files=(
        "/etc/shadow:640"
        "/etc/passwd:644"
        "/etc/ssh/sshd_config:600"
        "/etc/ssh/ssh_host_rsa_key:600"
        "/etc/ssh/ssh_host_ecdsa_key:600"
        "/boot:755"
        "/etc:755"
    )
    
    for entry in "${files[@]}"; do
        local path="${entry%%:*}"
        local expected="${entry##*:}"
        
        if [[ -e "$path" ]]; then
            local perms
            perms=$(stat -c "%a" "$path" 2>/dev/null || echo "N/A")
            if [[ "$perms" == "$expected" ]]; then
                log_pass "File: ${path} permissions = ${perms}"
            else
                log_warn "File: ${path} permissions = ${perms} (expected: ${expected})"
            fi
        else
            log_warn "File: ${path} not found"
        fi
    done
    
    log_info "Checking SUID/SGID files..."
    
    # Check for SUID/SGID files
    local suid_count
    suid_count=$(find /usr /usr/local -perm /6000 -type f 2>/dev/null | wc -l || echo "0")
    if [[ "$suid_count" -lt 20 ]]; then
        log_pass "SUID/SGID: ${suid_count} files found (acceptable)"
    else
        log_warn "SUID/SGID: ${suid_count} files found (too many)"
    fi
}

# ==================== Service Security ====================

audit_service() {
    log_section "Service Security Audit"
    
    log_info "Checking running services..."
    
    # Check for unnecessary services
    local services=("telnet" "ftp" "rsh" "rlogin" "rdate")
    for svc in "${services[@]}"; do
        if command -v "$svc" &>/dev/null; then
            log_warn "Service: ${svc} is installed"
        else
            log_pass "Service: ${svc} not installed"
        fi
    done
    
    log_info "Checking systemd hardening..."
    
    # Check systemd service hardening
    if command -v systemctl &>/dev/null; then
        local units
        units=$(systemctl list-units --type=service --state=running 2>/dev/null | wc -l || echo "0")
        log_info "Running systemd services: ${units}"
    fi
}

# ==================== Authentication ====================

audit_auth() {
    log_section "Authentication & Authorization Audit"
    
    log_info "Checking user accounts..."
    
    # Check for root accounts
    local root_login
    root_login=$(grep "^root:" /etc/shadow 2>/dev/null | cut -d: -f2 || echo "N/A")
    if [[ "$root_login" != "!"* ]] && [[ "$root_login" != "*" ]]; then
        log_warn "Auth: Root account is enabled"
    else
        log_pass "Auth: Root account is locked"
    fi
    
    # Check password policy
    if [[ -f "/etc/login.defs" ]]; then
        local pass_max
        pass_max=$(grep "^PASS_MAX_DAYS" /etc/login.defs 2>/dev/null | awk '{print $2}' || echo "N/A")
        if [[ "$pass_max" != "N/A" ]] && [[ "$pass_max" -le 90 ]]; then
            log_pass "Password: Max days = ${pass_max}"
        else
            log_warn "Password: Max days = ${pass_max} (should be <= 90)"
        fi
    fi
    
    log_info "Checking PAM configuration..."
    
    if [[ -f "/etc/pam.d/common-auth" ]]; then
        log_pass "PAM: common-auth exists"
    else
        log_warn "PAM: common-auth not found"
    fi
}

# ==================== Logging ====================

audit_logging() {
    log_section "Logging & Monitoring Audit"
    
    log_info "Checking syslog configuration..."
    
    if [[ -f "/etc/rsyslog.conf" ]]; then
        log_pass "Syslog: rsyslog.conf exists"
    elif [[ -f "/etc/syslog.conf" ]]; then
        log_pass "Syslog: syslog.conf exists"
    else
        log_warn "Syslog: No syslog configuration found"
    fi
    
    log_info "Checking log rotation..."
    
    if [[ -f "/etc/logrotate.conf" ]]; then
        log_pass "Logrotate: logrotate.conf exists"
    else
        log_warn "Logrotate: logrotate.conf not found"
    fi
    
    log_info "Checking journald configuration..."
    
    if [[ -f "/etc/systemd/journald.conf" ]]; then
        local max_use
        max_use=$(grep "^SystemMaxUse" /etc/systemd/journald.conf 2>/dev/null | awk '{print $2}' || echo "N/A")
        if [[ "$max_use" != "N/A" ]]; then
            log_pass "Journald: SystemMaxUse = ${max_use}"
        fi
    fi
}

# ==================== Summary ====================

print_summary() {
    echo ""
    echo "========================================="
    echo "  Security Audit Summary"
    echo "========================================="
    echo -e "  Total: ${TOTAL}"
    echo -e "  ${GREEN}Passed: ${PASS}${NC}"
    echo -e "  ${RED}Failed: ${FAIL}${NC}"
    echo -e "  ${YELLOW}Warnings: ${WARN}${NC}"
    echo "========================================="
    
    if [ $FAIL -eq 0 ]; then
        echo -e "${GREEN}Security audit passed!${NC}"
    else
        echo -e "${RED}Security audit found ${FAIL} critical issues!${NC}"
    fi
}

# ==================== Main ====================

echo "=== MTS Router Security Audit ==="
echo "Start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"

case "${AUDIT:-all}" in
    all)
        audit_kernel
        audit_network
        audit_filesystem
        audit_service
        audit_auth
        audit_logging
        ;;
    kernel) audit_kernel ;;
    network) audit_network ;;
    filesystem) audit_filesystem ;;
    service) audit_service ;;
    auth) audit_auth ;;
    logging) audit_logging ;;
    *) echo "Unknown audit: ${AUDIT}"; exit 1 ;;
esac

print_summary
echo "End: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
