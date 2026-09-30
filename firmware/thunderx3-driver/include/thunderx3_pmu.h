/* SPDX-License-Identifier: GPL-2.0 */
/*
 * thunderx3_pmu.h - ThunderX3 Performance Monitoring Unit Interface
 *
 * MTS-MC-5000 Mobile Core ThunderX3 Driver
 */

#ifndef THUNDERX3_PMU_H
#define THUNDERX3_PMU_H

#include <linux/types.h>

#define THUNDERX3_PMU_MAX_COUNTERS 32
#define THUNDERX3_PMU_MAX_CORES 96
#define THUNDERX3_PMU_NAME_LEN 32

/* PMU event types */
enum thunderx3_pmu_event_type {
    THUNDERX3_PMU_EVENT_CPU_CYCLES,
    THUNDERX3_PMU_EVENT_CPU_INSTRUCTIONS,
    THUNDERX3_PMU_EVENT_CACHE_REFERENCES,
    THUNDERX3_PMU_EVENT_CACHE_MISSES,
    THUNDERX3_PMU_EVENT_BRANCH_INSTRUCTIONS,
    THUNDERX3_PMU_EVENT_BRANCH_MISSES,
    THUNDERX3_PMU_EVENT_BUS_CYCLES,
    THUNDERX3_PMU_EVENT_STALLED_CYCLES_FRONT,
    THUNDERX3_PMU_EVENT_STALLED_CYCLES_BACK,
    THUNDERX3_PMU_EVENT_POWER,
    THUNDERX3_PMU_EVENT_TEMPERATURE,
    THUNDERX3_PMU_EVENT_DDR_READS,
    THUNDERX3_PMU_EVENT_DDR_WRITES,
    THUNDERX3_PMU_EVENT_DDR_BANDWIDTH,
    THUNDERX3_PMU_EVENT_PCIE_READS,
    THUNDERX3_PMU_EVENT_PCIE_WRITES,
    THUNDERX3_PMU_EVENT_PCIE_BANDWIDTH,
    THUNDERX3_PMU_EVENT_LAST
};

struct thunderx3_pmu_counter {
    uint32_t id;
    uint32_t core_id;
    enum thunderx3_pmu_event_type event;
    uint64_t value;
    uint64_t base_value;
    uint64_t max_value;
    uint64_t min_value;
    uint32_t enabled;
    uint32_t overflow_count;
    uint32_t width;
    char name[THUNDERX3_PMU_NAME_LEN];
};

struct thunderx3_pmu_snapshot {
    uint64_t timestamp;
    uint32_t num_cores;
    uint64_t cpu_cycles[THUNDERX3_PMU_MAX_CORES];
    uint64_t cpu_instructions[THUNDERX3_PMU_MAX_CORES];
    uint64_t cache_misses[THUNDERX3_PMU_MAX_CORES];
    uint64_t branch_misses[THUNDERX3_PMU_MAX_CORES];
    int32_t temperatures[THUNDERX3_PMU_MAX_CORES];
    uint32_t power_mw;
    uint32_t voltage_mv;
    uint32_t fan_rpm[8];
    uint32_t num_fans;
};

/* API */
int thunderx3_pmu_init(void);
void thunderx3_pmu_exit(void);
int thunderx3_pmu_counter_enable(uint32_t core_id,
                                  enum thunderx3_pmu_event_type event);
int thunderx3_pmu_counter_disable(uint32_t core_id,
                                   enum thunderx3_pmu_event_type event);
int thunderx3_pmu_counter_read(uint32_t core_id,
                                enum thunderx3_pmu_event_type event,
                                uint64_t *value);
int thunderx3_pmu_counter_reset(uint32_t core_id,
                                 enum thunderx3_pmu_event_type event);
int thunderx3_pmu_snapshot(struct thunderx3_pmu_snapshot *snap);
int thunderx3_pmu_start(void);
int thunderx3_pmu_stop(void);
int thunderx3_pmu_get_counters(struct thunderx3_pmu_counter **counters,
                                uint32_t *num_counters);

#endif /* THUNDERX3_PMU_H */
