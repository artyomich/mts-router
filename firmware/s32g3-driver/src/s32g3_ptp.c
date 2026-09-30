// SPDX-License-Identifier: GPL-2.0
//
// s32g3_ptp.c - S32G3 PTP (Precision Time Protocol) Implementation
//
// MTS-MB-3000 Mobile Backhaul S32G3 Driver

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ktime.h>
#include <linux/clocksource.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/mutex.h>

#include "s32g3.h"
#include "s32g3_ptp.h"

#define DRIVER_NAME "s32g3_ptp"
#define PTP_MAGIC 0x50545033  /* "PTP3" */

struct s32g3_ptp_priv {
    uint32_t magic;
    struct s32g3_ptp_clock clocks[8];
    struct s32g3_ptp_port ports[32];
    uint32_t num_clocks;
    uint32_t num_ports;
    uint64_t time_ns;
    int64_t offset_ns;
    int32_t freq_ppb;
    struct mutex lock;
    uint32_t enabled;
    struct ptp_clock *ptp_clock;
    struct ptp_clock_info ptp_info;
};

static struct s32g3_ptp_priv ptp_priv;

int s32g3_ptp_clock_enable(uint32_t clock_id)
{
    if (clock_id >= 8)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    if (ptp_priv.clocks[clock_id].mode == S32G3_PTP_MODE_UNKNOWN) {
        mutex_unlock(&ptp_priv.lock);
        return -EINVAL;
    }
    
    ptp_priv.clocks[clock_id].enabled = 1;
    ptp_priv.enabled = 1;
    
    mutex_unlock(&ptp_priv.lock);
    
    pr_info("s32g3_ptp: Clock %d enabled\n", clock_id);
    return 0;
}

int s32g3_ptp_clock_disable(uint32_t clock_id)
{
    if (clock_id >= 8)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    ptp_priv.clocks[clock_id].enabled = 0;
    
    uint32_t any_enabled = 0;
    for (uint32_t i = 0; i < ptp_priv.num_clocks; i++) {
        if (ptp_priv.clocks[i].enabled) {
            any_enabled = 1;
            break;
        }
    }
    
    if (!any_enabled)
        ptp_priv.enabled = 0;
    
    mutex_unlock(&ptp_priv.lock);
    
    pr_info("s32g3_ptp: Clock %d disabled\n", clock_id);
    return 0;
}

int s32g3_ptp_clock_set_mode(uint32_t clock_id, enum s32g3_ptp_mode mode)
{
    if (clock_id >= 8)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    ptp_priv.clocks[clock_id].mode = mode;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_clock_get_mode(uint32_t clock_id, enum s32g3_ptp_mode *mode)
{
    if (clock_id >= 8 || !mode)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    *mode = ptp_priv.clocks[clock_id].mode;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_clock_get_time(uint32_t clock_id, uint64_t *time_ns)
{
    if (clock_id >= 8 || !time_ns)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    *time_ns = ktime_get_ns();
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_clock_set_time(uint32_t clock_id, uint64_t time_ns)
{
    if (clock_id >= 8)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    ptp_priv.clocks[clock_id].current_time_ns = time_ns;
    ptp_priv.time_ns = time_ns;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_clock_adj_freq(uint32_t clock_id, int32_t ppb)
{
    if (clock_id >= 8)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    ptp_priv.clocks[clock_id].freq_ppb = ppb;
    ptp_priv.freq_ppb = ppb;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_clock_get_grandmaster(uint32_t clock_id,
                                      uint64_t *gm_id,
                                      uint8_t *gm_class)
{
    if (clock_id >= 8)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    if (gm_id)
        *gm_id = ptp_priv.clocks[clock_id].grandmaster_priority1;
    if (gm_class)
        *gm_class = ptp_priv.clocks[clock_id].grandmaster_clock_class;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_port_set_state(uint32_t port_id, uint32_t state)
{
    if (port_id >= 32)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    ptp_priv.ports[port_id].state = state;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_port_get_state(uint32_t port_id, uint32_t *state)
{
    if (port_id >= 32 || !state)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    *state = ptp_priv.ports[port_id].state;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_port_get_offset(uint32_t port_id, int64_t *offset_ns)
{
    if (port_id >= 32 || !offset_ns)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    *offset_ns = ptp_priv.offset_ns;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_port_get_delay(uint32_t port_id, uint64_t *delay_ns)
{
    if (port_id >= 32 || !delay_ns)
        return -EINVAL;
    
    mutex_lock(&ptp_priv.lock);
    
    *delay_ns = ptp_priv.ports[port_id].mean_path_delay_ns;
    
    mutex_unlock(&ptp_priv.lock);
    return 0;
}

int s32g3_ptp_init(void)
{
    memset(&ptp_priv, 0, sizeof(ptp_priv));
    ptp_priv.magic = PTP_MAGIC;
    mutex_init(&ptp_priv.lock);
    ptp_priv.num_clocks = 0;
    ptp_priv.num_ports = 0;
    ptp_priv.time_ns = ktime_get_ns();
    ptp_priv.offset_ns = 0;
    ptp_priv.freq_ppb = 0;
    ptp_priv.enabled = 0;
    ptp_priv.ptp_clock = NULL;
    
    pr_info("s32g3_ptp: PTP subsystem initialized\n");
    return 0;
}

void s32g3_ptp_exit(void)
{
    if (ptp_priv.magic == PTP_MAGIC) {
        pr_info("s32g3_ptp: PTP subsystem exited\n");
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Firmware Agent");
MODULE_DESCRIPTION("S32G3 PTP Subsystem");
MODULE_VERSION("1.0.0");
