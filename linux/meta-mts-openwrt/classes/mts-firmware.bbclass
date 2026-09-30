# MTS Firmware Update Class
# Provides firmware update functionality for MTS Router devices

# ============================================================================
# Firmware update variables
# ============================================================================

# Firmware partition layout
MTS_FW_BOOT_PART ?= "/dev/mmcblk0p1"
MTS_FW_ROOT_PART ?= "/dev/mmcblk0p2"
MTS_FW_FWLDR_PART ?= "/dev/mmcblk0p3"

# Firmware update settings
MTS_FW_UPDATE_METHOD ?= "dual-bank"
MTS_FW_BANK_A ?= "/dev/mmcblk0p2"
MTS_FW_BANK_B ?= "/dev/mmcblk0p3"
MTS_FW_ACTIVE_BANK ?= "A"

# Firmware signature verification
MTS_FW_VERIFY_SIG ?= "true"
MTS_FW_SIG_KEY ?= "/etc/firmware/signing.pem"

# Firmware rollback settings
MTS_FW_ROLLBACK_ENABLED ?= "true"
MTS_FW_ROLLBACK_THRESHOLD ?= "3"
MTS_FW_ROLLBACK_TIMEOUT ?= "300"

# ============================================================================
# Firmware image creation
# ============================================================================

mts_create_fw_image() {
    local fw_name=$1
    local fw_version=$2
    local fw_type=$3
    
    # Create firmware header
    cat > ${WORKDIR}/${fw_name}.hdr <<EOF
MAGIC=MTSFW
VERSION=${fw_version}
TYPE=${fw_type}
DEVICE=${MACHINE}
SIZE=$(stat -c%s ${WORKDIR}/${fw_name}.img 2>/dev/null || echo 0)
CRC32=0x00000000
SIGNATURE=
EOF
}

# ============================================================================
# Firmware signing
# ============================================================================

mts_sign_fw() {
    local fw_file=$1
    local key_file=$2
    
    if [ "${MTS_FW_VERIFY_SIG}" = "true" ] && [ -n "${key_file}" ]; then
        openssl dgst -sha256 -sign ${key_file} -out ${fw_file}.sig ${fw_file}
        echo "Firmware signed: ${fw_file}.sig"
    fi
}

# ============================================================================
# Firmware verification
# ============================================================================

mts_verify_fw() {
    local fw_file=$1
    local sig_file=$2
    local pub_key=$3
    
    if [ "${MTS_FW_VERIFY_SIG}" = "true" ] && [ -n "${sig_file}" ] && [ -n "${pub_key}" ]; then
        if openssl dgst -sha256 -verify ${pub_key} -signature ${sig_file} ${fw_file}; then
            echo "Firmware signature verified"
            return 0
        else
            echo "Firmware signature verification failed"
            return 1
        fi
    fi
    return 0
}

# ============================================================================
# Firmware install
# ============================================================================

mts_install_fw() {
    local fw_file=$1
    local target_bank=$2
    
    if [ -z "${D}" ]; then
        # Running on target
        local active_bank=${MTS_FW_ACTIVE_BANK:-A}
        local target_part
        
        case "${active_bank}" in
            A) target_part=${MTS_FW_BANK_B} ;;
            B) target_part=${MTS_FW_BANK_A} ;;
            *) target_part=${MTS_FW_BANK_B} ;;
        esac
        
        # Write firmware to inactive bank
        dd if=${fw_file} of=${target_part} bs=4096 conv=fsync
        
        # Update boot counter
        echo "Firmware installed to bank ${target_bank}"
    else
        # Running in build environment
        install -d ${D}${sysconfdir}/mts/firmware
        install -m 0644 ${fw_file} ${D}${sysconfdir}/mts/firmware/active.img
    fi
}

# ============================================================================
# Firmware rollback
# ============================================================================

mts_fw_rollback() {
    local active_bank=$1
    
    if [ "${MTS_FW_ROLLBACK_ENABLED}" = "true" ]; then
        local target_bank
        
        case "${active_bank}" in
            A) target_bank=B ;;
            B) target_bank=A ;;
            *) target_bank=B ;;
        esac
        
        echo "Rolling back to bank ${target_bank}"
        
        # Update boot configuration
        echo "boot_bank=${target_bank}" > /etc/mts/firmware/boot.conf
        
        # Reset boot counter
        echo "boot_count=0" >> /etc/mts/firmware/boot.conf
    fi
}

# ============================================================================
# Firmware health check
# ============================================================================

mts_fw_health_check() {
    local boot_count=${1:-0}
    local threshold=${MTS_FW_ROLLBACK_THRESHOLD:-3}
    local timeout=${MTS_FW_ROLLBACK_TIMEOUT:-300}
    
    if [ "${boot_count}" -ge "${threshold}" ]; then
        echo "Health check failed: boot count ${boot_count} >= threshold ${threshold}"
        return 1
    fi
    
    # Check firmware integrity
    if [ -f /etc/mts/firmware/active.img ]; then
        local crc
        crc=$(crc32 /etc/mts/firmware/active.img)
        local expected_crc
        expected_crc=$(cat /etc/mts/firmware/active.img.crc 2>/dev/null || echo "")
        
        if [ "${crc}" != "${expected_crc}" ]; then
            echo "Firmware CRC mismatch"
            return 1
        fi
    fi
    
    echo "Health check passed"
    return 0
}

# ============================================================================
# Firmware update service
# ============================================================================

mts_create_fw_update_service() {
    cat > ${WORKDIR}/mts-fw-update.service <<EOF
[Unit]
Description=MTS Firmware Update Service
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
ExecStart=/usr/bin/mts-fw-update
ExecReload=/bin/kill -HUP \$MAINPID
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

    cat > ${WORKDIR}/mts-fw-update <<EOF
#!/bin/sh
# MTS Firmware Update Service

FW_DIR=/etc/mts/firmware
BOOT_CONF=\${FW_DIR}/boot.conf
LOCK_FILE=/var/run/mts-fw-update.lock

# Read active bank
source \${BOOT_CONF} 2>/dev/null
ACTIVE_BANK=\${boot_bank:-A}

# Check for pending updates
check_pending_updates() {
    if [ -f \${FW_DIR}/pending.img ]; then
        return 0
    fi
    return 1
}

# Install pending update
install_update() {
    local target_bank
    case "\${ACTIVE_BANK}" in
        A) target_bank=B ;;
        B) target_bank=A ;;
        *) target_bank=B ;;
    esac
    
    # Verify firmware
    if ! mts_verify_fw /etc/mts/firmware/pending.img /etc/mts/firmware/pending.img.sig /etc/mts/firmware/signing.pem; then
        logger -t mts-fw-update "Firmware verification failed"
        return 1
    fi
    
    # Install firmware
    mts_install_fw /etc/mts/firmware/pending.img \${target_bank}
    
    # Update boot configuration
    echo "boot_bank=\${target_bank}" > \${BOOT_CONF}
    echo "boot_count=0" >> \${BOOT_CONF}
    
    # Clean up
    rm -f /etc/mts/firmware/pending.img /etc/mts/firmware/pending.img.sig
    
    logger -t mts-fw-update "Firmware update installed successfully"
    return 0
}

# Main loop
while true; do
    if check_pending_updates; then
        install_update
    fi
    sleep 60
done
EOF
    chmod +x ${WORKDIR}/mts-fw-update
}

# ============================================================================
# Class initialization
# ============================================================================

python mts_firmware_init() {
    # Set default values
    d.setVar('MTS_FW_UPDATE_METHOD', 'dual-bank')
    d.setVar('MTS_FW_VERIFY_SIG', 'true')
    d.setVar('MTS_FW_ROLLBACK_ENABLED', 'true')
}

mts_firmware_init()
