/*
 * mbs_ptp.h — PTP (Precision Time Protocol) HAL for MTS-MB-3000
 *
 * MTS Mobile Backhaul — PTP Grandmaster Engine HAL
 */

#ifndef MBS_PTP_H
#define MBS_PTP_H

#include "mbs_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PTP domain ranges */
#define MBS_PTP_MIN_DOMAIN 0
#define MBS_PTP_MAX_DOMAIN 255

/* PTP clock class values */
#define MBS_PTP_CLOCK_CLASS_DEFAULT 128
#define MBS_PTP_CLOCK_CLASS_68 0
#define MBS_PTP_CLOCK_CLASS_71 248

/* PTP priority values */
#define MBS_PTP_PRIORITY_DEFAULT 128

/* PTP sync interval (log seconds) */
#define MBS_PTP_SYNC_INTERVAL_MIN -30  /* 2^-30 ~ 1e-9s */
#define MBS_PTP_SYNC_INTERVAL_MAX 14   /* 2^14 = 16384s */

/* PTP profile types */
typedef enum {
    MBS_PTP_PROFILE_G8265_1 = 0,    /* G.8265.1 - PTP/ACE profile */
    MBS_PTP_PROFILE_G8275_1 = 1,    /* G.8275.1 - TSN profile */
    MBS_PTP_PROFILE_E812 = 2,       /* ITU-T E.812 - Telecom profile */
    MBS_PTP_PROFILE_DEFAULT = 3     /* Default profile */
} mbs_ptp_profile_t;

/* ==================== PTP Engine Operations ==================== */

/* Initialize PTP engine */
int mbs_ptp_engine_init(void);

/* Shutdown PTP engine */
int mbs_ptp_engine_exit(void);

/* Set PTP as grandmaster */
int mbs_ptp_set_grandmaster(uint32_t domain,
                             const char *clock_identity,
                             uint8_t priority1,
                             uint8_t priority2,
                             uint8_t clock_class,
                             uint8_t clock_type);

/* Set PTP as boundary clock */
int mbs_ptp_set_boundary_clock(uint32_t domain,
                                const char *clock_identity,
                                const char *ref_port,
                                uint8_t priority1,
                                uint8_t priority2);

/* Configure PTP profile */
int mbs_ptp_set_profile(mbs_ptp_profile_t profile);

/* Get PTP profile */
int mbs_ptp_get_profile(mbs_ptp_profile_t *profile);

/* ==================== PTP Clock Operations ==================== */

/* Get PTP clock status */
int mbs_ptp_get_clock_status(uint32_t clock_id,
                              mbs_ptp_clock_t *clock);

/* Get all PTP clocks */
int mbs_ptp_get_all_clocks(mbs_ptp_clock_t *clocks,
                            uint32_t *count);

/* Set PTP sync interval */
int mbs_ptp_set_sync_interval(uint32_t domain,
                               int8_t log_interval);

/* Get PTP sync interval */
int mbs_ptp_get_sync_interval(uint32_t domain,
                               int8_t *log_interval);

/* ==================== PTP Time Operations ==================== */

/* Get current PTP time */
int mbs_ptp_get_time(int64_t *seconds,
                      int32_t *nanoseconds);

/* Set PTP time (for grandmaster) */
int mbs_ptp_set_time(int64_t seconds,
                      int32_t nanoseconds);

/* Get PTP offset from master */
int mbs_ptp_get_offset(int64_t *offset_ns);

/* ==================== PTP Events ==================== */

/* PTP event types */
#define MBS_PTP_EVENT_MASTER_SELECTED 1
#define MBS_PBS_PTP_EVENT_SLAVE_SELECTED 2
#define MBS_PTP_EVENT_TIME_SYNC 3
#define MBS_PTP_EVENT_DOMAIN_CHANGE 4
#define MBS_PTP_EVENT_PROFILE_CHANGE 5

/* PTP event callback */
typedef void (*mbs_ptp_event_cb)(uint32_t event, void *data, void *priv);

/* Register PTP event callback */
int mbs_ptp_register_event_cb(mbs_ptp_event_cb cb, void *priv);

/* Unregister PTP event callback */
int mbs_ptp_unregister_event_cb(void);

#ifdef __cplusplus
}
#endif

#endif /* MBS_PTP_H */
