/*
 * tomtom_er.c — TomTom ASIC driver for MTS Enterprise Router
 *
 * MTS-ER-1000 Enterprise Router — Драйвер Broadcom TomTom ASIC
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>

#include "tomtom_er.h"

#define DRIVER_VERSION "1.0.0-er-tomtom"
#define DRIVER_NAME "tomtom_er"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Project");
MODULE_DESCRIPTION("TomTom ASIC driver for enterprise router");
MODULE_VERSION(DRIVER_VERSION);

static int debug_level = 1;
module_param(debug_level, int, 0644);
MODULE_PARM_DESC(debug_level, "Debug level (0=off, 1=error, 2=info, 3=debug)");

#define tom_dbg(fmt, ...) \
    do { if (debug_level >= 3) pr_debug(fmt, ##__VA_ARGS__); } while (0)
#define tom_info(fmt, ...) \
    do { if (debug_level >= 2) pr_info(fmt, ##__VA_ARGS__); } while (0)
#define tom_err(fmt, ...) \
    do { if (debug_level >= 1) pr_err(fmt, ##__VA_ARGS__); } while (0)

/* ==================== ASIC ==================== */

static er_tomtom_asic_t tomtom_asic;
static DEFINE_MUTEX(tomtom_mutex);

int er_tomtom_asic_init(void)
{
    tom_info("Initializing TomTom ASIC driver v%s\n", DRIVER_VERSION);

    memset(&tomtom_asic, 0, sizeof(er_tomtom_asic_t));
    tomtom_asic.id = 0;
    strcpy(tomtom_asic.type, ER_TOMTOM_ASIC_TOM3);
    strcpy(tomtom_asic.version, "1.0.0");
    strcpy(tomtom_asic.status, "active");
    tomtom_asic.temperature = 4500; /* 45.00°C */

    return 0;
}

int er_tomtom_asic_exit(void)
{
    strcpy(tomtom_asic.status, "inactive");
    tom_info("TomTom ASIC driver unloaded\n");
    return 0;
}

int er_tomtom_asic_get(er_tomtom_asic_t *asic)
{
    if (!asic)
        return -EINVAL;

    mutex_lock(&tomtom_mutex);
    memcpy(asic, &tomtom_asic, sizeof(er_tomtom_asic_t));
    mutex_unlock(&tomtom_mutex);
    return 0;
}

int er_tomtom_asic_get_temperature(int32_t *temp)
{
    if (!temp)
        return -EINVAL;

    mutex_lock(&tomtom_mutex);
    *temp = tomtom_asic.temperature;
    mutex_unlock(&tomtom_mutex);
    return 0;
}

/* ==================== TCAM ==================== */

static er_tomtom_tcam_t tomtom_tcams[ER_TOMTOM_MAX_TCAMS];
static int tcam_count = 0;

int er_tomtom_tcam_get(uint32_t table_id, er_tomtom_tcam_t *tcam)
{
    if (table_id >= ER_TOMTOM_MAX_TCAMS || !tcam)
        return -EINVAL;
    if (table_id >= (uint32_t)tcam_count)
        return -ENOENT;

    memcpy(tcam, &tomtom_tcams[table_id], sizeof(er_tomtom_tcam_t));
    return 0;
}

int er_tomtom_tcam_get_all(er_tomtom_tcam_t *tcams, uint32_t *count)
{
    int i;

    if (!tcams || !count)
        return -EINVAL;

    for (i = 0; i < tcam_count; i++)
        memcpy(&tcams[i], &tomtom_tcams[i], sizeof(er_tomtom_tcam_t));
    *count = tcam_count;
    return 0;
}

int er_tomtom_tcam_add_entry(uint32_t table_id, const void *key,
                              const void *mask, uint32_t action,
                              uint32_t *entry_id)
{
    if (table_id >= ER_TOMTOM_MAX_TCAMS || !key || !entry_id)
        return -EINVAL;
    if (table_id >= (uint32_t)tcam_count)
        return -ENOENT;

    tomtom_tcams[table_id].used_entries++;
    *entry_id = tomtom_tcams[table_id].used_entries;
    tom_info("Added TCAM entry %u to table %u\n", *entry_id, table_id);
    return 0;
}

int er_tomtom_tcam_delete_entry(uint32_t table_id, uint32_t entry_id)
{
    if (table_id >= ER_TOMTOM_MAX_TCAMS)
        return -EINVAL;
    if (table_id >= (uint32_t)tcam_count)
        return -ENOENT;

    tomtom_tcams[table_id].used_entries--;
    tom_info("Deleted TCAM entry %u from table %u\n", entry_id, table_id);
    return 0;
}

/* ==================== FDB ==================== */

static er_tomtom_fdb_entry_t tomtom_fdb[ER_TOMTOM_MAX_FDB];
static int fdb_count = 0;

int er_tomtom_fdb_get(uint32_t entry_id, er_tomtom_fdb_entry_t *entry)
{
    if (entry_id >= ER_TOMTOM_MAX_FDB || !entry)
        return -EINVAL;
    if (entry_id >= (uint32_t)fdb_count)
        return -ENOENT;

    memcpy(entry, &tomtom_fdb[entry_id], sizeof(er_tomtom_fdb_entry_t));
    return 0;
}

int er_tomtom_fdb_get_all(er_tomtom_fdb_entry_t *entries, uint32_t *count)
{
    int i;

    if (!entries || !count)
        return -EINVAL;

    for (i = 0; i < fdb_count; i++)
        memcpy(&entries[i], &tomtom_fdb[i], sizeof(er_tomtom_fdb_entry_t));
    *count = fdb_count;
    return 0;
}

int er_tomtom_fdb_add(const er_tomtom_fdb_entry_t *entry)
{
    if (!entry || fdb_count >= ER_TOMTOM_MAX_FDB)
        return -EINVAL;

    memcpy(&tomtom_fdb[fdb_count], entry, sizeof(er_tomtom_fdb_entry_t));
    tom_info("Added FDB entry: MAC %pM port %u\n",
             entry->mac, entry->port_id);
    fdb_count++;
    return 0;
}

int er_tomtom_fdb_delete(uint32_t entry_id)
{
    if (entry_id >= (uint32_t)fdb_count)
        return -EINVAL;

    fdb_count--;
    if (entry_id < (uint32_t)fdb_count)
        memmove(&tomtom_fdb[entry_id], &tomtom_fdb[entry_id + 1],
                (fdb_count - entry_id) * sizeof(er_tomtom_fdb_entry_t));
    return 0;
}

int er_tomtom_fdb_flush(void)
{
    fdb_count = 0;
    tom_info("FDB flushed\n");
    return 0;
}

/* ==================== RDB ==================== */

static er_tomtom_rdb_entry_t tomtom_rdb[ER_TOMTOM_MAX_RDB];
static int rdb_count = 0;

int er_tomtom_rdb_get(uint32_t entry_id, er_tomtom_rdb_entry_t *entry)
{
    if (entry_id >= ER_TOMTOM_MAX_RDB || !entry)
        return -EINVAL;
    if (entry_id >= (uint32_t)rdb_count)
        return -ENOENT;

    memcpy(entry, &tomtom_rdb[entry_id], sizeof(er_tomtom_rdb_entry_t));
    return 0;
}

int er_tomtom_rdb_get_all(er_tomtom_rdb_entry_t *entries, uint32_t *count)
{
    int i;

    if (!entries || !count)
        return -EINVAL;

    for (i = 0; i < rdb_count; i++)
        memcpy(&entries[i], &tomtom_rdb[i], sizeof(er_tomtom_rdb_entry_t));
    *count = rdb_count;
    return 0;
}

int er_tomtom_rdb_add(const er_tomtom_rdb_entry_t *entry)
{
    if (!entry || rdb_count >= ER_TOMTOM_MAX_RDB)
        return -EINVAL;

    memcpy(&tomtom_rdb[rdb_count], entry, sizeof(er_tomtom_rdb_entry_t));
    tom_info("Added RDB entry: %s via port %u\n",
             entry->prefix, entry->next_hop);
    rdb_count++;
    return 0;
}

int er_tomtom_rdb_delete(uint32_t entry_id)
{
    if (entry_id >= (uint32_t)rdb_count)
        return -EINVAL;

    rdb_count--;
    if (entry_id < (uint32_t)rdb_count)
        memmove(&tomtom_rdb[entry_id], &tomtom_rdb[entry_id + 1],
                (rdb_count - entry_id) * sizeof(er_tomtom_rdb_entry_t));
    return 0;
}

/* ==================== ACL ==================== */

static er_tomtom_acl_entry_t tomtom_acl[ER_TOMTOM_MAX_ACL_ENTRIES];
static int acl_count = 0;

int er_tomtom_acl_get(uint32_t rule_id, er_tomtom_acl_entry_t *entry)
{
    if (rule_id >= ER_TOMTOM_MAX_ACL_ENTRIES || !entry)
        return -EINVAL;
    if (rule_id >= (uint32_t)acl_count)
        return -ENOENT;

    memcpy(entry, &tomtom_acl[rule_id], sizeof(er_tomtom_acl_entry_t));
    return 0;
}

int er_tomtom_acl_get_all(er_tomtom_acl_entry_t *entries, uint32_t *count)
{
    int i;

    if (!entries || !count)
        return -EINVAL;

    for (i = 0; i < acl_count; i++)
        memcpy(&entries[i], &tomtom_acl[i], sizeof(er_tomtom_acl_entry_t));
    *count = acl_count;
    return 0;
}

int er_tomtom_acl_add(const er_tomtom_acl_entry_t *entry)
{
    if (!entry || acl_count >= ER_TOMTOM_MAX_ACL_ENTRIES)
        return -EINVAL;

    memcpy(&tomtom_acl[acl_count], entry, sizeof(er_tomtom_acl_entry_t));
    tom_info("Added ACL rule: %s (priority=%u)\n",
             entry->name, entry->priority);
    acl_count++;
    return 0;
}

int er_tomtom_acl_delete(uint32_t rule_id)
{
    if (rule_id >= (uint32_t)acl_count)
        return -EINVAL;

    acl_count--;
    if (rule_id < (uint32_t)acl_count)
        memmove(&tomtom_acl[rule_id], &tomtom_acl[rule_id + 1],
                (acl_count - rule_id) * sizeof(er_tomtom_acl_entry_t));
    return 0;
}

/* ==================== Init/Exit ==================== */

static int __init tomtom_er_init(void)
{
    return er_tomtom_asic_init();
}

static void __exit tomtom_er_exit(void)
{
    er_tomtom_asic_exit();
}

module_init(tomtom_er_init);
module_exit(tomtom_er_exit);
