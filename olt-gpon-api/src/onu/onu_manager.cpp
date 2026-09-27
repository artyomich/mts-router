/**
 * MTS-OLT-2000 ONU Manager — ONU discovery and management
 * Provides mock implementation for ONU state tracking
 */

#include "hal/onu_hal.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

static mts_olt2000_onu_t onus[MTS_OLT2000_MAX_ONUS];
static uint32_t num_onus = 0;

static int discover_onu_internal(uint32_t pon_port, mts_olt2000_onu_t **onus_out, uint32_t *count) {
    if (!onus_out || !count) return -1;
    *count = 0;
    *onus_out = nullptr;
    return 0;
}

static int activate_onu_internal(uint32_t pon_port, const char *serial, mts_olt2000_onu_t *onu) {
    if (!serial || !onu) return -1;
    if (num_onus >= MTS_OLT2000_MAX_ONUS) return -1;

    memset(onu, 0, sizeof(*onu));
    snprintf(onu->onu_id, sizeof(onu->onu_id), "ONU-%04d", num_onus);
    onu->pon_port = pon_port;
    onu->status = MTS_OLT2000_ONU_ACTIVATING;
    strlcpy(onu->serial_number, serial, sizeof(onu->serial_number));
    strlcpy(onu->mac_address, "00:11:22:33:44:55", sizeof(onu->mac_address));
    strlcpy(onu->firmware_version, "1.0.0", sizeof(onu->firmware_version));
    onu->power_level_dbm = -20;
    onu->distance_m = 1000;
    onu->vlan = 100;
    strlcpy(onu->qos_profile, "standard", sizeof(onu->qos_profile));
    onu->bandwidth_up_mbps = 100;
    onu->bandwidth_down_mbps = 1000;

    num_onus++;
    return 0;
}

static int deactivate_onu_internal(const char *onu_id) {
    if (!onu_id) return -1;
    for (uint32_t i = 0; i < num_onus; i++) {
        if (strcmp(onus[i].onu_id, onu_id) == 0) {
            onus[i].status = MTS_OLT2000_ONU_OFFLINE;
            return 0;
        }
    }
    return -1;
}

static int get_tlv_internal(const char *onu_id, uint16_t *type, void *value, uint32_t *len) {
    (void)onu_id;
    (void)type;
    (void)value;
    (void)len;
    return 0;
}

static int set_tlv_internal(const char *onu_id, uint16_t type, const void *value, uint32_t len) {
    (void)onu_id;
    (void)type;
    (void)value;
    (void)len;
    return 0;
}

static mts_olt2000_onu_ops_t onu_ops = {
    .init = nullptr,
    .cleanup = nullptr,
    .discover = discover_onu_internal,
    .activate = activate_onu_internal,
    .deactivate = deactivate_onu_internal,
    .get_tlv = get_tlv_internal,
    .set_tlv = set_tlv_internal
};

const mts_olt2000_onu_ops_t *mts_olt2000_onu_get_ops(void) {
    return &onu_ops;
}

int mts_olt2000_onu_register_ops(const mts_olt2000_onu_ops_t *ops) {
    if (!ops) return -1;
    return 0;
}
