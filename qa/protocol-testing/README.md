# MTS Router — Protocol Testing Framework

## Overview

Comprehensive protocol testing framework for all MTS Router devices.

## Test Suites

### BGP Testing
- BGP peer establishment
- Route advertisement/withdrawal
- BGP session reset
- Route dampening
- MP-BGP (VPNv4, VPNv6)
- BGP graceful restart

### MPLS Testing
- LSP establishment (RSVP-TE, LDP, BGP-LS)
- PW (Pseudowire) testing
- MPLS-TP OAM
- MPLS forwarding verification
- ECMP for MPLS

### SRv6 Testing
- SR Policy establishment
- SID list verification
- SRH header validation
- SRv6 transport verification
- SRv6 OAM

### GTP-U Testing (Mobile Core)
- GTP tunnel establishment
- Packet forwarding
- GTP-U error handling
- TON (Tunnel Endpoint) verification

### PFCP Testing (5GC UPF)
- Session establishment
- PDR/URR/SFER validation
- FAR (Forwarding Action Rule) verification
- QER (QoS Enforcement Rule) testing

## Usage

```bash
# Run all protocol tests
./scripts/test-all-protocols.sh

# Run specific protocol
./scripts/test-all-protocols.sh --protocol bgp
./scripts/test-all-protocols.sh --protocol mpls
./scripts/test-all-protocols.sh --protocol srv6
./scripts/test-all-protocols.sh --protocol gtp
./scripts/test-all-protocols.sh --protocol pfcp

# Run with specific device
./scripts/test-all-protocols.sh --device mts-cr9000
./scripts/test-all-protocols.sh --device mts-mc5000
./scripts/test-all-protocols.sh --device mts-mb3000
```

## Test Results

```json
{
  "timestamp": "2024-01-01T00:00:00Z",
  "device": "mts-cr9000",
  "protocols": {
    "bgp": {"passed": 42, "failed": 0, "skipped": 2},
    "mpls": {"passed": 38, "failed": 1, "skipped": 0},
    "srv6": {"passed": 25, "failed": 0, "skipped": 3}
  }
}
```
