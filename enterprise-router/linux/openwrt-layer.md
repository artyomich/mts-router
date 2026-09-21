# MTS-ER-1000 — OpenWrt Layer

## 1. Структура проекта

```
mts-enterprise-router/
├── package/
│   ├── mts-sdwan/
│   │   ├── Makefile
│   │   └── src/
│   │       └── sdwan_engine.c
│   └── mts-er/
│       ├── Makefile
│       └── src/
│           └── er_core.c
├── target/linux/
│   └── mts-er/
│       ├── files/
│       ├── image/
│       ├── mts-er.dts
│       └── patches-6.6/
├── configs/
│   └── mts-er.config
├── feeds/
│   └── mts/
│       └── packages/
└── build/
```

## 2. target/linux/mts-er/files/board.d/mts-er

```bash
# Board init script for MTS-ER-1000

board_name() {
    echo "mts-er-1000"
}

board_detect() {
    local board_id
    board_id=$(cat /sys/class/i2c-adapter/i2c-1/1-0050/eeprom 2>/dev/null)
    if [ -n "$board_id" ] && echo "$board_id" | grep -q "MTS-ER-1000"; then
        return 0
    fi
    return 1
}

board_setup() {
    echo "Setting up MTS-ER-1000 board..."

    # Configure WAN ports
    for i in $(seq 0 3); do
        echo "Configuring WAN port $i..."
    done

    # Configure LAN ports
    for i in $(seq 0 7); do
        echo "Configuring LAN port $i..."
    done

    # Setup LEDs
    echo "Setting up LEDs..."

    # Setup fans
    echo "Setting up fans..."
}

board_preupgrade() {
    echo "Pre-upgrade cleanup..."
}

board_postupgrade() {
    echo "Post-upgrade setup..."
}