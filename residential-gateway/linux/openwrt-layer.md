# MTS-RG-500 — OpenWrt Layer

## 1. Структура проекта

```
mts-residential-gateway/
├── package/
│   ├── mts-rg/
│   │   ├── Makefile
│   │   └── src/
│   │       └── rg_core.c
│   ├── mt7981-driver/
│   │   ├── Makefile
│   │   └── src/
│   │       └── mt7981_wifi.c
│   └── mts-voip/
│       ├── Makefile
│       └── src/
│           └── asterisk_mts.c
├── target/linux/
│   └── mts-rg/
│       ├── files/
│       ├── image/
│       ├── mts-rg.dts
│       └── patches-6.6/
├── configs/
│   └── mts-rg.config
├── feeds/
│   └── mts/
│       └── packages/
└── build/
```

## 2. target/linux/mts-rg/files/board.d/mts-rg

```bash
# Board init script for MTS-RG-500

board_name() {
    echo "mts-rg-500"
}

board_detect() {
    local board_id
    board_id=$(cat /sys/class/i2c-adapter/i2c-1/1-0050/eeprom 2>/dev/null)
    if [ -n "$board_id" ] && echo "$board_id" | grep -q "MTS-RG-500"; then
        return 0
    fi
    return 1
}

board_setup() {
    echo "Setting up MTS-RG-500 board..."

    # Configure GPON port
    echo "Configuring GPON port..."

    # Configure Ethernet ports
    for i in $(seq 0 3); do
        echo "Configuring Ethernet port $i..."
    done

    # Configure WiFi
    echo "Configuring WiFi..."

    # Configure VoIP
    echo "Configuring VoIP..."

    # Configure USB
    echo "Configuring USB..."
}

board_preupgrade() {
    echo "Pre-upgrade cleanup..."
}

board_postupgrade() {
    echo "Post-upgrade setup..."
}