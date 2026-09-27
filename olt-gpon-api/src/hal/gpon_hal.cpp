/**
 * MTS-OLT-2000 GPON HAL — GPON PHY management implementation
 * Provides mock implementation for GPON port and ONU monitoring
 */

#include "hal/gpon_hal.h"
#include <cstdio>
#include <cstring>
#include <chrono>

static bool gpon_initialized = false;
static mts_olt2000_olt_status_t g_olt_status;
static mts_olt2000_pon_port_t g_pon_ports[MTS_OLT2000_MAX_PON_PORTS];
static mts_olt2000_onu_t g_onus[MTS_OLT2000_MAX_ONUS];
static uint32_t g_num_onus = 0;

int mts_olt2000_gpon_init(void) {
    if (gpon_initialized) return 0;

    memset(&g_olt_status, 0, sizeof(g_olt_status));
    memset(g_pon_ports, 0, sizeof(g_pon_ports));
    memset(g_onus, 0, sizeof(g_onus));

    strlcpy(g_olt_status.olt_id, "MTS-OLT-2000-001", sizeof(g_olt_status.olt_id));
    g_olt_status.status = MTS_OLT2000_PON_PORT_UP;
    strlcpy(g_olt_status.firmware_version, "1.0.0", sizeof(g_olt_status.firmware_version));
    g_olt_status.pon_ports = MTS_OLT2000_MAX_PON_PORTS;

    for (uint32_t i = 0; i < MTS_OLT2000_MAX_PON_PORTS; i++) {
        g_pon_ports[i].port_id = i;
        g_pon_ports[i].status = MTS_OLT2000_PON_PORT_UP;
        g_pon_ports[i].rx_power_dbm = -15.0;
        g_pon_ports[i].tx_power_dbm = 18.0;
        g_pon_ports[i].temperature_c = 45.0;
        g_pon_ports[i].active_onu = 0;
        g_pon_ports[i].max_onu = MTS_OLT2000_MAX_ONU_PER_PORT;
    }

    gpon_initialized = true;
    return 0;
}

void mts_olt2000_gpon_cleanup(void) {
    gpon_initialized = false;
    g_num_onus = 0;
}

int mts_olt2000_gpon_get_olt_status(mts_olt2000_olt_status_t *status) {
    if (!status) return -1;
    *status = g_olt_status;
    return 0;
}

int mts_olt2000_gpon_get_pon_port(uint32_t port_id, mts_olt2000_pon_port_t *port) {
    if (!port || port_id >= MTS_OLT2000_MAX_PON_PORTS) return -1;
    *port = g_pon_ports[port_id];
    return 0;
}

int mts_olt2000_gpon_get_all_pon_ports(mts_olt2000_pon_port_t *ports, int max_ports) {
    if (!ports || max_ports <= 0) return -1;
    int count = (max_ports < (int)MTS_OLT2000_MAX_PON_PORTS) ? max_ports : (int)MTS_OLT2000_MAX_PON_PORTS;
    for (int i = 0; i < count; i++) {
        ports[i] = g_pon_ports[i];
    }
    return count;
}

int mts_olt2000_gpon_get_onu(const char *onu_id, mts_olt2000_onu_t *onu) {
    if (!onu_id || !onu) return -1;
    for (uint32_t i = 0; i < g_num_onus; i++) {
        if (strcmp(g_onus[i].onu_id, onu_id) == 0) {
            *onu = g_onus[i];
            return 0;
        }
    }
    return -1;
}

int mts_olt2000_gpon_get_all_onus(mts_olt2000_onu_t *onus, int max_onus) {
    if (!onus || max_onus <= 0) return -1;
    int count = (max_onus < (int)g_num_onus) ? max_onus : (int)g_num_onus;
    for (int i = 0; i < count; i++) {
        onus[i] = g_onus[i];
    }
    return count;
}

int mts_olt2000_gpon_set_onu(const char *onu_id, uint32_t pon_port, uint32_t vlan,
                              const char *qos_profile, uint32_t bandwidth_up_mbps,
                              uint32_t bandwidth_down_mbps) {
    if (!onu_id || pon_port >= MTS_OLT2000_MAX_PON_PORTS) return -1;
    if (g_num_onus >= MTS_OLT2000_MAX_ONUS) return -1;

    uint32_t id = g_num_onus;
    memset(&g_onus[id], 0, sizeof(g_onus[id]));
    snprintf(g_onus[id].onu_id, sizeof(g_onus[id].onu_id), "ONU-%04d", id);
    g_onus[id].pon_port = pon_port;
    g_onus[id].status = MTS_OLT2000_ONU_ACTIVATING;
    strlcpy(g_onus[id].serial_number, "SN00000000", sizeof(g_onus[id].serial_number));
    strlcpy(g_onus[id].mac_address, "00:11:22:33:44:55", sizeof(g_onus[id].mac_address));
    strlcpy(g_onus[id].firmware_version, "1.0.0", sizeof(g_onus[id].firmware_version));
    g_onus[id].power_level_dbm = -20;
    g_onus[id].distance_m = 1000;
    g_onus[id].vlan = vlan;
    if (qos_profile) strlcpy(g_onus[id].qos_profile, qos_profile, sizeof(g_onus[id].qos_profile));
    g_onus[id].bandwidth_up_mbps = bandwidth_up_mbps;
    g_onus[id].bandwidth_down_mbps = bandwidth_down_mbps;
    g_onus[id].rx_bytes = 0;
    g_onus[id].tx_bytes = 0;

    g_pon_ports[pon_port].active_onu++;
    g_olt_status.online_onu++;
    g_num_onus++;
    return 0;
}

int mts_olt2000_gpon_reset_onu(const char *onu_id) {
    if (!onu_id) return -1;
    for (uint32_t i = 0; i < g_num_onus; i++) {
        if (strcmp(g_onus[i].onu_id, onu_id) == 0) {
            g_onus[i].status = MTS_OLT2000_ONU_ACTIVATING;
            return 0;
        }
    }
    return -1;
}
