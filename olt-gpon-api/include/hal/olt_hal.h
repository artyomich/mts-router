/*
 * olt_hal.h — Hardware Abstraction Layer for MTS-OLT-2000
 *
 * MTS OLT GPON (MTS-OLT-2000) — HAL для Tofino 2 + RTL960x
 */

#ifndef OLT_HAL_H
#define OLT_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum values */
#define OLT_MAX_GPON_PORTS 8
#define OLT_MAX_ONU_PER_PORT 128
#define OLT_MAX_ONU 1024
#define OLT_MAX_WDM_CHANNELS 4
#define OLT_MAX_OMCI_SESSIONS 1024
#define OLT_MAX_P4_TABLES 4096

/* GPON port status */
#define OLT_PORT_UP "UP"
#define OLT_PORT_DOWN "DOWN"
#define OLT_PORT_MAINTENANCE "MAINTENANCE"
#define OLT_PORT_FAULT "FAULT"

/* ONU status */
#define OLT_ONU_UNKNOWN "UNKNOWN"
#define OLT_ONU_DISCONNECTED "DISCONNECTED"
#define OLT_ONU_REGISTERED "REGISTERED"
#define OLT_ONU_OPERATIONAL "OPERATIONAL"
#define OLT_ONU_MAINTENANCE "MAINTENANCE"

/* Device health status */
#define OLT_HEALTH_HEALTHY "healthy"
#define OLT_HEALTH_DEGRADED "degraded"
#define OLT_HEALTH_CRITICAL "critical"

/* ==================== GPON ==================== */

typedef struct {
    uint32_t port_id;
    char name[32];
    uint32_t status;
    uint32_t mode;
    int32_t tx_power;
    int32_t rx_power;
    int32_t bias_current;
    int32_t temperature;
    uint32_t num_onu;
    uint32_t max_onu;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t tx_errors;
    uint64_t rx_errors;
    uint64_t tx_drops;
    uint64_t rx_drops;
} olt_gpon_port_t;

typedef struct {
    uint32_t onu_id;
    char mac[18];
    char serial[21];
    char vendor[33];
    uint32_t state;
    int32_t rx_power;
    int32_t distance_us;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
} olt_onu_info_t;

int olt_hal_gpon_init(void);
int olt_hal_gpon_exit(void);
int olt_hal_gpon_port_get(uint32_t port_id, olt_gpon_port_t *port);
int olt_hal_gpon_port_get_all(olt_gpon_port_t *ports, uint32_t *count);
int olt_hal_gpon_port_configure(uint32_t port_id, const olt_gpon_port_t *port);
int olt_hal_gpon_port_enable(uint32_t port_id);
int olt_hal_gpon_port_disable(uint32_t port_id);
int olt_hal_onu_get(uint32_t port_id, uint32_t onu_id, olt_onu_info_t *onu);
int olt_hal_onu_get_all(uint32_t port_id, olt_onu_info_t *onus, uint32_t *count);
int olt_hal_onu_enable(uint32_t port_id, uint32_t onu_id);
int olt_hal_onu_disable(uint32_t port_id, uint32_t onu_id);

/* ==================== OMCI ==================== */

typedef struct {
    uint32_t onu_id;
    uint16_t mib_version;
    uint32_t entity_instance;
    uint32_t oid;
    uint32_t mib_type;
    uint64_t last_updated;
    uint8_t data[256];
    uint32_t data_len;
} olt_omci_mib_entry_t;

typedef struct {
    uint32_t event_id;
    uint32_t onu_id;
    uint32_t entity_instance;
    uint32_t oid;
    uint32_t event_type;
    int32_t severity;
    int64_t timestamp;
    char description[128];
} olt_omci_event_t;

int olt_hal_omci_init(void);
int olt_hal_omci_exit(void);
int olt_hal_omci_get_mib(uint32_t onu_id, uint32_t oid, olt_omci_mib_entry_t *mib);
int olt_hal_omci_set_mib(uint32_t onu_id, const olt_omci_mib_entry_t *mib);
int olt_hal_omci_get_events(olt_omci_event_t *events, uint32_t *count, uint32_t max_events);
int olt_hal_omci_send_message(uint32_t onu_id, const uint8_t *msg, uint32_t msg_len);

/* ==================== WDM ==================== */

typedef struct {
    uint32_t channel_id;
    uint32_t wavelength;
    int32_t tx_power;
    int32_t rx_power;
    uint32_t status;
    int32_t temperature;
    int32_t bias_current;
    uint32_t tx_fault;
    uint32_t rx_loss;
    int32_t target_tx_power;
    int32_t agc_range_min;
    int32_t agc_range_max;
} olt_wdm_channel_t;

int olt_hal_wdm_init(void);
int olt_hal_wdm_exit(void);
int olt_hal_wdm_get(uint32_t channel_id, olt_wdm_channel_t *channel);
int olt_hal_wdm_get_all(olt_wdm_channel_t *channels, uint32_t *count);
int olt_hal_wdm_set(uint32_t channel_id, const olt_wdm_channel_t *channel);
int olt_hal_wdm_enable(uint32_t channel_id);
int olt_hal_wdm_disable(uint32_t channel_id);

/* ==================== TR-069 ==================== */

typedef struct {
    char status[16];
    char acs_url[256];
    uint32_t acs_port;
    uint32_t connection_requests;
    uint32_t max_connections;
    uint32_t active_sessions;
    int64_t last_connection;
    int64_t next_connection;
} olt_tr069_status_t;

typedef struct {
    char firmware_url[512];
    char file_name[128];
    uint64_t file_size;
    char file_type[32];
    char version[64];
    char status[16];
    uint64_t progress_pct;
    int64_t started_at;
    int64_t completed_at;
} olt_firmware_status_t;

int olt_hal_tr069_init(void);
int olt_hal_tr069_exit(void);
int olt_hal_tr069_get_status(olt_tr069_status_t *status);
int olt_hal_tr069_set_config(const char *acs_url, uint32_t port,
                               const char *username, const char *password);
int olt_hal_firmware_get_status(olt_firmware_status_t *status);
int olt_hal_firmware_trigger_download(const char *url, const char *file_type);

/* ==================== P4 Pipeline ==================== */

typedef struct {
    char pipeline_id[64];
    char program_name[128];
    char version[32];
    char status[16];
    uint64_t compiled_at;
    uint64_t loaded_at;
    uint32_t num_tables;
    uint64_t num_entries;
} olt_p4_pipeline_t;

int olt_hal_p4_get_status(olt_p4_pipeline_t *pipeline);
int olt_hal_p4_load(const char *pipeline_file);
int olt_hal_p4_get_stats(uint64_t *tables, uint64_t *entries);

/* ==================== Health ==================== */

typedef struct {
    char device_id[64];
    char model[32];
    char firmware[32];
    double cpu_usage;
    double memory_usage;
    double temperature;
    char status[16];
    uint64_t uptime_seconds;
} olt_device_health_t;

int olt_hal_health_get(olt_device_health_t *health);

#ifdef __cplusplus
}
#endif

#endif /* OLT_HAL_H */
