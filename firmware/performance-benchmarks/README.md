# Firmware Performance Benchmarks

## Overview

Performance benchmarking suite for MTS Router firmware drivers.
Tests throughput, latency, packet processing, and resource utilization.

## Benchmark Suite

| Benchmark | Description | Metric | Target |
|-----------|-------------|--------|--------|
| tofino2-throughput | Tofino 2 packet forwarding | Mpps | 120 Mpps |
| tofino2-latency | Tofino 2 processing latency | ns | < 500ns |
| thunderx3-network | ThunderX3 network throughput | Gbps | 200 Gbps |
| thunderx3-crypto | ThunderX3 crypto throughput | Gbps | 50 Gbps |
| s32g3-forwarding | S32G3 MPLS-TP forwarding | Mpps | 25 Mpps |
| s32g3-ptp | S32G3 PTP precision | ns | < 100ns |
| rtl960x-gpon | RTL960x GPON throughput | Gbps | 2.5 Gbps |
| mt7981-wifi | MT7981 WiFi throughput | Mbps | 1200 Mbps |
| mt7981-ethernet | MT7981 Ethernet throughput | Gbps | 4 Gbps |
| tomtom-forwarding | TomTom ASIC forwarding | Mpps | 30 Mpps |

## Running Benchmarks

```bash
# Run all benchmarks
./run_benchmarks.sh --all

# Run specific benchmark
./run_benchmarks.sh --bench tofino2-throughput

# Run with custom parameters
./run_benchmarks.sh --all --packets 64 --flows 1000 --duration 60

# Generate report
./run_benchmarks.sh --all --report /tmp/benchmark-report.html
```

## Benchmark Configuration

### tofino2-throughput

```yaml
benchmark: tofino2-throughput
description: "Tofino 2 packet forwarding throughput"
params:
  packet_sizes: [64, 128, 256, 512, 1024, 1280, 1518]
  flows: 1000
  duration: 60
  warmup: 10
metrics:
  - name: throughput_mpps
    unit: Mpps
    target: 120
  - name: cpu_usage
    unit: percent
    target: 60
  - name: memory_mb
    unit: MB
    target: 512
```

### tofino2-latency

```yaml
benchmark: tofino2-latency
description: "Tofino 2 processing latency"
params:
  packet_sizes: [64, 128, 256, 512]
  samples: 10000
  distribution: uniform
metrics:
  - name: latency_p50
    unit: ns
    target: 200
  - name: latency_p99
    unit: ns
    target: 500
  - name: latency_p999
    unit: ns
    target: 1000
```

### thunderx3-network

```yaml
benchmark: thunderx3-network
description: "ThunderX3 network throughput"
params:
  packet_sizes: [64, 128, 256, 512, 1024, 1280, 9022]
  flows: 100
  duration: 60
  offload: true
metrics:
  - name: throughput_gbps
    unit: Gbps
    target: 200
  - name: packets_per_flow
    unit: Mpps
    target: 10
```

### thunderx3-crypto

```yaml
benchmark: thunderx3-crypto
description: "ThunderX3 crypto acceleration throughput"
params:
  algorithms: [AES-GCM, AES-XTS, ChaCha20, SHA256]
  key_sizes: [128, 192, 256]
  packet_sizes: [64, 128, 256, 512, 1024]
  flows: 50
  duration: 60
metrics:
  - name: throughput_gbps
    unit: Gbps
    target: 50
  - name: latency_us
    unit: μs
    target: 10
```

### s32g3-forwarding

```yaml
benchmark: s32g3-forwarding
description: "S32G3 MPLS-TP forwarding"
params:
  packet_sizes: [64, 128, 256, 512, 1024]
  mpls_labels: 1000
  flows: 500
  duration: 60
metrics:
  - name: throughput_mpps
    unit: Mpps
    target: 25
  - name: mpls_lookup_us
    unit: μs
    target: 1
```

### s32g3-ptp

```yaml
benchmark: s32g3-ptp
description: "S32G3 PTP grandmaster precision"
params:
  domain: 0
  sync_interval_ms: 10
  samples: 10000
  filter: median
metrics:
  - name: offset_ns
    unit: ns
    target: 100
  - name: jitter_ns
    unit: ns
    target: 50
  - name: frequency_ppb
    unit: ppb
    target: 0.5
```

### rtl960x-gpon

```yaml
benchmark: rtl960x-gpon
description: "RTL960x GPON throughput"
params:
  upstream_rate: 1.244
  downstream_rate: 2.488
  onu_count: 64
  packet_sizes: [64, 128, 256, 512, 1024, 1518]
  duration: 60
metrics:
  - name: upstream_mbps
    unit: Mbps
    target: 1244
  - name: downstream_mbps
    unit: Mbps
    target: 2488
  - name: onu_latency_ms
    unit: ms
    target: 125
```

### mt7981-wifi

```yaml
benchmark: mt7981-wifi
description: "MT7981 WiFi 6 throughput"
params:
  bands: [2.4GHz, 5GHz]
  mimo: [2x2, 4x4]
  htmode: [HT40, VHT80]
  clients: 32
  packet_sizes: [64, 128, 256, 512, 1024, 1460]
  duration: 60
metrics:
  - name: throughput_mbps_2g
    unit: Mbps
    target: 600
  - name: throughput_mbps_5g
    unit: Mbps
    target: 1200
  - name: latency_ms
    unit: ms
    target: 5
```

### mt7981-ethernet

```yaml
benchmark: mt7981-ethernet
description: "MT7981 Ethernet throughput"
params:
  ports: 4
  speeds: [100, 1000]
  duplex: full
  packet_sizes: [64, 128, 256, 512, 1024, 1518]
  flows: 100
  duration: 60
metrics:
  - name: throughput_gbps
    unit: Gbps
    target: 4
  - name: latency_us
    unit: μs
    target: 10
```

### tomtom-forwarding

```yaml
benchmark: tomtom-forwarding
description: "TomTom ASIC forwarding performance"
params:
  packet_sizes: [64, 128, 256, 512, 1024]
  routes: 10000
  flows: 5000
  duration: 60
  protocols: [IPv4, IPv6, MPLS]
metrics:
  - name: throughput_mpps
    unit: Mpps
    target: 30
  - name: route_lookup_us
    unit: μs
    target: 0.5
```

## Benchmark Infrastructure

### Hardware Requirements

| Benchmark | Minimum Hardware | Recommended |
|-----------|-----------------|-------------|
| tofino2-* | Tofino 2 dev kit | 2x Tofino 2 boards |
| thunderx3-* | ThunderX3 server | 2x ThunderX3 servers |
| s32g3-* | S32G3 dev board | S32G3 RDB board |
| rtl960x-* | RTL960x dev kit | OLT test platform |
| mt7981-* | MT7981 dev board | MTS-RG-500 prototype |
| tomtom-* | TomTom dev kit | Enterprise router |

### Software Requirements

```bash
# Required tools
apt-get install -y iperf3 qperf hping3 nuttcp

# DPDK (for high-performance testing)
git clone https://github.com/dpdk/dpdk.git
cd dpdk && meson setup build && ninja -C build

# PTP testing
apt-get install -y ptp4l phc2sys fuser

# WiFi testing
apt-get install -y iw hostapd madwifi-tools
```

## Results Format

```json
{
  "benchmark": "tofino2-throughput",
  "timestamp": "2026-10-01T10:00:00Z",
  "hardware": {
    "platform": "MTS-CR-9000",
    "firmware_version": "1.0.0",
    "kernel_version": "5.15.0"
  },
  "results": [
    {
      "packet_size": 64,
      "throughput_mpps": 85.5,
      "cpu_usage_percent": 72.3,
      "memory_mb": 256,
      "latency_p50_ns": 250,
      "latency_p99_ns": 500
    },
    {
      "packet_size": 1518,
      "throughput_mpps": 120.2,
      "cpu_usage_percent": 45.1,
      "memory_mb": 128,
      "latency_p50_ns": 100,
      "latency_p99_ns": 200
    }
  ],
  "summary": {
    "max_throughput_mpps": 120.2,
    "avg_cpu_percent": 58.7,
    "target_met": true
  }
}
```
