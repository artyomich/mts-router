/**
 * MTS-OLT-2000 GPON Engine — GPON PHY monitoring implementation
 * Provides mock implementation for GPON port statistics and status
 */

#include "hal/gpon_hal.h"
#include <cstdio>
#include <cstring>
#include <chrono>

static bool gpon_initialized = false;
static mts_olt2000_olt_status_t olt_status;
static mts_olt2000_pon_port_t pon_ports[MTS_OLT2000_MAX_PON_PORTS];
static uint32_t num_pon_ports = 0;

int mts_olt2000_gpon_init(void) {
    if (gpon_initialized) {
        return 0;
    }

    memset(&olt_status, 0, sizeof(olt_status));
    memset(pon_ports, 0, sizeof(pon_ports));

    snprintf(olt_status.olt_id, sizeof(olt_status.olt_id), "MTS-OLT-2000-001");
    olt_status.status = MTS_OLT2000_PON_PORT_UP;
    olt_status.pon_ports = MTS_OLT2000_MAX_PON_PORTS;
    strlcpy(olt_status.firmware_version, "1.0.0", sizeof(olt_status.firmware_version));

    num_pon_ports = MTS_OLT2000_MAX_PON_PORTS;
    for (uint32_t i = 0; i < num_pon_ports; i++) {
        pon_ports[i].port_id = i;
        pon_ports[i].status = MTS_OLT2000_PON_PORT_UP;
        pon_ports[i].rx_power_dbm = -15.0;
        pon_ports[i].tx_power_dbm = 18.0;
        pon_ports[i].temperature_c = 45.0;
        pon_ports[i].active_onu = 0;
        pon_ports[i].max_onu = MTS_OLT2000_MAX_ONU_PER_PORT;
    }

    gpon_initialized = true;
    return 0;
}

void mts_olt2000_gpon_cleanup(void) {
    gpon_initialized = false;
    num_pon_ports = 0;
}

int mts_olt2000_gpon_get_olt_status(mts_olt2000_olt_status_t *status) {
    if (!status) return -1;
    *status = olt_status;
    return 0;
}

int mts_olt2000_gpon_get_pon_port(uint32_t port_id, mts_olt2000_pon_port_t *port) {
    if (!port || port_id >= num_pon_ports) return -1;
    *port = pon_ports[port_id];
    return 0;
}

int mts_olt2000_gpon_get_all_pon_ports(mts_olt2000_pon_port_t *ports, uint32_t max_ports, uint32_t *count) {
    if (!ports || !count || max_ports == 0) return -1;
    uint32_t actual_count = (max_ports < num_pon_ports) ? max_ports : num_pon_ports;
    for (uint32_t i = 0; i < actual_count; i++) {
        ports[i] = pon_ports[i];
    }
    *count = actual_count;
    return 0;
}

int mts_olt2000_gpon_enable_port(uint32_t port_id) {
    if (port_id >= num_pon_ports) return -1;
    pon_ports[port_id].status = MTS_OLT2000_PON_PORT_UP;
    return 0;
}

int mts_olt2000_gpon_disable_port(uint32_t port_id) {
    if (port_id >= num_pon_ports) return -1;
    pon_ports[port_id].status = MTS_OLT2000_PON_PORT_DOWN;
    return 0;
}
