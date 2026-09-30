/*
 * tomtom_er.h — TomTom ASIC driver for MTS Enterprise Router
 *
 * MTS-ER-1000 Enterprise Router — Драйвер Broadcom TomTom ASIC
 */

#ifndef TOMTOM_ER_H
#define TOMTOM_ER_H

#include <linux/types.h>

#define ER_TOMTOM_MAX_PORTS 8
#define ER_TOMTOM_MAX_TCAMS 16
#define ER_TOMTOM_MAX_FDB 4096
#define ER_TOMTOM_MAX_RDB 2048
#define ER_TOMTOM_MAX_ACL_ENTRIES 8192

/* ASIC types */
#define ER_TOMTOM_ASIC_TOM3 "tom3"
#define ER_TOMTOM_ASIC_TOM4 "tom4"

/* Action types */
#define ER_TOMTOM_ACTION_FORWARD "forward"
#define ER_TOMTOM_ACTION_DROP "drop"
#define ER_TOMTOM_ACTION_MIRROR "mirror"
#define ER_TOMTOM_ACTION_ROUTE "route"
#define ER_TOMTOM_ACTION_MPLS "mpls"

/* ==================== ASIC ==================== */

typedef struct {
    uint32_t id;
    char type[16];
    char version[32];
    char status[16];
    uint32_t temperature;
    int32_t tx_power;
    int32_t rx_power;
    uint64_t packets_processed;
    uint64_t bytes_processed;
    uint64_t errors;
} er_tomtom_asic_t;

/* ==================== TCAM ==================== */

typedef struct {
    uint32_t table_id;
    char name[64];
    uint32_t width;
    uint32_t depth;
    uint32_t used_entries;
    uint32_t max_entries;
    char status[16];
    uint64_t hits;
    uint64_t misses;
} er_tomtom_tcam_t;

/* ==================== FDB ==================== */

typedef struct {
    uint32_t entry_id;
    uint8_t mac[6];
    uint32_t port_id;
    uint32_t vlan_id;
    uint32_t is_static;
    uint32_t is_aging;
    uint64_t last_seen;
    uint64_t learn_count;
} er_tomtom_fdb_entry_t;

/* ==================== RDB ==================== */

typedef struct {
    uint32_t entry_id;
    char prefix[64];
    uint32_t prefix_len;
    uint32_t next_hop;
    uint32_t metric;
    uint32_t action;
    char status[16];
    uint64_t packets;
    uint64_t bytes;
} er_tomtom_rdb_entry_t;

/* ==================== ACL ==================== */

typedef struct {
    uint32_t rule_id;
    char name[64];
    uint32_t priority;
    uint32_t match_fields;
    uint32_t action;
    uint32_t hit_count;
    char status[16];
} er_tomtom_acl_entry_t;

/* ==================== API ==================== */

int er_tomtom_asic_init(void);
int er_tomtom_asic_exit(void);
int er_tomtom_asic_get(er_tomtom_asic_t *asic);
int er_tomtom_asic_get_temperature(int32_t *temp);

int er_tomtom_tcam_get(uint32_t table_id, er_tomtom_tcam_t *tcam);
int er_tomtom_tcam_get_all(er_tomtom_tcam_t *tcams, uint32_t *count);
int er_tomtom_tcam_add_entry(uint32_t table_id, const void *key,
                              const void *mask, uint32_t action,
                              uint32_t *entry_id);
int er_tomtom_tcam_delete_entry(uint32_t table_id, uint32_t entry_id);

int er_tomtom_fdb_get(uint32_t entry_id, er_tomtom_fdb_entry_t *entry);
int er_tomtom_fdb_get_all(er_tomtom_fdb_entry_t *entries, uint32_t *count);
int er_tomtom_fdb_add(const er_tomtom_fdb_entry_t *entry);
int er_tomtom_fdb_delete(uint32_t entry_id);
int er_tomtom_fdb_flush(void);

int er_tomtom_rdb_get(uint32_t entry_id, er_tomtom_rdb_entry_t *entry);
int er_tomtom_rdb_get_all(er_tomtom_rdb_entry_t *entries, uint32_t *count);
int er_tomtom_rdb_add(const er_tomtom_rdb_entry_t *entry);
int er_tomtom_rdb_delete(uint32_t entry_id);

int er_tomtom_acl_get(uint32_t rule_id, er_tomtom_acl_entry_t *entry);
int er_tomtom_acl_get_all(er_tomtom_acl_entry_t *entries, uint32_t *count);
int er_tomtom_acl_add(const er_tomtom_acl_entry_t *entry);
int er_tomtom_acl_delete(uint32_t rule_id);

#endif /* TOMTOM_ER_H */
