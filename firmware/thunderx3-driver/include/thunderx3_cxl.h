/* SPDX-License-Identifier: GPL-2.0 */
/*
 * thunderx3_cxl.h - ThunderX3 CXL (Compute Express Link) Interface
 *
 * MTS-MC-5000 Mobile Core ThunderX3 Driver
 */

#ifndef THUNDERX3_CXL_H
#define THUNDERX3_CXL_H

#include <linux/types.h>

#define THUNDERX3_CXL_MAX_DEVICES 4
#define THUNDERX3_CXL_MAX_REGIONS 16
#define THUNDERX3_CXL_NAME_LEN 32

enum thunderx3_cxl_device_type {
    THUNDERX3_CXL_NONE = 0,
    THUNDERX3_CXL_TYPE1 = 1,
    THUNDERX3_CXL_TYPE2 = 2,
    THUNDERX3_CXL_TYPE3 = 3,
    THUNDERX3_CXL_UNKNOWN = 255
};

struct thunderx3_cxl_region {
    uint32_t id;
    uint64_t base;
    uint64_t size;
    uint32_t active;
    char name[THUNDERX3_CXL_NAME_LEN];
};

struct thunderx3_cxl_device {
    uint32_t id;
    enum thunderx3_cxl_device_type type;
    char name[THUNDERX3_CXL_NAME_LEN];
    uint32_t status;
    uint64_t mem_base;
    uint64_t mem_size;
    uint32_t mem_type;
    uint32_t mem_speed;
    uint32_t mem_width;
    uint32_t num_regions;
    struct thunderx3_cxl_region regions[THUNDERX3_CXL_MAX_REGIONS];
    uint32_t cap_version;
    uint32_t cap_flags;
    uint32_t enabled;
    uint32_t pad[4];
};

/* API */
int thunderx3_cxl_init(void);
void thunderx3_cxl_exit(void);
int thunderx3_cxl_device_enable(uint32_t cxl_id);
int thunderx3_cxl_device_disable(uint32_t cxl_id);
int thunderx3_cxl_device_get(uint32_t cxl_id,
                              struct thunderx3_cxl_device *cxl);
int thunderx3_cxl_region_add(uint32_t cxl_id,
                              uint64_t base, uint64_t size,
                              uint32_t *region_id);
int thunderx3_cxl_region_remove(uint32_t cxl_id, uint32_t region_id);
int thunderx3_cxl_get_memory_info(uint64_t *total, uint64_t *free);
int thunderx3_cxl_device_list(struct thunderx3_cxl_device **devices,
                               uint32_t *num_devices);

#endif /* THUNDERX3_CXL_H */
