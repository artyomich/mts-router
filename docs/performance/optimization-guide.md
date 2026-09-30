# MTS Router — Performance Optimization Guide

## Overview

Comprehensive performance optimization guide for all MTS Router devices.

## 1. CPU Optimization

### 1.1 Interrupt Affinity
```bash
# Bind network interrupts to specific CPUs
echo 0x3 > /proc/irq/$(cat /sys/class/net/eth0/device/irq)/smp_affinity

# Bind DPDK interrupts
echo 0xC > /proc/irq/<dpdk_irq>/smp_affinity
```

### 1.2 CPU Governor
```bash
# Set performance governor
echo performance > /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Set turbo boost
echo 1 > /sys/devices/system/cpu/cpufreq/boost
```

### 1.3 Hugepages
```bash
# Allocate hugepages for DPDK
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# Mount hugetlbfs
mount -t hugetlbfs none /dev/hugepages
```

## 2. Network Optimization

### 2.1 Socket Buffers
```bash
# Increase socket buffers
sysctl -w net.core.rmem_max=16777216
sysctl -w net.core.wmem_max=16777216
sysctl -w net.core.rmem_default=16777216
sysctl -w net.core.wmem_default=16777216
```

### 2.2 TCP Tuning
```bash
# TCP optimization
sysctl -w net.ipv4.tcp_max_syn_backlog=65535
sysctl -w net.ipv4.tcp_syncookies=1
sysctl -w net.ipv4.tcp_tw_reuse=1
sysctl -w net.ipv4.tcp_fin_timeout=15
sysctl -w net.ipv4.tcp_keepalive_time=600
sysctl -w net.ipv4.tcp_keepalive_intvl=30
sysctl -w net.ipv4.tcp_keepalive_probes=5
```

### 2.3 Network Parameters
```bash
# Network parameters
sysctl -w net.core.netdev_max_backlog=5000
sysctl -w net.core.optmem_max=81920
sysctl -w net.ipv4.ip_local_port_range="1024 65535"
sysctl -w net.ipv4.tcp_max_tw_buckets=1000000
```

## 3. Memory Optimization

### 3.1 NUMA Awareness
```bash
# Check NUMA topology
numactl --hardware

# Bind to specific NUMA node
numactl --cpunodebind=0 --membind=0 <command>
```

### 3.2 Transparent Huge Pages
```bash
# Disable THP for DPDK
echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag
```

## 4. DPDK Optimization

### 4.1 DPDK Configuration
```bash
# Setup DPDK
modprobe uio
insmod igb_uio.ko
dpdk-devbind.py --bind=uio-pci-generic eth0 eth1

# Hugepages
hugectl --pages 1024
```

### 4.2 DPDK Parameters
```bash
# DPDK performance parameters
export DPDK_RXTX_CALLBACKS=1
export DPDK_MAX_PKT_BURST=512
export DPDK_RX_DESC=4096
export DPDK_TX_DESC=4096
```

## 5. BGP Optimization

### 5.1 BGP Parameters
```bash
# BGP optimization
sysctl -w net.ipv4.tcp_max_syn_backlog=65535
sysctl -w net.core.somaxconn=65535
```

### 5.2 BGP Tuning
```bash
# BGP session optimization
# - Increase BGP table capacity
# - Enable graceful restart
# - Enable multipath
# - Configure route dampening
```

## 6. MPLS Optimization

### 6.1 MPLS Parameters
```bash
# MPLS optimization
sysctl -w net.ipv4.conf.all.mpls_rpf_loose_feat=1
sysctl -w net.ipv6.conf.all.mpls_rpf_loose_feat=1
```

### 6.2 MPLS Tuning
```bash
# MPLS stack size
sysctl -w net.ipv4.mpls.route_mtu_discovery=1
```

## 7. SRv6 Optimization

### 7.1 SRv6 Parameters
```bash
# SRv6 optimization
sysctl -w net.ipv6.conf.all.autoconf=0
sysctl -w net.ipv6.conf.all.accept_ra=0
```

### 7.2 SRv6 Tuning
```bash
# SRv6 SID table optimization
# - Use TCAM for SID lookup
# - Enable SID compression
# - Enable SID caching
```

## 8. Device-Specific Optimizations

### 8.1 Core Router (MTS-CR-9000)
- Intel Tofino 2: Enable P4 pipeline optimization
- EPYC: Use all cores for control plane
- 400G ports: Enable RSS for load balancing

### 8.2 Mobile Core (MTS-MC-5000)
- ThunderX3: Enable ARM NEON for GTP-U
- EPYC: Use NUMA-aware allocation
- K3s: Tune container networking

### 8.3 Mobile Backhaul (MTS-MB-3000)
- S32G3: Enable hardware timestamping
- 10G ports: Enable jumbo frames
- PTP: Enable grandmaster mode

### 8.4 OLT GPON (MTS-OLT-2000)
- Tofino 2: Enable TCAM optimization
- RTL960x: Enable ONU discovery acceleration
- GPON: Enable GEM port aggregation

### 8.5 Enterprise Router (MTS-ER-1000)
- S32G3: Enable hardware IPsec
- TomTom: Enable ASIC offload
- SD-WAN: Enable path selection optimization

### 8.6 Residential Gateway (MTS-RG-500)
- MT7981: Enable WiFi 6 TWT
- RTL960x: Enable ONU power saving
- VoIP: Enable codec optimization
