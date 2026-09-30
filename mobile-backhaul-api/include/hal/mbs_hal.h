/*
 * mbs_hal.h — Hardware Abstraction Layer for MTS-MB-3000
 *
 * MTS Mobile Backhaul (MTS-MB-3000) — HAL для NXP S32G3
 */

#ifndef MBS_HAL_H
#define MBS_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum values */
#define MBS_MAX_PORTS 8
#define MBS_MAX_TUNNELS 256
#define MBS_MAX_PW 128
#define MBS_MAX_PTP_CLOCKS 4
#define MBS_MAX_DPDK_PORTS 16

/* Port types */
#define MBS_PORT_SFP "sfp"
#define MBS_PORT_SFP_PLUS "sfp_plus"
#define MBS_PORT_QSFP "qsfp"
#define MBS_PORT_ETHERNET "ethernet"

/* Port modes */
#define MBS_PORT_MODE_MPLS_TP "mpls-tp"
#define MBS_PORT_MODE_ETHERNET "ethernet"
#define MBS_PORT_MODE_HYBRID "hybrid"

/* PTP clock types */
#define MBS_PTP_CLOCK_MASTER "master"
#define MBS_PTP_CLOCK_SLAVE "slave"
#define MBS_PTP_CLOCK_BOUNDARY "boundary"
#define MBS_PTP_CLOCK_TRANSPARENT "transparent"

/* Device health status */
#define MBS_HEALTH_HEALTHY "healthy"
#define MBS_HEALTH_DEGRADED "degraded"
#define MBS_HEALTH_CRITICAL "critical"

/* MPLS-TP tunnel status */
#define MBS_TUNNEL_ACTIVE "active"
#define MBS_TUNNEL_INACTIVE "inactive"
#define MBS_TUNNEL_ERROR "error"

/* Pseudowire status */
#define MBS_PW_UP "up"
#define MBS_PW_DOWN "down"
#define MBS_PW_INITIALIZING "initializing"

/* OAM types */
#define MBS_OAM_CIRCUITRY "circuity"
#define MBS_OAM_ENDPOINT "endpoint"
#define MBS_OAM_BGW "bgw"

/* PTP profiles */
#define MBS_PTP_PROFILE_G8265 "G.8265.1"
#define MBS_PTP_PROFILE_G8275 "G.8275.1"
#define MBS_PTP_PROFILE_E812 "E.812"

/* ==================== Port ==================== */

typedef struct {
    uint32_t port_id;
    char name[32];
    char type[16];
    uint32_t speed_mbps;
    char status[16];
    char mode[16];
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_drops;
    uint64_t tx_drops;
    double rx_power_dbm;
    double tx_power_dbm;
    double temperature_c;
} mbs_port_status_t;

int mbs_hal_port_init(void);
int mbs_hal_port_exit(void);
int mbs_hal_port_get_status(uint32_t port_id, mbs_port_status_t *status);
int mbs_hal_port_get_all(mbs_port_status_t *ports, uint32_t *count);
int mbs_hal_port_set_mode(uint32_t port_id, const char *mode);

/* ==================== MPLS-TP ==================== */

typedef struct {
    uint32_t tunnel_id;
    char name[64];
    char ingress_port[32];
    char egress_port[32];
    uint32_t label;
    uint32_t next_label;
    char next_hop[64];
    char status[16];
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_drops;
    uint64_t tx_drops;
} mbs_tunnel_info_t;

typedef struct {
    uint32_t pw_id;
    char name[64];
    char peer_ip[64];
    uint32_t local_label;
    uint32_t remote_label;
    char encapsulation[16];
    char status[16];
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} mbs_pw_info_t;

typedef struct {
    uint32_t oam_id;
    char type[16];
    char target[64];
    char status[16];
    uint64_t failures;
    uint64_t intervals;
    double failure_rate;
} mbs_oam_info_t;

int mbs_hal_mpls_init(void);
int mbs_hal_mpls_exit(void);
int mbs_hal_tunnel_get(uint32_t tunnel_id, mbs_tunnel_info_t *tunnel);
int mbs_hal_tunnel_get_all(mbs_tunnel_info_t *tunnels, uint32_t *count);
int mbs_hal_tunnel_create(const mbs_tunnel_info_t *tunnel);
int mbs_hal_tunnel_delete(uint32_t tunnel_id);
int mbs_hal_pw_get(uint32_t pw_id, mbs_pw_info_t *pw);
int mbs_hal_pw_get_all(mbs_pw_info_t *pws, uint32_t *count);
int mbs_hal_pw_create(const mbs_pw_info_t *pw);
int mbs_hal_pw_delete(uint32_t pw_id);
int mbs_hal_oam_get(uint32_t oam_id, mbs_oam_info_t *oam);
int mbs_hal_oam_start(uint32_t oam_id, const char *type, const char *target, uint32_t interval_ms);
int mbs_hal_oam_stop(uint32_t oam_id);

/* ==================== PTP ==================== */

typedef struct {
    uint32_t clock_id;
    char clock_identity[64];
    char domain[32];
    char priority1[16];
    char priority2[16];
    char class_value[16];
    char clock_type[16];
    char status[16];
    double offset_from_master_ns;
    uint64_t num_steps_removed;
    uint64_t log_sync_interval;
} mbs_ptp_clock_t;

typedef struct {
    char profile_name[32];
    uint32_t domain_number;
    uint32_t priority1;
    uint32_t priority2;
    uint32_t max_steps;
    uint32_t clock_class;
    uint32_t clock_type;
    uint32_t utc_offset;
} mbs_ptp_profile_t;

int mbs_hal_ptp_init(void);
int mbs_hal_ptp_exit(void);
int mbs_hal_ptp_get_clocks(mbs_ptp_clock_t *clocks, uint32_t *count);
int mbs_hal_ptp_get_profile(mbs_ptp_profile_t *profile);
int mbs_hal_ptp_set_profile(const mbs_ptp_profile_t *profile);

/* ==================== DPDK ==================== */

typedef struct {
    uint32_t port_id;
    uint64_t packets_processed;
    uint64_t packets_dropped;
    uint64_t bytes_processed;
    uint64_t errors;
    uint64_t bursts;
    double throughput_mbps;
    double cpu_usage;
} mbs_dpdk_stats_t;

int mbs_hal_dpdk_init(void);
int mbs_hal_dpdk_exit(void);
int mbs_hal_dpdk_get_stats(uint32_t port_id, mbs_dpdk_stats_t *stats);
int mbs_hal_dpdk_get_all(mbs_dpdk_stats_t *stats, uint32_t *count);

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
    uint64_t packet_loss_pct;
    double latency_ms;
} mbs_device_health_t;

int mbs_hal_health_get(mbs_device_health_t *health);

#ifdef __cplusplus
}
#endif

#endif /* MBS_HAL_H */
