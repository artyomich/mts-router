/**
 * MTS-OLT-2000 GPON HAL — GPON PHY management
 */

#ifndef MTS_OLT2000_GPON_HAL_H
#define MTS_OLT2000_GPON_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MTS_OLT2000_MAX_PON_PORTS 16
#define MTS_OLT2000_MAX_ONU_PER_PORT 128
#define MTS_OLT2000_MAX_ONUS 2048

/* PON port status */
typedef enum {
    MTS_OLT2000_PON_PORT_UP = 0,
    MTS_OLT2000_PON_PORT_DOWN,
    MTS_OLT2000_PON_PORT_ERROR,
    MTS_OLT2000_PON_PORT_INITIALIZING
} mts_olt2000_pon_port_status_t;

/* ONU status */
typedef enum {
    MTS_OLT2000_ONU_ONLINE = 0,
    MTS_OLT2000_ONU_OFFLINE,
    MTS_OLT2000_ONU_ERROR,
    MTS_OLT2000_ONU_ACTIVATING
} mts_olt2000_onu_status_t;

/* PON port info */
typedef struct {
    uint32_t port_id;
    mts_olt2000_pon_port_status_t status;
    double rx_power_dbm;
    double tx_power_dbm;
    double temperature_c;
    uint32_t active_onu;
    uint32_t max_onu;
} mts_olt2000_pon_port_t;

/* ONU info */
typedef struct {
    char onu_id[32];
    uint32_t pon_port;
    mts_olt2000_onu_status_t status;
    char serial_number[16];
    char mac_address[18];
    char firmware_version[64];
    int32_t power_level_dbm;
    uint32_t distance_m;
    uint32_t vlan;
    char qos_profile[32];
    uint32_t bandwidth_up_mbps;
    uint32_t bandwidth_down_mbps;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} mts_olt2000_onu_t;

/* OLT status */
typedef struct {
    char olt_id[32];
    mts_olt2000_pon_port_status_t status;
    char firmware_version[64];
    uint32_t total_onu;
    uint32_t online_onu;
    uint32_t pon_ports;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} mts_olt2000_olt_status_t;

/* ==================== API ==================== */

/**
 * Initialize GPON HAL
 * @return 0 on success, -1 on error
 */
int mts_olt2000_gpon_init(void);

/**
 * Cleanup GPON HAL
 */
void mts_olt2000_gpon_cleanup(void);

/**
 * Get OLT status
 * @param status Output buffer for OLT status
 * @return 0 on success, -1 on error
 */
int mts_olt2000_gpon_get_olt_status(mts_olt2000_olt_status_t *status);

/**
 * Get PON port status
 * @param port_id Port ID
 * @param port Output buffer for port info
 * @return 0 on success, -1 on error
 */
int mts_olt2000_gpon_get_pon_port(uint32_t port_id, mts_olt2000_pon_port_t *port);

/**
 * Get all PON port statuses
 * @param ports Output array of port info
 * @param max_ports Maximum number of ports
 * @return Number of ports returned, -1 on error
 */
int mts_olt2000_gpon_get_all_pon_ports(mts_olt2000_pon_port_t *ports, int max_ports);

/**
 * Get ONU info
 * @param onu_id ONU ID
 * @param onu Output buffer for ONU info
 * @return 0 on success, -1 on error
 */
int mts_olt2000_gpon_get_onu(const char *onu_id, mts_olt2000_onu_t *onu);

/**
 * Get all ONUs
 * @param onus Output array of ONU info
 * @param max_onus Maximum number of ONUs
 * @return Number of ONUs returned, -1 on error
 */
int mts_olt2000_gpon_get_all_onus(mts_olt2000_onu_t *onus, int max_onus);

/**
 * Configure ONU
 * @param onu_id ONU ID
 * @param pon_port PON port ID
 * @param vlan VLAN ID
 * @param qos_profile QoS profile name
 * @param bandwidth_up_mbps Upstream bandwidth in Mbps
 * @param bandwidth_down_mbps Downstream bandwidth in Mbps
 * @return 0 on success, -1 on error
 */
int mts_olt2000_gpon_set_onu(const char *onu_id, uint32_t pon_port, uint32_t vlan,
                               const char *qos_profile, uint32_t bandwidth_up_mbps,
                               uint32_t bandwidth_down_mbps);

/**
 * Reset ONU
 * @param onu_id ONU ID
 * @return 0 on success, -1 on error
 */
int mts_olt2000_gpon_reset_onu(const char *onu_id);

#ifdef __cplusplus
}
#endif

#endif /* MTS_OLT2000_GPON_HAL_H */
