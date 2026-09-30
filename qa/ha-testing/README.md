# MTS Router — HA (High Availability) Testing Framework

## Overview

Comprehensive HA testing for all MTS Router devices supporting redundancy.

## Test Categories

### VRRP Testing
- VRRP master/backup election
- VRRP preemption
- VRRP priority changes
- VRRP tracking
- VRRP failover time

### BFD Testing
- BFD session establishment
- BFD fast detection
- BFD under loss
- BFD with different modes (async, demand)
- BFD failover

### LACP Testing
- LACP link aggregation
- LACP failover
- LACP load balancing
- LACP hash algorithm verification

### HSRP Testing
- HSRP active/standby election
- HSRP preemption
- HSRP tracking
- HSRP failover

### NSR/NSSA Testing (Core Router)
- NSR (Non-Stop Routing)
- NSSA (Non-Stop Forwarding)
- Control plane failover
- Data plane continuity

### SSO Testing (Mobile Core)
- SSO (Stateful Switchover)
- GRP (Graceful Redundancy Protocol)
- State synchronization
- Failover with state

## Usage

```bash
# Run all HA tests
./scripts/test-ha.sh

# Run specific test
./scripts/test-ha.sh --test vrrp
./scripts/test-ha.sh --test bfd
./scripts/test-ha.sh --test lacp
./scripts/test-ha.sh --test nsr
./scripts/test-ha.sh --test sso

# Run with specific device
./scripts/test-ha.sh --device mts-cr9000
./scripts/test-ha.sh --device mts-mc5000
```

## Results Format

```json
{
  "device": "mts-cr9000",
  "timestamp": "2024-01-01T00:00:00Z",
  "tests": {
    "vrrp": {
      "failover_time_ms": 1500,
      "max_failover_ms": 3000,
      "passed": true
    },
    "bfd": {
      "detection_time_ms": 300,
      "max_detection_ms": 900,
      "passed": true
    }
  }
}
```
