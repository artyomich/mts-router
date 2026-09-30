# MTS Router — Security Audit Framework

## Overview

Comprehensive security audit framework for all MTS Router devices.

## Audit Categories

### 1. Kernel Security
- Kernel configuration hardening
- Kernel module signing
- Kernel exploit mitigation (KASLR, SMEP, SMAP)
- Kernel logging (klogd, dmesg restrictions)
- Kernel capabilities (capsh)

### 2. Network Security
- SSH configuration (ssh_config, sshd_config)
- Firewall rules (iptables/nftables)
- Network namespace isolation
- Network encryption (IPsec, TLS)
- Network access control

### 3. File System Security
- File permissions (755, 644, 600)
- File integrity monitoring (AIDE, Tripwire)
- Root file system read-only
- Secure boot (UEFI Secure Boot)
- TPM integration

### 4. Service Security
- Service hardening (systemd hardening)
- Service isolation (namespaces, cgroups)
- Service logging (journald)
- Service monitoring (health checks)
- Service auto-restart

### 5. Authentication & Authorization
- User management
- Password policies
- PAM configuration
- Certificate management
- API key management

### 6. Logging & Monitoring
- Syslog configuration
- Log rotation
- Log encryption
- SIEM integration
- Alert management

## Usage

```bash
# Run full security audit
./scripts/security-audit.sh

# Run specific audit
./scripts/security-audit.sh --audit kernel
./scripts/security-audit.sh --audit network
./scripts/security-audit.sh --audit filesystem
./scripts/security-audit.sh --audit service
./scripts/security-audit.sh --audit auth
./scripts/security-audit.sh --audit logging

# Run with specific device
./scripts/security-audit.sh --device mts-cr9000
```

## Results Format

```json
{
  "device": "mts-cr9000",
  "timestamp": "2024-01-01T00:00:00Z",
  "audit": {
    "kernel": {"score": 85, "issues": 3},
    "network": {"score": 90, "issues": 2},
    "filesystem": {"score": 95, "issues": 1},
    "service": {"score": 80, "issues": 5},
    "auth": {"score": 88, "issues": 3},
    "logging": {"score": 75, "issues": 6}
  },
  "overall_score": 85,
  "critical_issues": 0,
  "high_issues": 3,
  "medium_issues": 7,
  "low_issues": 12
}
```
