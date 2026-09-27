# MTS Router — Embedded Linux Subsystem

This directory contains all embedded Linux build infrastructure for the MTS Router family of products.

## Overview

The embedded Linux subsystem provides:

- **Yocto/Bitbake images** for Core Router (MTS-CR-9000) and Mobile Core (MTS-MC-5000)
- **Buildroot images** for Mobile Backhaul (MTS-MB-3000)
- **OpenWrt images** for OLT GPON (MTS-OLT-2000), Enterprise Router (MTS-ER-1000), and Residential Gateway (MTS-RG-500)
- **U-Boot bootloader** configurations for all devices
- **Test suites** for boot sequence, networking stack, and DPDK integration

## Directory Structure

```
linux/
├── meta-mts/                          # Yocto layer for all MTS devices
│   ├── conf/
│   │   ├── layer.conf                 # Layer configuration
│   │   └── machine/                   # Machine configurations
│   │       ├── mts-cr9000.conf        # Core Router
│   │       ├── mts-mc5000.conf        # Mobile Core
│   │       ├── mts-mb3000.conf        # Mobile Backhaul
│   │       ├── mts-olt2000.conf       # OLT GPON
│   │       ├── mts-er1000.conf        # Enterprise Router
│   │       └── mts-rg500.conf         # Residential Gateway
│   ├── classes/
│   │   └── mts-board.bbclass          # Base board class
│   ├── recipes-core/
│   │   └── images/                    # Image recipes
│   │       ├── mts-image-common.inc   # Common image config
│   │       ├── mts-core-router-image.bb
│   │       ├── mts-mobile-core-image.bb
│   │       ├── mts-mobile-backhaul-image.bb
│   │       ├── mts-olt-gpon-image.bb
│   │       ├── mts-enterprise-image.bb
│   │       └── mts-residential-image.bb
│   ├── recipes-kernel/
│   │   └── linux/
│   │       ├── mts-kernel-%.bbappend  # Kernel recipe
│   │       └── configs/               # Kernel config fragments
│   │           ├── mts-base.cfg
│   │           ├── mts-cr9000.cfg
│   │           ├── mts-mc5000.cfg
│   │           ├── mts-mb3000.cfg
│   │           ├── mts-olt2000.cfg
│   │           ├── mts-er1000.cfg
│   │           └── mts-rg500.cfg
│   └── recipes-bsp/
│       └── u-boot/
│           ├── u-boot-mts_2024.04.bb
│           └── files/
│               └── uEnv.txt
├── buildroot/
│   └── mts-s32g3-defconfig            # Buildroot config for S32G3
└── README.md
```

## Quick Start

### Building Core Router (Yocto)

```bash
# Initialize Yocto environment
source poky/oe-init-build-env build/cr9000

# Add layers
bitbake-layers add-layer ../meta-openembedded
bitbake-layers add-layer ../meta-virtualization
bitbake-layers add-layer ../../linux/meta-mts

# Build
bitbake mts-core-router-image
```

### Building Mobile Core (Yocto + K3s)

```bash
source poky/oe-init-build-env build/mc5000
bitbake-layers add-layer ../meta-openembedded
bitbake-layers add-layer ../meta-virtualization
bitbake-layers add-layer ../../linux/meta-mts
bitbake mts-mobile-core-image
```

### Building Mobile Backhaul (Buildroot)

```bash
# From buildroot directory
make mts-s32g3_defconfig
make BR2_EXTERNAL=../../linux/meta-mts
```

### Building OLT/Enterprise/Residential (OpenWrt)

```bash
# OLT GPON
./scripts/build-olt-gpon.sh

# Enterprise Router
./scripts/build-enterprise.sh

# Residential Gateway
./scripts/build-residential.sh
```

## Running Tests

### Boot Sequence Tests

```bash
# Test all devices
./scripts/tests/test-boot-sequence.sh all

# Test specific device
./scripts/tests/test-boot-sequence.sh mts-cr9000
```

### Networking Stack Tests

```bash
# Test all devices
./scripts/tests/test-networking-stack.sh all

# Test specific device
./scripts/tests/test-networking-stack.sh mts-mc5000
```

### DPDK Integration Tests

```bash
# Test all devices
./scripts/tests/test-dpdk-integration.sh all

# Test specific device
./scripts/tests/test-dpdk-integration.sh mts-cr9000
```

### Image Verification

```bash
# Verify all images
./scripts/verify-image.sh all

# Verify specific device
./scripts/verify-image.sh mts-cr9000
```

## Device Specifications

### MTS-CR-9000 (Core Router)
- **Processor:** AMD EPYC x86_64
- **ASIC:** Intel Tofino 2
- **OS:** Yocto (Kirkstone)
- **Key Features:** BGP/MPLS/SRv6, P4Runtime, DPDK
- **Image:** `mts-core-router-image.bb`

### MTS-MC-5000 (Mobile Core)
- **Processor:** AMD EPYC + ThunderX3 ARM64
- **OS:** Yocto (Kirkstone) + K3s
- **Key Features:** 5G UPF/SMF/AMF/PCF, Kubernetes, DPDK
- **Image:** `mts-mobile-core-image.bb`

### MTS-MB-3000 (Mobile Backhaul)
- **Processor:** NXP S32G3 ARM32
- **Switch:** Marvell 88Q5242
- **OS:** Buildroot
- **Key Features:** MPLS-TP, PTP grandmaster, SyncE, DPDK
- **Config:** `mts-s32g3-defconfig`

### MTS-OLT-2000 (OLT GPON)
- **Processor:** AMD EPYC x86_64
- **ASIC:** Intel Tofino 2
- **GPON:** Realtek RTL960x
- **OS:** OpenWrt 23.05
- **Key Features:** GPON OMCI, TR-069, LuCI
- **Build:** `scripts/build-olt-gpon.sh`

### MTS-ER-1000 (Enterprise Router)
- **Processor:** NXP S32G3 ARM32
- **Switch:** TomTom
- **OS:** OpenWrt 23.05
- **Key Features:** SD-WAN, IPSec, BGP/OSPF
- **Build:** `scripts/build-enterprise.sh`

### MTS-RG-500 (Residential Gateway)
- **Processor:** MediaTek MT7981 ARM64
- **GPON:** Realtek RTL960x
- **WiFi:** MT76 (WiFi 6)
- **OS:** OpenWrt 23.05
- **Key Features:** VoIP (Asterisk), IPTV, TR-069, WiFi 6
- **Build:** `scripts/build-residential.sh`

## CI/CD Pipeline

The CI/CD pipeline is defined in [`.github/workflows/embedded-linux.yml`](../.github/workflows/embedded-linux.yml).

### Pipeline Stages

1. **Layer Validation** - Validates meta-mts layer structure
2. **Image Building** - Builds images for each device
3. **Boot Testing** - Tests boot sequence
4. **Network Testing** - Tests networking stack
5. **DPDK Testing** - Tests DPDK integration
6. **Image Verification** - Verifies image integrity
7. **Artifact Upload** - Uploads build artifacts

### Triggering Builds

```bash
# Via GitHub Actions UI
# Go to Actions > Embedded Linux CI/CD > Run workflow

# Via CLI (gh CLI)
gh workflow run embedded-linux.yml \
  -f device=all \
  -f build_type=full
```

## Machine Configuration Reference

Each machine configuration file (`conf/machine/*.conf`) defines:

| Variable | Description |
|----------|-------------|
| `MACHINE` | Machine identifier |
| `TARGET_ARCH` | Target architecture |
| `MTS_BOARD_MODEL` | Board model name |
| `MTS_BOARD_OS` | OS type (yocto/buildroot/openwrt) |
| `IMAGE_FSTYPES` | Output image formats |
| `OPENWRT_SUPPORT` | OpenWrt build support flag |
| `BUILDROOT_SUPPORT` | Buildroot build support flag |
| `DPDK_SUPPORT` | DPDK support flag |
| `BGP_SUPPORT` | BGP routing support flag |
| `MPLS_SUPPORT` | MPLS support flag |
| `PTP_SUPPORT` | Precision Time Protocol support |

## Kernel Configuration Fragments

Kernel config fragments are located in `recipes-kernel/linux/configs/`:

| File | Purpose |
|------|---------|
| `mts-base.cfg` | Common kernel options for all devices |
| `mts-cr9000.cfg` | Tofino 2 + x86_64 specific options |
| `mts-mc5000.cfg` | ThunderX3 + ARM64 + K3s options |
| `mts-mb3000.cfg` | S32G3 + MPLS-TP + PTP options |
| `mts-olt2000.cfg` | Tofino 2 + RTL960x + OpenWrt options |
| `mts-er1000.cfg` | S32G3 + TomTom + IPSec options |
| `mts-rg500.cfg` | MT7981 + RTL960x + WiFi 6 options |

## Troubleshooting

### Common Issues

1. **Yocto build fails with layer errors**
   - Ensure all required layers are added: `meta-openembedded`, `meta-virtualization`
   - Check Yocto version compatibility

2. **OpenWrt build fails**
   - Install required host packages: `sudo scripts/install-openwrt-deps.sh`
   - Check SDK download URL is accessible

3. **Buildroot build fails**
   - Verify defconfig file exists
   - Check BR_PATH is set correctly

4. **Tests fail**
   - Ensure build artifacts exist before running tests
   - Check test result logs in `build/test-results/`

### Getting Help

- Check logs in `build/*/` directories
- Review test results in `build/test-results/`
- See [AGENTS.md](../AGENTS.md) for architecture overview
