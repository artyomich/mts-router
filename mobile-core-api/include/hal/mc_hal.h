/*
 * mc_hal.h — Hardware Abstraction Layer for MTS-MC-5000
 *
 * MTS Mobile Core (MTS-MC-5000) — HAL для ThunderX3
 */

#ifndef MC_HAL_H
#define MC_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum values */
#define MC_MAX_UPF_SESSIONS 1048576
#define MC_MAX_SMF_SESSIONS 262144
#define MC_MAX_PFCP_SESSIONS 524288
#define MC_MAX_GTP_TUNNELS 131072
#define MC_MAX_NRF_ENTRIES 64
#define MC_MAX_5QI 64
#define MC_MAX_PORTS 8

/* UPF status */
#define MC_UPF_ACTIVE "active"
#define MC_UPF_INACTIVE "inactive"
#define MC_UPF_ERROR "error"

/* PDU session status */
#define MC_SESSION_ACTIVE "active"
#define MC_SESSION_INACTIVE "inactive"
#define MC_SESSION_RELEASING "releasing"

/* PFCP session status */
#define MC_PFCP_ESTABLISHED "established"
#define MC_PFCP_INACTIVE "inactive"

/* GTP tunnel status */
#define MC_GTP_ACTIVE "active"
#define MC_GTP_INACTIVE "inactive"

/* Device health status */
#define MC_HEALTH_HEALTHY "healthy"
#define MC_HEALTH_DEGRADED "degraded"
#define MC_HEALTH_CRITICAL "critical"

/* ==================== UPF ==================== */

typedef struct {
    char upf_id[64];
    char status[16];
    uint32_t active_sessions;
    uint32_t max_sessions;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    double cpu_usage;
    double memory_usage;
    int64_t last_updated;
} mc_upf_status_t;

typedef struct {
    char session_id[64];
    char ue_ip[64];
    char upf_ip[64];
    uint32_t teid;
    uint32_t qfi;
    uint32_t five_qi;
    char pnni[32];
    char status[16];
    int64_t created_at;
    int64_t last_active;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} mc_pdu_session_t;

int mc_hal_upf_init(void);
int mc_hal_upf_exit(void);
int mc_hal_upf_get_status(mc_upf_status_t *status);
int mc_hal_upf_configure(uint32_t max_sessions);
int mc_hal_pdu_session_get(const char *session_id, mc_pdu_session_t *session);
int mc_hal_pdu_session_get_all(mc_pdu_session_t *sessions, uint32_t *count);
int mc_hal_pdu_session_create(mc_pdu_session_t *session);
int mc_hal_pdu_session_delete(const char *session_id);

/* ==================== SMF ==================== */

typedef struct {
    char smf_id[64];
    char status[16];
    uint32_t active_sessions;
    uint32_t max_sessions;
    double cpu_usage;
    double memory_usage;
} mc_smf_status_t;

typedef struct {
    char nf_id[64];
    char nf_type[16];
    char status[16];
    char uri[256];
    char supported_nfs[1024];
    uint32_t priority;
    uint32_t capacity;
} mc_nrf_entry_t;

int mc_hal_smf_init(void);
int mc_hal_smf_exit(void);
int mc_hal_smf_get_status(mc_smf_status_t *status);
int mc_hal_smf_get_nrf_entries(mc_nrf_entry_t *entries, uint32_t *count);
int mc_hal_smf_register_nrf(const mc_nrf_entry_t *entry);
int mc_hal_smf_deregister_nrf(const char *nf_id);

/* ==================== PFCP ==================== */

typedef struct {
    char session_id[64];
    char f_seid[64];
    char peer_ip[64];
    char type[16];
    char status[16];
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} mc_pfcp_session_t;

typedef struct {
    uint32_t rule_id;
    char description[128];
    char action[16];
    uint32_t qos_index;
} mc_pfcp_rule_t;

int mc_hal_pfcp_init(void);
int mc_hal_pfcp_exit(void);
int mc_hal_pfcp_session_get(const char *session_id, mc_pfcp_session_t *session);
int mc_hal_pfcp_session_get_all(mc_pfcp_session_t *sessions, uint32_t *count);
int mc_hal_pfcp_session_create(const mc_pfcp_session_t *session);
int mc_hal_pfcp_session_delete(const char *session_id);

/* ==================== GTP ==================== */

typedef struct {
    char tunnel_id[64];
    char local_ip[64];
    char remote_ip[64];
    uint32_t local_teid;
    uint32_t remote_teid;
    char type[8];
    char status[16];
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} mc_gtp_tunnel_t;

int mc_hal_gtp_init(void);
int mc_hal_gtp_exit(void);
int mc_hal_gtp_tunnel_get(const char *tunnel_id, mc_gtp_tunnel_t *tunnel);
int mc_hal_gtp_tunnel_get_all(mc_gtp_tunnel_t *tunnels, uint32_t *count);
int mc_hal_gtp_tunnel_create(const mc_gtp_tunnel_t *tunnel);
int mc_hal_gtp_tunnel_delete(const char *tunnel_id);

/* ==================== 5QI ==================== */

typedef struct {
    uint32_t five_qi;
    char name[32];
    char resource_type[32];
    uint32_t priority_level;
    uint32_t packet_delay_budget_ms;
    uint32_t max_data_burst_size;
    double error_rate;
} mc_5qi_config_t;

int mc_hal_5qi_get(uint32_t five_qi, mc_5qi_config_t *config);
int mc_hal_5qi_set(const mc_5qi_config_t *config);
int mc_hal_5qi_get_all(mc_5qi_config_t *configs, uint32_t *count);

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
} mc_device_health_t;

int mc_hal_health_get(mc_device_health_t *health);

#ifdef __cplusplus
}
#endif

#endif /* MC_HAL_H */
