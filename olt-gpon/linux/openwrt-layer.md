# MTS-OLT-2000 — OpenWrt Layer

## 1. Структура проекта

```
mts-olt-gpon/
├── package/
│   ├── mts-olt/
│   │   ├── Makefile
│   │   ├── src/
│   │   │   ├── olt_core.c
│   │   │   ├── olt_gpon.c
│   │   │   └── olt_omci.c
│   │   └── files/
│   │       └── init.d/mts-olt
│   ├── rtl960x-driver/
│   │   ├── Makefile
│   │   └── src/
│   │       └── rtl960x_gpon.c
│   └── mts-tr069/
│       ├── Makefile
│       └── src/
│           └── cwmpd.c
├── target/linux/
│   └── mts-olt-gpon/
│       ├── files/
│       ├── image/
│       ├── mts-olt-gpon.dts
│       └── patches-6.6/
├── configs/
│   └── mts-olt-gpon.config
├── feeds/
│   └── mts/
│       └── packages/
└── build/
```

## 2. target/linux/mts-olt-gpon/files/board.d/mts-olt-gpon

```bash
# Board init script for MTS-OLT-2000

board_name() {
    echo "mts-olt-gpon"
}

board_detect() {
    # Detect board via UART or EEPROM
    local board_id
    board_id=$(cat /sys/class/i2c-adapter/i2c-1/1-0050/eeprom 2>/dev/null)
    if [ -n "$board_id" ] && echo "$board_id" | grep -q "MTS-OLT-2000"; then
        return 0
    fi
    return 1
}

board_setup() {
    # Setup GPIO for power control
    echo "Setting up MTS-OLT-2000 board..."

    # Configure GPON ports
    for i in $(seq 0 15); do
        echo "Configuring GPON port $i..."
    done

    # Configure uplink ports
    echo "Configuring uplink ports..."

    # Setup LEDs
    echo "Setting up LEDs..."

    # Setup fans
    echo "Setting up fans..."
}

board_preupgrade() {
    # Pre-upgrade cleanup
    echo "Pre-upgrade cleanup..."
}

board_postupgrade() {
    # Post-upgrade setup
    echo "Post-upgrade setup..."
}