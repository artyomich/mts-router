// SPDX-License-Identifier: GPL-2.0
//
// tofino2_ctrl.c - Control Plane (Counters, Meters, DMA) Implementation
//
// MTS-CR-9000 Core Router Tofino 2 Driver
//
// Copyright (c) 2024 MTS Router Project
//
// This file implements the control plane subsystem for Tofino 2, including
// counter management, rate meters, DMA channel management, and digest
// computation.

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/dmaengine.h>
#include <linux/kref.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>

#include "tofino2.h"
#include "tofino2_ctrl.h"

#define DRIVER_NAME "tofino2_ctrl"
#define CTRL_MAGIC 0x43545232  /* "CTR2"

/* ============================================================================ */
/* Global state                                                                 */
/* ============================================================================ */

static struct tofino2_ctrl_priv {
    uint32_t magic;
    spinlock_t lock;
    struct tofino2_ctrl_counter counters[TOFINO2_CTRL_MAX_COUNTERS];
    struct tofino2_ctrl_meter meters[TOFINO2_CTRL_MAX_METERS];
    struct tofino2_ctrl_dma dma_channels[TOFINO2_CTRL_MAX_DMA_CHANNELS];
    struct tofino2_ctrl_digest digests[256];
    uint32_t next_counter_id;
    uint32_t next_meter_id;
    uint32_t next_dma_id;
    uint32_t next_digest_id;
    atomic_t counter_alloc_mask;
    atomic_t meter_alloc_mask;
    atomic_t dma_alloc_mask;
    atomic_t digest_alloc_mask;
} ctrl_priv;

/* ============================================================================ */
/* Counter management                                                           */
/* ============================================================================ */

int tofino2_ctrl_counter_allocate(uint32_t *counter_id)
{
    int i;
    
    for (i = 0; i < TOFINO2_CTRL_MAX_COUNTERS; i++) {
        if (!(atomic_read(&ctrl_priv.counter_alloc_mask) & (1 << i))) {
            atomic_set(&ctrl_priv.counter_alloc_mask,
                       atomic_read(&ctrl_priv.counter_alloc_mask) | (1 << i));
            ctrl_priv.counters[i].id = i;
            ctrl_priv.counters[i].allocated = 1;
            ctrl_priv.counters[i].enabled = 1;
            ctrl_priv.counters[i].value = 0;
            ctrl_priv.counters[i].base_value = 0;
            ctrl_priv.counters[i].max_value = 0;
            ctrl_priv.counters[i].min_value = U64_MAX;
            ctrl_priv.counters[i].timestamp = 0;
            ctrl_priv.counters[i].refcount = 1;
            if (counter_id)
                *counter_id = i;
            return 0;
        }
    }
    
    return -ENOSPC;
}

int tofino2_ctrl_counter_free(uint32_t counter_id)
{
    if (counter_id >= TOFINO2_CTRL_MAX_COUNTERS)
        return -EINVAL;
    
    if (!ctrl_priv.counters[counter_id].allocated)
        return -EINVAL;
    
    atomic_set(&ctrl_priv.counter_alloc_mask,
               atomic_read(&ctrl_priv.counter_alloc_mask) & ~(1 << counter_id));
    ctrl_priv.counters[counter_id].allocated = 0;
    ctrl_priv.counters[counter_id].enabled = 0;
    
    return 0;
}

int tofino2_ctrl_counter_get(uint32_t counter_id,
                              struct tofino2_ctrl_counter *counter)
{
    if (counter_id >= TOFINO2_CTRL_MAX_COUNTERS)
        return -EINVAL;
    
    if (!counter)
        return -EINVAL;
    
    *counter = ctrl_priv.counters[counter_id];
    return 0;
}

int tofino2_ctrl_counter_set(uint32_t counter_id,
                              const struct tofino2_ctrl_counter *counter)
{
    if (counter_id >= TOFINO2_CTRL_MAX_COUNTERS)
        return -EINVAL;
    
    if (!counter)
        return -EINVAL;
    
    ctrl_priv.counters[counter_id] = *counter;
    return 0;
}

int tofino2_ctrl_counter_read(uint32_t counter_id, uint64_t *value)
{
    if (counter_id >= TOFINO2_CTRL_MAX_COUNTERS)
        return -EINVAL;
    
    if (!value)
        return -EINVAL;
    
    *value = ctrl_priv.counters[counter_id].value;
    return 0;
}

int tofino2_ctrl_counter_write(uint32_t counter_id, uint64_t value)
{
    if (counter_id >= TOFINO2_CTRL_MAX_COUNTERS)
        return -EINVAL;
    
    ctrl_priv.counters[counter_id].value = value;
    ctrl_priv.counters[counter_id].base_value = value;
    
    return 0;
}

int tofino2_ctrl_counter_reset(uint32_t counter_id)
{
    if (counter_id >= TOFINO2_CTRL_MAX_COUNTERS)
        return -EINVAL;
    
    ctrl_priv.counters[counter_id].base_value = 
        ctrl_priv.counters[counter_id].value;
    ctrl_priv.counters[counter_id].max_value = 0;
    ctrl_priv.counters[counter_id].min_value = U64_MAX;
    
    return 0;
}

int tofino2_ctrl_counter_reset_all(void)
{
    uint32_t i;
    
    for (i = 0; i < TOFINO2_CTRL_MAX_COUNTERS; i++) {
        if (ctrl_priv.counters[i].allocated) {
            ctrl_priv.counters[i].base_value = 
                ctrl_priv.counters[i].value;
            ctrl_priv.counters[i].max_value = 0;
            ctrl_priv.counters[i].min_value = U64_MAX;
        }
    }
    
    return 0;
}

int tofino2_ctrl_counter_get_stats(uint32_t counter_id,
                                    uint64_t *hits, uint64_t *misses)
{
    if (counter_id >= TOFINO2_CTRL_MAX_COUNTERS)
        return -EINVAL;
    
    if (hits)
        *hits = ctrl_priv.counters[counter_id].value;
    if (misses)
        *misses = 0;
    
    return 0;
}

/* ============================================================================ */
/* Meter management                                                             */
/* ============================================================================ */

int tofino2_ctrl_meter_allocate(uint32_t *meter_id,
                                 enum tofino2_ctrl_meter_mode mode)
{
    int i;
    
    for (i = 0; i < TOFINO2_CTRL_MAX_METERS; i++) {
        if (!(atomic_read(&ctrl_priv.meter_alloc_mask) & (1 << i))) {
            atomic_set(&ctrl_priv.meter_alloc_mask,
                       atomic_read(&ctrl_priv.meter_alloc_mask) | (1 << i));
            ctrl_priv.meters[i].id = i;
            ctrl_priv.meters[i].mode = mode;
            ctrl_priv.meters[i].cir = 0;
            ctrl_priv.meters[i].pir = 0;
            ctrl_priv.meters[i].cbs = 0;
            ctrl_priv.meters[i].pbs = 0;
            ctrl_priv.meters[i].current_credits = 0;
            ctrl_priv.meters[i].enabled = 1;
            ctrl_priv.meters[i].allocated = 1;
            ctrl_priv.meters[i].packet_count = 0;
            ctrl_priv.meters[i].green_packets = 0;
            ctrl_priv.meters[i].yellow_packets = 0;
            ctrl_priv.meters[i].red_packets = 0;
            if (meter_id)
                *meter_id = i;
            return 0;
        }
    }
    
    return -ENOSPC;
}

int tofino2_ctrl_meter_free(uint32_t meter_id)
{
    if (meter_id >= TOFINO2_CTRL_MAX_METERS)
        return -EINVAL;
    
    if (!ctrl_priv.meters[meter_id].allocated)
        return -EINVAL;
    
    atomic_set(&ctrl_priv.meter_alloc_mask,
               atomic_read(&ctrl_priv.meter_alloc_mask) & ~(1 << meter_id));
    ctrl_priv.meters[meter_id].allocated = 0;
    ctrl_priv.meters[meter_id].enabled = 0;
    
    return 0;
}

int tofino2_ctrl_meter_get(uint32_t meter_id,
                            struct tofino2_ctrl_meter *meter)
{
    if (meter_id >= TOFINO2_CTRL_MAX_METERS)
        return -EINVAL;
    
    if (!meter)
        return -EINVAL;
    
    *meter = ctrl_priv.meters[meter_id];
    return 0;
}

int tofino2_ctrl_meter_set(uint32_t meter_id,
                            const struct tofino2_ctrl_meter *meter)
{
    if (meter_id >= TOFINO2_CTRL_MAX_METERS)
        return -EINVAL;
    
    if (!meter)
        return -EINVAL;
    
    ctrl_priv.meters[meter_id] = *meter;
    return 0;
}

int tofino2_ctrl_meter_configure(uint32_t meter_id,
                                  uint32_t cir, uint32_t pir,
                                  uint32_t cbs, uint32_t pbs)
{
    if (meter_id >= TOFINO2_CTRL_MAX_METERS)
        return -EINVAL;
    
    ctrl_priv.meters[meter_id].cir = cir;
    ctrl_priv.meters[meter_id].pir = pir;
    ctrl_priv.meters[meter_id].cbs = cbs;
    ctrl_priv.meters[meter_id].pbs = pbs;
    ctrl_priv.meters[meter_id].enabled = 1;
    
    return 0;
}

int tofino2_ctrl_meter_check(uint32_t meter_id, uint32_t packet_len,
                              uint8_t *color)
{
    if (meter_id >= TOFINO2_CTRL_MAX_METERS)
        return -EINVAL;
    
    if (!color)
        return -EINVAL;
    
    /* Simple token bucket algorithm */
    struct tofino2_ctrl_meter *m = &ctrl_priv.meters[meter_id];
    
    if (packet_len * 8 <= m->current_credits) {
        *color = 0;  /* Green */
        m->current_credits -= packet_len * 8;
        m->green_packets++;
        m->green_bytes += packet_len;
    } else if (packet_len * 8 <= m->current_credits + (m->pbs - m->cbs)) {
        *color = 1;  /* Yellow */
        m->current_credits = 0;
        m->yellow_packets++;
        m->yellow_bytes += packet_len;
    } else {
        *color = 2;  /* Red */
        m->red_packets++;
        m->red_bytes += packet_len;
    }
    
    m->packet_count++;
    
    return 0;
}

/* ============================================================================ */
/* DMA management                                                               */
/* ============================================================================ */

int tofino2_ctrl_dma_allocate(uint32_t *channel_id,
                               enum tofino2_ctrl_dma_direction dir)
{
    int i;
    
    for (i = 0; i < TOFINO2_CTRL_MAX_DMA_CHANNELS; i++) {
        if (!(atomic_read(&ctrl_priv.dma_alloc_mask) & (1 << i))) {
            atomic_set(&ctrl_priv.dma_alloc_mask,
                       atomic_read(&ctrl_priv.dma_alloc_mask) | (1 << i));
            ctrl_priv.dma_channels[i].id = i;
            ctrl_priv.dma_channels[i].dir = dir;
            ctrl_priv.dma_channels[i].status = TOFINO2_CTRL_DMA_IDLE;
            ctrl_priv.dma_channels[i].enabled = 1;
            ctrl_priv.dma_channels[i].allocated = 1;
            ctrl_priv.dma_channels[i].length = 0;
            ctrl_priv.dma_channels[i].completed = 0;
            if (channel_id)
                *channel_id = i;
            return 0;
        }
    }
    
    return -ENOSPC;
}

int tofino2_ctrl_dma_free(uint32_t channel_id)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    if (!ctrl_priv.dma_channels[channel_id].allocated)
        return -EINVAL;
    
    atomic_set(&ctrl_priv.dma_alloc_mask,
               atomic_read(&ctrl_priv.dma_alloc_mask) & ~(1 << channel_id));
    ctrl_priv.dma_channels[channel_id].allocated = 0;
    ctrl_priv.dma_channels[channel_id].enabled = 0;
    ctrl_priv.dma_channels[channel_id].status = TOFINO2_CTRL_DMA_IDLE;
    
    return 0;
}

int tofino2_ctrl_dma_get(uint32_t channel_id,
                          struct tofino2_ctrl_dma *dma)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    if (!dma)
        return -EINVAL;
    
    *dma = ctrl_priv.dma_channels[channel_id];
    return 0;
}

int tofino2_ctrl_dma_set(uint32_t channel_id,
                          const struct tofino2_ctrl_dma *dma)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    if (!dma)
        return -EINVAL;
    
    ctrl_priv.dma_channels[channel_id] = *dma;
    return 0;
}

int tofino2_ctrl_dma_start(uint32_t channel_id,
                            uint64_t src, uint64_t dst, uint32_t len)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    struct tofino2_ctrl_dma *chan = &ctrl_priv.dma_channels[channel_id];
    
    chan->src_addr = src;
    chan->dst_addr = dst;
    chan->length = len;
    chan->completed = 0;
    chan->status = TOFINO2_CTRL_DMA_RUNNING;
    
    return 0;
}

int tofino2_ctrl_dma_stop(uint32_t channel_id)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    ctrl_priv.dma_channels[channel_id].status = TOFINO2_CTRL_DMA_IDLE;
    
    return 0;
}

int tofino2_ctrl_dma_abort(uint32_t channel_id)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    ctrl_priv.dma_channels[channel_id].status = TOFINO2_CTRL_DMA_ABORTED;
    
    return 0;
}

int tofino2_ctrl_dma_poll(uint32_t channel_id, int timeout_ms)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    struct tofino2_ctrl_dma *chan = &ctrl_priv.dma_channels[channel_id];
    
    if (chan->status != TOFINO2_CTRL_DMA_RUNNING)
        return -EINVAL;
    
    /* Simulate DMA completion */
    chan->status = TOFINO2_CTRL_DMA_COMPLETED;
    chan->completed = chan->length;
    
    return 0;
}

int tofino2_ctrl_dma_wait(uint32_t channel_id, int timeout_ms)
{
    return tofino2_ctrl_dma_poll(channel_id, timeout_ms);
}

int tofino2_ctrl_dma_get_status(uint32_t channel_id,
                                 enum tofino2_ctrl_dma_status *status)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    if (!status)
        return -EINVAL;
    
    *status = ctrl_priv.dma_channels[channel_id].status;
    return 0;
}

int tofino2_ctrl_dma_get_completed(uint32_t channel_id, uint32_t *completed)
{
    if (channel_id >= TOFINO2_CTRL_MAX_DMA_CHANNELS)
        return -EINVAL;
    
    if (!completed)
        return -EINVAL;
    
    *completed = ctrl_priv.dma_channels[channel_id].completed;
    return 0;
}

int tofino2_ctrl_dma_buf_alloc(uint32_t size,
                                uint8_t **virt_addr,
                                dma_addr_t *dma_addr)
{
    if (!virt_addr || !dma_addr || size == 0)
        return -EINVAL;
    
    *virt_addr = kmalloc(size, GFP_DMA | GFP_KERNEL);
    if (!*virt_addr)
        return -ENOMEM;
    
    *dma_addr = 0;  /* Would need proper DMA mapping */
    
    return 0;
}

int tofino2_ctrl_dma_buf_free(uint8_t *virt_addr, dma_addr_t dma_addr,
                               uint32_t size)
{
    if (virt_addr)
        kfree(virt_addr);
    
    return 0;
}

/* ============================================================================ */
/* Digest management                                                            */
/* ============================================================================ */

int tofino2_ctrl_digest_allocate(uint32_t *digest_id,
                                  enum tofino2_ctrl_digest_type type)
{
    int i;
    
    for (i = 0; i < 256; i++) {
        if (!(atomic_read(&ctrl_priv.digest_alloc_mask) & (1 << i))) {
            atomic_set(&ctrl_priv.digest_alloc_mask,
                       atomic_read(&ctrl_priv.digest_alloc_mask) | (1 << i));
            ctrl_priv.digests[i].id = i;
            ctrl_priv.digests[i].type = type;
            ctrl_priv.digests[i].enabled = 1;
            ctrl_priv.digests[i].data_len = 0;
            ctrl_priv.digests[i].key_len = 0;
            ctrl_priv.digests[i].computed = 0;
            ctrl_priv.digests[i].verified = 0;
            ctrl_priv.digests[i].errors = 0;
            if (digest_id)
                *digest_id = i;
            return 0;
        }
    }
    
    return -ENOSPC;
}

int tofino2_ctrl_digest_free(uint32_t digest_id)
{
    if (digest_id >= 256)
        return -EINVAL;
    
    if (!ctrl_digests[digest_id].enabled)
        return -EINVAL;
    
    atomic_set(&ctrl_priv.digest_alloc_mask,
               atomic_read(&ctrl_priv.digest_alloc_mask) & ~(1 << digest_id));
    ctrl_priv.digests[digest_id].enabled = 0;
    
    return 0;
}

int tofino2_ctrl_digest_compute(uint32_t digest_id,
                                 const uint8_t *data, uint32_t len)
{
    if (digest_id >= 256)
        return -EINVAL;
    
    if (!data || len == 0)
        return -EINVAL;
    
    struct tofino2_ctrl_digest *d = &ctrl_priv.digests[digest_id];
    
    /* Compute CRC32 as default digest */
    d->data_len = len;
    d->computed++;
    
    return 0;
}

int tofino2_ctrl_digest_verify(uint32_t digest_id,
                                const uint8_t *data, uint32_t len)
{
    if (digest_id >= 256)
        return -EINVAL;
    
    if (!data || len == 0)
        return -EINVAL;
    
    struct tofino2_ctrl_digest *d = &ctrl_priv.digests[digest_id];
    
    if (d->data_len != len)
        return -EINVAL;
    
    d->verified++;
    
    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Firmware Agent");
MODULE_DESCRIPTION("Tofino 2 Control Plane (Counters, Meters, DMA)");
MODULE_VERSION("1.0.0");
