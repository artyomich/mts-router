/* SPDX-License-Identifier: GPL-2.0 */
/*
 * s32g3_sync.h - NXP S32G3 SyncE Interface
 *
 * MTS-MB-3000 Mobile Backhaul S32G3 Driver
 */

#ifndef S32G3_SYNC_H
#define S32G3_SYNC_H

#include <linux/types.h>

#define S32G3_SYNC_MAX_PORTS 8
#define S32G3_SYNC_NAME_LEN 32

enum s32g3_sync_mode {
    S32G3_SYNC_MODE_MASTER,
    S32G3_SYNC_MODE_SLAVE,
    S32G3_SYNC_MODE_INTERNAL_OSCILLATOR,
    S32G3_SYNC_MODE_HOLDOVER,
    S32G3_SYNC_MODE_UNKNOWN
};

struct s32g3_sync_port {
    uint32_t id;
    char name[S32G3_SYNC_NAME_LEN];
    enum s32g3_sync_mode mode;
    uint32_t frequency;
    uint32_t accuracy;
    uint32_t holdover;
    uint32_t holdover_accuracy;
    uint32_t holdover_duration;
    uint8_t enabled;
    uint8_t pad[3];
};

struct s32g3_sync_device {
    uint32_t id;
    char name[32];
    uint32_t num_ports;
    struct s32g3_sync_port *ports;
    uint32_t selected_port;
    uint32_t enabled;
    void *priv;
    struct device *dev;
    struct mutex lock;
};

/* API */
int s32g3_sync_init(void);
void s32g3_sync_exit(void);
int s32g3_sync_port_set_mode(uint32_t port_id, enum s32g3_sync_mode mode);
int s32g3_sync_port_get_mode(uint32_t port_id, enum s32g3_sync_mode *mode);
int s32g3_sync_port_get_frequency(uint32_t port_id, uint32_t *freq);
int s32g3_sync_select_port(uint32_t port_id);
int s32g3_sync_get_selected_port(uint32_t *port_id);

#endif /* S32G3_SYNC_H */
