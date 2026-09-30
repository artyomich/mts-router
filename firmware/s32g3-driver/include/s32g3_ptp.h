/* SPDX-License-Identifier: GPL-2.0 */
/*
 * s32g3_ptp.h - NXP S32G3 PTP (Precision Time Protocol) Interface
 *
 * MTS-MB-3000 Mobile Backhaul S32G3 Driver
 */

#ifndef S32G3_PTP_H
#define S32G3_PTP_H

#include <linux/types.h>

#define S32G3_PTP_MAX_PORTS 8
#define S32G3_PTP_MAX_DOMAINS 32
#define S32G3_PTP_NAME_LEN 32
#define S32G3_PTP_CLOCK_NAME_LEN 64

enum s32g3_ptp_mode {
    S32G3_PTP_MODE_GRANDMASTER,
    S32G3_PTP_MODE_BOUNDARY,
    S32G3_PTP_MODE_ordinary,
    S32G3_PTP_MODE_SLAVE,
    S32G3_PTP_MODE_UNKNOWN
};

enum s32g3_ptp_transport {
    S32G3_PTP_TRANSPORT_L2,
    S32G3_PTP_TRANSPORT_UDP_IPv4,
    S32G3_PTP_TRANSPORT_UDP_IPv6,
    S32G3_PTP_TRANSPORT_ETH
};

struct s32g3_ptp_clock {
    uint32_t id;
    char name[S32G3_PTP_CLOCK_NAME_LEN];
    enum s32g3_ptp_mode mode;
    uint32_t domain;
    uint32_t port_count;
    uint64_t current_time_ns;
    uint64_t last_sync_time_ns;
    uint64_t last_follow_up_time_ns;
    uint64_t last_delay_req_time_ns;
    uint64_t last_delay_resp_time_ns;
    uint64_t grandmaster_priority1;
    uint64_t grandmaster_priority2;
    uint64_t grandmaster_clock_quality;
    uint8_t grandmaster_clock_class;
    uint8_t grandmaster_clock_accuracy;
    uint16_t grandmaster_clock_variance;
    uint64_t grandmaster_time_source;
    uint32_t steps_removed;
    uint32_t time_source;
    uint8_t enabled;
    uint8_t pad[3];
};

struct s32g3_ptp_port {
    uint32_t id;
    uint32_t clock_id;
    char name[S32G3_PTP_NAME_LEN];
    uint32_t port_id;
    enum s32g3_ptp_mode role;
    uint32_t state;
    uint32_t parent_port_id;
    uint64_t parent_time_ns;
    uint64_t offset_from_parent_ns;
    uint64_t mean_path_delay_ns;
    uint32_t log_sync_interval;
    uint32_t log_min_delay_req_interval;
    uint32_t log_announce_interval;
    uint32_t announce_receipt_timeout;
    uint32_t priority1;
    uint8_t priority2;
    uint8_t domain;
    uint8_t number_steps_supported;
    uint8_t clock_class;
    uint8_t accuracy_ns;
    uint8_t time_source;
    uint8_t enabled;
    uint8_t pad[3];
};

struct s32g3_ptp_device {
    uint32_t id;
    char name[S32G3_PTP_CLOCK_NAME_LEN];
    uint32_t num_clocks;
    struct s32g3_ptp_clock *clocks;
    uint32_t num_ports;
    struct s32g3_ptp_port *ports;
    uint64_t time_ns;
    uint64_t freq_ppb;
    uint32_t enabled;
    void *priv;
    struct device *dev;
    struct mutex lock;
};

/* API */
int s32g3_ptp_init(void);
void s32g3_ptp_exit(void);
int s32g3_ptp_clock_enable(uint32_t clock_id);
int s32g3_ptp_clock_disable(uint32_t clock_id);
int s32g3_ptp_clock_set_mode(uint32_t clock_id, enum s32g3_ptp_mode mode);
int s32g3_ptp_clock_get_mode(uint32_t clock_id, enum s32g3_ptp_mode *mode);
int s32g3_ptp_clock_get_time(uint32_t clock_id, uint64_t *time_ns);
int s32g3_ptp_clock_set_time(uint32_t clock_id, uint64_t time_ns);
int s32g3_ptp_clock_adj_freq(uint32_t clock_id, int32_t ppb);
int s32g3_ptp_clock_get_grandmaster(uint32_t clock_id,
                                      uint64_t *gm_id,
                                      uint8_t *gm_class);
int s32g3_ptp_port_set_state(uint32_t port_id, uint32_t state);
int s32g3_ptp_port_get_state(uint32_t port_id, uint32_t *state);
int s32g3_ptp_port_get_offset(uint32_t port_id, int64_t *offset_ns);
int s32g3_ptp_port_get_delay(uint32_t port_id, uint64_t *delay_ns);

#endif /* S32G3_PTP_H */
