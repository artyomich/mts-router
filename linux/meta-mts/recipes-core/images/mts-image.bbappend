# MTS Image bbappend for all devices
# Provides device-specific image configuration

# Core Router image
IMAGE_INSTALL_append_mts-cr9000 = " \
    ${MTS_CR_PACKAGES} \
"

# Mobile Core image
IMAGE_INSTALL_append_mts-mc5000 = " \
    ${MTS_MC_PACKAGES} \
"

# Mobile Backhaul image
IMAGE_INSTALL_append_mts-mb3000 = " \
    ${MTS_MB_PACKAGES} \
"

# OLT GPON image
IMAGE_INSTALL_append_mts-olt2000 = " \
    ${MTS_OLT_PACKAGES} \
"

# Enterprise Router image
IMAGE_INSTALL_append_mts-er1000 = " \
    ${MTS_ER_PACKAGES} \
"

# Residential Gateway image
IMAGE_INSTALL_append_mts-rg500 = " \
    ${MTS_RG_PACKAGES} \
"

# Common packages for all devices
IMAGE_INSTALL_append = " \
    ${MTS_COMMON_PACKAGES} \
"

# Image features
IMAGE_FEATURES += "ssh-server-opensysv package-management"

# Enable telemetry
EXTRA_IMAGEFEATURES += "telemetry-dump"
