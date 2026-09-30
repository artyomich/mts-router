/*
 * tomtom.h — Основные структуры и определения для Broadcom TomTom ASIC
 *
 * MTS-ER-1000 Enterprise Router — Драйвер TomTom ASIC
 */

#ifndef TOMTOM_H
#define TOMTOM_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

#define TOMTOM_MODULE_NAME "tomtom"
#define TOMTOM_MAX_PORTS 64
#define TOMTOM_MAX_TABLES 4096
#define TOMTOM_MAX_ENTRIES 1048576
#define TOMTOM_MAX_FLOWS 65536
#define TOMTOM_IOCTL_MAX 16

/* Device states */
enum tomtom_state {
    TOMTOM_STATE_INIT,
    TOMTOM_STATE_READY,
    TOMTOM_STATE_RUNNING,
    TOMTOM_STATE_ERROR,
    TOMTOM_STATE_STOPPED
};

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

/* ASIC table */
struct tomtom_asic_table {
    uint32_t id;
    char name[64];
    uint32_t type;        /* TOMTOM_TABLE_* */
    uint32_t key_size;
    uint32_t action_size;
    uint32_t max_entries;
    uint32_t current_entries;
    uint32_t hit_rate;
    uint32_t miss_rate;
    uint64_t total_hits;
    uint64_t total_misses;
};

/* ASIC flow entry */
struct tomtom_flow_entry {
    uint32_t id;
    uint8_t key[128];
    uint8_t action[128];
    uint32_t priority;
    uint32_t table_id;
    uint64_t packets;
    uint64_t bytes;
    uint32_t state;       /* ACTIVE/INACTIVE/EXPIRED */
    uint64_t created;
    uint64_t last_seen;
};

/* ASIC device */
struct tomtom_device {
    uint32_t id;
    char name[32];
    enum tomtom_state state;
    uint32_t num_tables;
    uint32_t num_flows;
    struct tomtom_asic_table *tables;
    struct tomtom_flow_entry *flows;
    void *priv;
    struct device *dev;
    struct mutex lock;
    struct workqueue_struct *wq;
    struct timer_list timer;
    uint32_t temperature;
    uint32_t power_consumption;
};

/* IOCTL commands */
#define TOMTOM_IOC_MAGIC 'T'
#define TOMTOM_IOC_GET_DEV      _IOR(TOMTOM_IOC_MAGIC, 1, struct tomtom_device)
#define TOMTOM_IOC_SET_DEV      _IOW(TOMTOM_IOC_MAGIC, 2, struct tomtom_device)
#define TOMTOM_IOC_GET_TABLE    _IOR(TOMTOM_IOC_MAGIC, 3, struct tomtom_asic_table)
#define TOMTOM_IOC_SET_TABLE    _IOW(TOMTOM_IOC_MAGIC, 4, struct tomtom_asic_table)
#define TOMTOM_IOC_GET_TABLES   _IOR(TOMTOM_IOC_MAGIC, 5, struct tomtom_asic_table[])
#define TOMTOM_IOC_ADD_ENTRY    _IOW(TOMTOM_IOC_MAGIC, 6, struct tomtom_flow_entry)
#define TOMTOM_IOC_DEL_ENTRY    _IOW(TOMTOM_IOC_MAGIC, 7, uint32_t)
#define TOMTOM_IOC_LOOKUP       _IOW(TOMTOM_IOC_MAGIC, 8, struct tomtom_flow_entry)
#define TOMTOM_IOC_GET_STATS    _IOR(TOMTOM_IOC_MAGIC, 9, struct tomtom_device)
#define TOMTOM_IOC_GET_TEMP     _IOR(TOMTOM_IOC_MAGIC, 10, uint32_t)
#define TOMTOM_IOC_GET_POWER    _IOR(TOMTOM_IOC_MAGIC, 11, uint32_t)
#define TOMTOM_IOC_RESET        _IO(TOMTOM_IOC_MAGIC, 12)
#define TOMTOM_IOC_FLUSH_TABLE  _IOW(TOMTOM_IOC_MAGIC, 13, uint32_t)
#define TOMTOM_IOC_GET_FLOWS    _IOR(TOMTOM_IOC_MAGIC, 14, struct tomtom_flow_entry[])

/* Function prototypes */
int tomtom_init(void);
void tomtom_exit(void);
int tomtom_probe(struct device *dev);
int tomtom_remove(struct device *dev);
struct tomtom_device *tomtom_get_device(uint32_t id);
int tomtom_add_table(struct tomtom_device *dev, struct tomtom_asic_table *table);
int tomtom_del_table(struct tomtom_device *dev, uint32_t table_id);
int tomtom_add_entry(struct tomtom_device *dev, struct tomtom_flow_entry *entry);
int tomtom_del_entry(struct tomtom_device *dev, uint32_t entry_id);
int tomtom_lookup(struct tomtom_device *dev, uint32_t table_id, uint8_t *key, uint8_t *action);
int tomtom_flush_table(struct tomtom_device *dev, uint32_t table_id);
int tomtom_get_temperature(struct tomtom_device *dev);
int tomtom_get_power(struct tomtom_device *dev);

#endif /* TOMTOM_H */
