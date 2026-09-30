// SPDX-License-Identifier: GPL-2.0
//
// thunderx3_cxl.c - ThunderX3 CXL (Compute Express Link) Implementation
//
// MTS-MC-5000 Mobile Core ThunderX3 Driver
//
// Copyright (c) 2024 MTS Router Project

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>

#include "thunderx3.h"
#include "thunderx3_cxl.h"

#define DRIVER_NAME "thunderx3_cxl"
#define CXL_MAGIC 0x43584C33  /* "CXL3" */

/* ============================================================================ */
/* Internal state                                                               */
/* ============================================================================ */

struct thunderx3_cxl_priv {
    uint32_t magic;
    struct thunderx3_cxl_device devices[THUNDERX3_CXL_MAX_DEVICES];
    uint32_t num_devices;
    uint64_t total_memory;
    uint64_t free_memory;
    struct mutex lock;
    uint32_t enabled;
};

static struct thunderx3_cxl_priv cxl_priv;

/* ============================================================================ */
/* CXL device management                                                        */
/* ============================================================================ */

int thunderx3_cxl_device_enable(uint32_t cxl_id)
{
    if (cxl_id >= THUNDERX3_CXL_MAX_DEVICES)
        return -EINVAL;
    
    mutex_lock(&cxl_priv.lock);
    
    if (cxl_priv.devices[cxl_id].type == THUNDERX3_CXL_NONE) {
        mutex_unlock(&cxl_priv.lock);
        return -EINVAL;
    }
    
    cxl_priv.devices[cxl_id].enabled = 1;
    cxl_priv.devices[cxl_id].status = 1;
    
    mutex_unlock(&cxl_priv.lock);
    
    pr_info("thunderx3_cxl: CXL device %d enabled\n", cxl_id);
    return 0;
}

int thunderx3_cxl_device_disable(uint32_t cxl_id)
{
    if (cxl_id >= THUNDERX3_CXL_MAX_DEVICES)
        return -EINVAL;
    
    mutex_lock(&cxl_priv.lock);
    
    cxl_priv.devices[cxl_id].enabled = 0;
    cxl_priv.devices[cxl_id].status = 0;
    
    mutex_unlock(&cxl_priv.lock);
    
    pr_info("thunderx3_cxl: CXL device %d disabled\n", cxl_id);
    return 0;
}

int thunderx3_cxl_device_get(uint32_t cxl_id,
                              struct thunderx3_cxl_device *cxl)
{
    if (cxl_id >= THUNDERX3_CXL_MAX_DEVICES || !cxl)
        return -EINVAL;
    
    mutex_lock(&cxl_priv.lock);
    
    *cxl = cxl_priv.devices[cxl_id];
    
    mutex_unlock(&cxl_priv.lock);
    
    return 0;
}

int thunderx3_cxl_region_add(uint32_t cxl_id,
                              uint64_t base, uint64_t size,
                              uint32_t *region_id)
{
    if (cxl_id >= THUNDERX3_CXL_MAX_DEVICES)
        return -EINVAL;
    
    mutex_lock(&cxl_priv.lock);
    
    struct thunderx3_cxl_device *dev = &cxl_priv.devices[cxl_id];
    
    if (dev->num_regions >= THUNDERX3_CXL_MAX_REGIONS) {
        mutex_unlock(&cxl_priv.lock);
        return -ENOSPC;
    }
    
    uint32_t id = dev->num_regions;
    dev->regions[id].id = id;
    dev->regions[id].base = base;
    dev->regions[id].size = size;
    dev->regions[id].active = 1;
    strscpy(dev->regions[id].name, "cxl-region", sizeof(dev->regions[id].name));
    dev->num_regions++;
    
    if (region_id)
        *region_id = id;
    
    cxl_priv.free_memory -= size;
    
    mutex_unlock(&cxl_priv.lock);
    
    pr_info("thunderx3_cxl: Region added to CXL %d: base=0x%lx, size=0x%lx\n",
            cxl_id, base, size);
    return 0;
}

int thunderx3_cxl_region_remove(uint32_t cxl_id, uint32_t region_id)
{
    if (cxl_id >= THUNDERX3_CXL_MAX_DEVICES)
        return -EINVAL;
    
    mutex_lock(&cxl_priv.lock);
    
    struct thunderx3_cxl_device *dev = &cxl_priv.devices[cxl_id];
    
    if (region_id >= dev->num_regions) {
        mutex_unlock(&cxl_priv.lock);
        return -EINVAL;
    }
    
    uint64_t size = dev->regions[region_id].size;
    dev->regions[region_id].active = 0;
    
    /* Shift regions array */
    for (uint32_t i = region_id; i < dev->num_regions - 1; i++) {
        dev->regions[i] = dev->regions[i + 1];
    }
    dev->num_regions--;
    
    cxl_priv.free_memory += size;
    
    mutex_unlock(&cxl_priv.lock);
    
    pr_info("thunderx3_cxl: Region %d removed from CXL %d\n",
            region_id, cxl_id);
    return 0;
}

int thunderx3_cxl_get_memory_info(uint64_t *total, uint64_t *free)
{
    if (total)
        *total = cxl_priv.total_memory;
    if (free)
        *free = cxl_priv.free_memory;
    
    return 0;
}

int thunderx3_cxl_device_list(struct thunderx3_cxl_device **devices,
                               uint32_t *num_devices)
{
    if (!devices || !num_devices)
        return -EINVAL;
    
    mutex_lock(&cxl_priv.lock);
    
    *num_devices = cxl_priv.num_devices;
    
    for (uint32_t i = 0; i < cxl_priv.num_devices; i++) {
        devices[i] = kzalloc(sizeof(**devices), GFP_ATOMIC);
        if (devices[i])
            *devices[i] = cxl_priv.devices[i];
    }
    
    mutex_unlock(&cxl_priv.lock);
    
    return 0;
}

/* ============================================================================ */
/* CXL init/exit                                                                */
/* ============================================================================ */

int thunderx3_cxl_init(struct thunderx3_device *dev)
{
    if (!dev)
        return -EINVAL;
    
    memset(&cxl_priv, 0, sizeof(cxl_priv));
    cxl_priv.magic = CXL_MAGIC;
    mutex_init(&cxl_priv.lock);
    cxl_priv.num_devices = 0;
    cxl_priv.total_memory = 0;
    cxl_priv.free_memory = 0;
    cxl_priv.enabled = 0;
    
    pr_info("thunderx3_cxl: CXL subsystem initialized\n");
    return 0;
}

void thunderx3_cxl_exit(struct thunderx3_device *dev)
{
    if (cxl_priv.magic == CXL_MAGIC) {
        pr_info("thunderx3_cxl: CXL subsystem exited\n");
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Firmware Agent");
MODULE_DESCRIPTION("ThunderX3 CXL Subsystem");
MODULE_VERSION("1.0.0");
