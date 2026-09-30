# MTS Router — Performance Testing Framework

## Overview

Comprehensive performance testing for all MTS Router devices.

## Test Categories

### Throughput Testing
- Wire-speed forwarding verification
- Maximum throughput measurement
- Per-port throughput
- Aggregate throughput

### Latency Testing
- Packet latency (one-way, round-trip)
- Jitter measurement
- Latency under load
- Microburst handling

### Packet Rate Testing
- PPS (packets per second) measurement
- Minimum packet size (64B)
- Maximum packet size (9216B)
- Mixed packet sizes

### Scalability Testing
- Maximum routes
- Maximum BGP peers
- Maximum MPLS LSPs
- Maximum VLANs
- Maximum QoS policies

### Resource Utilization
- CPU usage under load
- Memory consumption
- TCAM utilization
- Buffer utilization

## Usage

```bash
# Run all performance tests
./scripts/test-performance.sh

# Run specific test
./scripts/test-performance.sh --test throughput
./scripts/test-performance.sh --test latency
./scripts/test-performance.sh --test packet-rate
./scripts/test-performance.sh --test scalability

# Run with specific device
./scripts/test-performance.sh --device mts-cr9000
```

## Results Format

```json
{
  "device": "mts-cr9000",
  "timestamp": "2024-01-01T00:00:00Z",
  "tests": {
    "throughput": {
      "64B": {"pps": 14880952, "gbps": 6.3},
      "1518B": {"pps": 14880952, "gbps": 394.0}
    },
    "latency": {
      "avg_us": 0.5,
      "max_us": 2.1,
      "min_us": 0.3
    }
  }
}
```
