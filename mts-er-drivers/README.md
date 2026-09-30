# MTS Enterprise Router Drivers (MTS-ER-1000)

Drivers for the MTS Enterprise Router based on NXP S32G3 and Broadcom TomTom ASIC.

## Directory Structure

```
mts-er-drivers/
├── s32g3/
│   ├── include/
│   │   └── s32g3_er.h          # S32G3 driver headers
│   ├── src/
│   │   └── s32g3_er.c          # S32G3 driver implementation
│   ├── tests/
│   │   ├── unit/               # Unit tests
│   │   └── integration/        # Integration tests
│   └── Makefile
├── tomtom/
│   ├── include/
│   │   └── tomtom_er.h         # TomTom ASIC headers
│   ├── src/
│   │   └── tomtom_er.c         # TomTom ASIC implementation
│   ├── tests/
│   │   ├── unit/               # Unit tests
│   │   └── integration/        # Integration tests
│   └── Makefile
└── README.md
```

## Components

### S32G3 Driver

Main SoC driver for NXP S32G3:
- Port management (4x 10GbE ports)
- Interface management (physical, VLAN, bond, tunnel)
- Route management (up to 8192 routes)
- QoS management (8 queues per port)

### TomTom ASIC Driver

Broadcom TomTom ASIC driver for advanced forwarding:
- TCAM management (16 tables)
- FDB (MAC forwarding database) up to 4096 entries
- RDB (routing database) up to 2048 entries
- ACL management (8192 entries)

## Building

```bash
# Build S32G3 driver
cd s32g3
make

# Build TomTom driver
cd ../tomtom
make
```

## Installation

```bash
sudo make install
sudo modprobe s32g3_er
sudo modprobe tomtom_er
```

## Testing

```bash
# Run driver tests
make test

# Run kernel module tests
sudo insmod s32g3_er.ko
sudo insmod tomtom_er.ko
sudo rmmod tomtom_er s32g3_er
```

## License

GPL-2.0
