// SPDX-License-Identifier: GPL-2.0
//
// s32g3_sync.c - S32G3 SyncE Implementation
//
// MTS-MB-3000 Mobile Backhaul S32G3 Driver

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/mutex.h>

#include "s32g3.h"
#include "s32g3_sync.h"

#define DRIVER_NAME "s32g3_sync"
#define SYNC_MAGIC 0x53594E43  /* "SYNC" */

struct s32g3_sync_priv {
    uint32_t magic;
    struct s32g3_sync_port ports[S32G3_SYNC_MAX_PORTS];
    uint32_t num_ports;
    uint32_t selected_port;
    uint32_t enabled;
    struct mutex lock;
};

static struct s32g3_sync_priv sync_priv;

int s32g3_sync_port_set_mode(uint32_t port_id, enum s32g3_sync_mode mode)
{
    if (port_id >= S32G3_SYNC_MAX_PORTS)
        return -EINVAL;
    
    mutex_lock(&sync_priv.lock);
    
    sync_priv.ports[port_id].mode = mode;
    
    mutex_unlock(&sync_priv.lock);
    return 0;
}

int s32g3_sync_port_get_mode(uint32_t port_id, enum s32g3_sync_mode *mode)
{
    if (port_id >= S32G3_SYNC_MAX_PORTS || !mode)
        return -EINVAL;
    
    mutex_lock(&sync_priv.lock);
    
    *mode = sync_priv.ports[port_id].mode;
    
    mutex_unlock(&sync_priv.lock);
    return 0;
}

int s32g3_sync_port_get_frequency(uint32_t port_id, uint32_t *freq)
{
    if (port_id >= S32G3_SYNC_MAX_PORTS || !freq)
        return -EINVAL;
    
    mutex_lock(&sync_priv.lock);
    
    *freq = sync_priv.ports[port_id].frequency;
    
    mutex_unlock(&sync_priv.lock);
    return 0;
}

int s32g3_sync_select_port(uint32_t port_id)
{
    if (port_id >= S32G3_SYNC_MAX_PORTS)
        return -EINVAL;
    
    mutex_lock(&sync_priv.lock);
    
    sync_priv.selected_port = port_id;
    
    mutex_unlock(&sync_priv.lock);
    return 0;
}

int s32g3_sync_get_selected_port(uint32_t *port_id)
{
    if (!port_id)
        return -EINVAL;
    
    mutex_lock(&sync_priv.lock);
    
    *port_id = sync_priv.selected_port;
    
    mutex_unlock(&sync_priv.lock);
    return 0;
}

int s32g3_sync_init(void)
{
    memset(&sync_priv, 0, sizeof(sync_priv));
    sync_priv.magic = SYNC_MAGIC;
    mutex_init(&sync_priv.lock);
    sync_priv.num_ports = 0;
    sync_priv.selected_port = 0;
    sync_priv.enabled = 0;
    
    pr_info("s32g3_sync: SyncE subsystem initialized\n");
    return 0;
}

void s32g3_sync_exit(void)
{
    if (sync_priv.magic == SYNC_MAGIC) {
        pr_info("s32g3_sync: SyncE subsystem exited\n");
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Firmware Agent");
MODULE_DESCRIPTION("S32G3 SyncE Subsystem");
MODULE_VERSION("1.0.0");
