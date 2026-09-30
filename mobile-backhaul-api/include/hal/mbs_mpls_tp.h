/*
 * mbs_mpls_tp.h — MPLS-TP HAL for MTS-MB-3000
 *
 * MTS Mobile Backhaul — MPLS-TP Engine HAL
 */

#ifndef MBS_MPLS_TP_H
#define MBS_MPLS_TP_H

#include "mbs_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MPLS-TP specific constants */
#define MBS_MPLS_TP_MAX_LABELS 1048575
#define MBS_MPLS_TP_MIN_LABEL 16
#define MBS_MPLS_TP_MAX_LABELS_PER_ENTRY 3
#define MBS_MPLS_TP_MAX_PW 1024
#define MBS_MPLS_TP_MAX_OAM 64

/* Encapsulation types */
#define MBS_MPLS_TP_ENC_ETHERNET "eth"
#define MBS_MPLS_TP_ENC_VLAN "vlan"
#define MBS_MPLS_TP_ENC_GTP "gtp"

/* OAM interval ranges */
#define MBS_MPLS_TP_OAM_MIN_INTERVAL_MS 1000
#define MBS_MPLS_TP_OAM_MAX_INTERVAL_MS 86400000

/* ==================== MPLS-TP Tunnel Operations ==================== */

/* Initialize MPLS-TP engine */
int mbs_mpls_tp_engine_init(void);

/* Shutdown MPLS-TP engine */
int mbs_mpls_tp_engine_exit(void);

/* Create MPLS-TP tunnel with labels */
int mbs_mpls_tp_tunnel_create(uint32_t *tunnel_id,
                               const char *name,
                               const char *ingress_port,
                               const char *egress_port,
                               uint32_t label,
                               uint32_t next_label,
                               const char *next_hop);

/* Delete MPLS-TP tunnel */
int mbs_mpls_tp_tunnel_delete(uint32_t tunnel_id);

/* Update MPLS-TP tunnel labels */
int mbs_mpls_tp_tunnel_update_labels(uint32_t tunnel_id,
                                       uint32_t label,
                                       uint32_t next_label);

/* Get tunnel statistics */
int mbs_mpls_tp_tunnel_get_stats(uint32_t tunnel_id,
                                  uint64_t *rx_packets,
                                  uint64_t *tx_packets,
                                  uint64_t *rx_bytes,
                                  uint64_t *tx_bytes,
                                  uint64_t *rx_errors,
                                  uint64_t *tx_errors);

/* ==================== Pseudowire Operations ==================== */

/* Create pseudowire */
int mbs_mpls_tp_pw_create(uint32_t *pw_id,
                           const char *name,
                           const char *peer_ip,
                           uint32_t local_label,
                           uint32_t remote_label,
                           const char *encapsulation);

/* Delete pseudowire */
int mbs_mpls_tp_pw_delete(uint32_t pw_id);

/* Update pseudowire labels */
int mbs_mpls_tp_pw_update_labels(uint32_t pw_id,
                                  uint32_t local_label,
                                  uint32_t remote_label);

/* Get pseudowire statistics */
int mbs_mpls_tp_pw_get_stats(uint32_t pw_id,
                              uint64_t *rx_packets,
                              uint64_t *tx_packets,
                              uint64_t *rx_bytes,
                              uint64_t *tx_bytes);

/* ==================== OAM Operations ==================== */

/* Start OAM monitoring */
int mbs_mpls_tp_oam_start(uint32_t *oam_id,
                           const char *type,
                           const char *target,
                           uint32_t interval_ms);

/* Stop OAM monitoring */
int mbs_mpls_tp_oam_stop(uint32_t oam_id);

/* Get OAM statistics */
int mbs_mpls_tp_oam_get_stats(uint32_t oam_id,
                               uint64_t *failures,
                               uint64_t *intervals,
                               double *failure_rate);

/* ==================== Forwarding Table ==================== */

/* Add forwarding entry */
int mbs_mpls_tp_fwd_add(uint32_t label,
                         const char *egress_port,
                         uint32_t action_label);

/* Remove forwarding entry */
int mbs_mpls_tp_fwd_remove(uint32_t label);

/* Get forwarding table size */
int mbs_mpls_tp_fwd_get_size(uint32_t *entries);

/* Clear forwarding table */
int mbs_mpls_tp_fwd_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* MBS_MPLS_TP_H */
