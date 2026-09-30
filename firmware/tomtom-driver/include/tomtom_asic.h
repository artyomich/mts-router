/*
 * tomtom_asic.h — ASIC management for Broadcom TomTom
 *
 * MTS-ER-1000 Enterprise Router — Управление ASIC таблицами
 */

#ifndef TOMTOM_ASIC_H
#define TOMTOM_ASIC_H

#include <linux/types.h>

#define TOMTOM_ASIC_MAX_TABLES 4096
#define TOMTOM_ASIC_MAX_ENTRIES 1048576

/* ASIC table types */
enum tomtom_table_type {
    TOMTOM_TABLE_L2 = 0,
    TOMTOM_TABLE_L3,
    TOMTOM_TABLE_L4,
    TOMTOM_TABLE_FLOW,
    TOMTOM_TABLE_QOS,
    TOMTOM_TABLE_ACL,
    TOMTOM_TABLE_MPLS,
    TOMTOM_TABLE_SRV6
};

/* ASIC table entry */
struct tomtom_asic_table_entry {
    uint32_t id;
    char name[64];
    uint32_t type;
    uint32_t key_size;
    uint32_t action_size;
    uint32_t max_entries;
    uint32_t current_entries;
    uint32_t hit_rate;
    uint32_t miss_rate;
    uint64_t total_hits;
    uint64_t total_misses;
};

/* ASIC device */
struct tomtom_asic_device {
    uint32_t id;
    char name[32];
    struct tomtom_asic_table_entry tables[TOMTOM_ASIC_MAX_TABLES];
    uint32_t num_tables;
    void *priv;
    struct device *dev;
    struct mutex lock;
    uint32_t firmware_version;
    uint32_t hw_revision;
};

/* Function prototypes */
int tomtom_asic_init(struct tomtom_asic_device *adev);
void tomtom_asic_exit(struct tomtom_asic_device *adev);
int tomtom_asic_probe(struct device *dev);
int tomtom_asic_remove(struct device *dev);
int tomtom_asic_add_table(struct tomtom_asic_device *adev, struct tomtom_asic_table_entry *table);
int tomtom_asic_del_table(struct tomtom_asic_device *adev, uint32_t table_id);
int tomtom_asic_get_table(struct tomtom_asic_device *adev, uint32_t table_id, struct tomtom_asic_table_entry *table);
int tomtom_asic_get_all_tables(struct tomtom_asic_device *adev, struct tomtom_asic_table_entry *tables, uint32_t max);
int tomtom_asic_add_entry(struct tomtom_asic_device *adev, uint32_t table_id, uint8_t *key, uint8_t *action, uint32_t priority);
int tomtom_asic_del_entry(struct tomtom_asic_device *adev, uint32_t table_id, uint8_t *key);
int tomtom_asic_lookup(struct tomtom_asic_device *adev, uint32_t table_id, uint8_t *key, uint8_t *action);
int tomtom_asic_flush_table(struct tomtom_asic_device *adev, uint32_t table_id);
int tomtom_asic_get_stats(struct tomtom_asic_device *adev);

#endif /* TOMTOM_ASIC_H */
