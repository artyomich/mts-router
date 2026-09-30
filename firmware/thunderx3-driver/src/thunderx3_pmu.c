// SPDX-License-Identifier: GPL-2.0
//
// thunderx3_pmu.c - ThunderX3 Performance Monitoring Unit Implementation
//
// MTS-MC-5000 Mobile Core ThunderX3 Driver
//
// Copyright (c) 2024 MTS Router Project

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/perf_event.h>
#include <linux/cpu_pm.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>

#include "thunderx3.h"
#include "thunderx3_pmu.h"

#define DRIVER_NAME "thunderx3_pmu"
#define PMU_MAGIC 0x504D5533  /* "PMU3" */

/* ============================================================================ */
/* Internal state                                                               */
/* ============================================================================ */

struct thunderx3_pmu_priv {
    uint32_t magic;
    struct thunderx3_pmc pmcs[THUNDERX3_PMU_MAX_COUNTERS];
    uint32_t num_pmcs;
    spinlock_t lock;
    uint32_t enabled;
    uint64_t poll_interval;
    uint64_t last_poll;
    uint64_t total_samples;
    uint64_t total_errors;
};

static struct thunderx3_pmu_priv pmu_priv;

/* ============================================================================ */
/* PMU counter management                                                       */
/* ============================================================================ */

int thunderx3_pmc_enable(uint32_t core_id,
                          enum thunderx3_pmu_event event)
{
    if (core_id >= THUNDERX3_MAX_CORES_PER_DEV)
        return -EINVAL;
    
    spin_lock(&pmu_priv.lock);
    
    for (uint32_t i = 0; i < pmu_priv.num_pmcs; i++) {
        if (!pmu_priv.pmcs[i].enabled &&
            pmu_priv.pmcs[i].core_id == core_id) {
            pmu_priv.pmcs[i].event = event;
            pmu_priv.pmcs[i].enabled = 1;
            pmu_priv.pmcs[i].value = 0;
            pmu_priv.pmcs[i].base_value = 0;
            pmu_priv.pmcs[i].max_value = 0;
            pmu_priv.pmcs[i].min_value = UINT64_MAX;
            spin_unlock(&pmu_priv.lock);
            return 0;
        }
    }
    
    /* Find free PMC */
    for (uint32_t i = 0; i < THUNDERX3_PMU_MAX_COUNTERS; i++) {
        if (!pmu_priv.pmcs[i].enabled) {
            pmu_priv.pmcs[i].id = i;
            pmu_priv.pmcs[i].core_id = core_id;
            pmu_priv.pmcs[i].event = event;
            pmu_priv.pmcs[i].enabled = 1;
            pmu_priv.pmcs[i].value = 0;
            pmu_priv.pmcs[i].base_value = 0;
            pmu_priv.pmcs[i].max_value = 0;
            pmu_priv.pmcs[i].min_value = UINT64_MAX;
            pmu_priv.num_pmcs++;
            spin_unlock(&pmu_priv.lock);
            return 0;
        }
    }
    
    spin_unlock(&pmu_priv.lock);
    return -ENOSPC;
}

int thunderx3_pmc_disable(uint32_t core_id,
                           enum thunderx3_pmu_event event)
{
    spin_lock(&pmu_priv.lock);
    
    for (uint32_t i = 0; i < pmu_priv.num_pmcs; i++) {
        if (pmu_priv.pmcs[i].enabled &&
            pmu_priv.pmcs[i].core_id == core_id &&
            pmu_priv.pmcs[i].event == event) {
            pmu_priv.pmcs[i].enabled = 0;
            spin_unlock(&pmu_priv.lock);
            return 0;
        }
    }
    
    spin_unlock(&pmu_priv.lock);
    return -EINVAL;
}

int thunderx3_pmc_read(uint32_t core_id,
                        enum thunderx3_pmu_event event,
                        uint64_t *value)
{
    if (!value)
        return -EINVAL;
    
    spin_lock(&pmu_priv.lock);
    
    for (uint32_t i = 0; i < pmu_priv.num_pmcs; i++) {
        if (pmu_priv.pmcs[i].enabled &&
            pmu_priv.pmcs[i].core_id == core_id &&
            pmu_priv.pmcs[i].event == event) {
            *value = pmu_priv.pmcs[i].value;
            spin_unlock(&pmu_priv.lock);
            return 0;
        }
    }
    
    spin_unlock(&pmu_priv.lock);
    return -EINVAL;
}

int thunderx3_pmc_reset(uint32_t core_id,
                         enum thunderx3_pmu_event event)
{
    spin_lock(&pmu_priv.lock);
    
    for (uint32_t i = 0; i < pmu_priv.num_pmcs; i++) {
        if (pmu_priv.pmcs[i].enabled &&
            pmu_priv.pmcs[i].core_id == core_id &&
            pmu_priv.pmcs[i].event == event) {
            pmu_priv.pmcs[i].base_value = pmu_priv.pmcs[i].value;
            pmu_priv.pmcs[i].max_value = 0;
            pmu_priv.pmcs[i].min_value = UINT64_MAX;
            spin_unlock(&pmu_priv.lock);
            return 0;
        }
    }
    
    spin_unlock(&pmu_priv.lock);
    return -EINVAL;
}

/* ============================================================================ */
/* PMU snapshot and control                                                     */
/* ============================================================================ */

int thunderx3_pmu_snapshot(struct thunderx3_pmc *pmcs,
                            uint32_t *num_pmcs)
{
    if (!pmcs || !num_pmcs)
        return -EINVAL;
    
    spin_lock(&pmu_priv.lock);
    
    *num_pmcs = pmu_priv.num_pmcs;
    
    for (uint32_t i = 0; i < pmu_priv.num_pmcs; i++) {
        pmcs[i] = pmu_priv.pmcs[i];
    }
    
    spin_unlock(&pmu_priv.lock);
    
    return 0;
}

int thunderx3_pmu_start(void)
{
    spin_lock(&pmu_priv.lock);
    pmu_priv.enabled = 1;
    pmu_priv.total_samples = 0;
    pmu_priv.total_errors = 0;
    spin_unlock(&pmu_priv.lock);
    
    pr_info("thunderx3_pmu: PMU started\n");
    return 0;
}

int thunderx3_pmu_stop(void)
{
    spin_lock(&pmu_priv.lock);
    pmu_priv.enabled = 0;
    spin_unlock(&pmu_priv.lock);
    
    pr_info("thunderx3_pmu: PMU stopped\n");
    return 0;
}

/* ============================================================================ */
/* PMU init/exit                                                                */
/* ============================================================================ */

int thunderx3_pmu_init(struct thunderx3_device *dev)
{
    if (!dev)
        return -EINVAL;
    
    memset(&pmu_priv, 0, sizeof(pmu_priv));
    pmu_priv.magic = PMU_MAGIC;
    spin_lock_init(&pmu_priv.lock);
    pmu_priv.num_pmcs = 0;
    pmu_priv.enabled = 0;
    pmu_priv.poll_interval = 1000000000;  /* 1 second */
    
    pr_info("thunderx3_pmu: PMU subsystem initialized\n");
    return 0;
}

void thunderx3_pmu_exit(struct thunderx3_device *dev)
{
    if (pmu_priv.magic == PMU_MAGIC) {
        pr_info("thunderx3_pmu: PMU subsystem exited\n");
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Firmware Agent");
MODULE_DESCRIPTION("ThunderX3 Performance Monitoring Unit");
MODULE_VERSION("1.0.0");
