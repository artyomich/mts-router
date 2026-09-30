/* SPDX-License-Identifier: GPL-2.0 */
/*
 * tofino2_telemetry.h - Telemetry and Monitoring Interface
 *
 * MTS-CR-9000 Core Router Tofino 2 Driver
 *
 * Copyright (c) 2024 MTS Router Project
 */

#ifndef TOFINO2_TELEMETRY_H
#define TOFINO2_TELEMETRY_H

#include <linux/types.h>
#include <linux/ktime.h>

/* ============================================================================ */
/* Constants                                                                    */
/* ============================================================================ */

#define TOFINO2_TELEM_MAX_HIST_ENTRIES 3600   /* 1 hour at 1s interval */
#define TOFINO2_TELEM_DEFAULT_INTERVAL 1       /* Default 1 second */
#define TOFINO2_TELEM_MAX_INTERVAL     3600    /* Max 1 hour */
#define TOFINO2_TELEM_MIN_INTERVAL     1       /* Min 1 second */
#define TOFINO2_TELEM_MAX_FANS         16
#define TOFINO2_TELEM_MAX_VOLTAGE      16
#define TOFINO2_TELEM_MAX_TEMP         16
#define TOFINO2_TELEM_MAX_POWER        8
#define TOFINO2_TELEM_NAME_LEN         32
#define TOFINO2_TELEM_SNAPSHOT_INTERVAL 100000  /* 100us in nanoseconds */

/* ============================================================================ */
/* Enums                                                                        */
/* ============================================================================ */

/**
 * enum tofino2_telem_type - Telemetry data types
 */
enum tofino2_telem_type {
    TOFINO2_TELEM_NONE         = 0,
    TOFINO2_TELEM_COUNTER      = 1,
    TOFINO2_TELEM_GAUGE        = 2,
    TOFINO2_TELEM_GAUGE64      = 3,
    TOFINO2_TELEM_TIMETICKS    = 4,
    TOFINO2_TELEM_OCTET        = 5,
    TOFINO2_TELEM_STRING       = 6,
    TOFINO2_TELEM_TABLE        = 7,
    TOFINO2_TELEM_STATUS       = 8,
    TOFINO2_TELEM_UNSIGNED32   = 9,
    TOFINO2_TELEM_UNSIGNED64   = 10,
    TOFINO2_TELEM_INT32        = 11,
    TOFINO2_TELEM_INT64        = 12,
    TOFINO2_TELEM_FLOAT        = 13,
    TOFINO2_TELEM_DOUBLE       = 14,
    TOFINO2_TELEM_BOOLEAN      = 15,
    TOFINO2_TELEM_MAC          = 16,
    TOFINO2_TELEM_IPv4         = 17,
    TOFINO2_TELEM_IPv6         = 18,
    TOFINO2_TELEM_TIMESTAMP    = 19,
    TOFINO2_TELEM_DURATION     = 20
};

/**
 * enum tofino2_telem_source - Telemetry data source
 */
enum tofino2_telem_source {
    TOFINO2_TELEM_SRC_UNKNOWN  = 0,
    TOFINO2_TELEM_SRC ASIC     = 1,
    TOFINO2_TELEM_SRC_PHY      = 2,
    TOFINO2_TELEM_SRC_DMA      = 3,
    TOFINO2_TELEM_SRC_PCIE     = 4,
    TOFINO2_TELEM_SRC_CPU      = 5,
    TOFINO2_TELEM_SRC_MEMORY   = 6,
    TOFINO2_TELEM_SRC_POWER    = 7,
    TOFINO2_TELEM_SRC_THERMAL  = 8,
    TOFINO2_TELEM_SRC_FAN      = 9,
    TOFINO2_TELEM_SRC_VENDOR   = 10
};

/**
 * enum tofino2_telem_agg - Aggregation type
 */
enum tofino2_telem_agg {
    TOFINO2_TELEM_AGG_NONE     = 0,
    TOFINO2_TELEM_AGG_SUM      = 1,
    TOFINO2_TELEM_AGG_AVG      = 2,
    TOFINO2_TELEM_AGG_MIN      = 3,
    TOFINO2_TELEM_AGG_MAX      = 4,
    TOFINO2_TELEM_AGG_DIFF     = 5,
    TOFINO2_TELEM_AGG_COUNT    = 6,
    TOFINO2_TELEM_AGG_RATE     = 7
};

/* ============================================================================ */
/* Structures                                                                   */
/* ============================================================================ */

/**
 * struct tofino2_telem_value - Single telemetry value
 */
struct tofino2_telem_value {
    uint32_t id;                            /* Metric ID */
    enum tofino2_telem_type type;           /* Value type */
    enum tofino2_telem_source source;       /* Data source */
    enum tofino2_telem_agg agg;             /* Aggregation type */
    uint64_t value;                         /* Current value */
    int64_t value_signed;                   /* Signed value */
    double value_float;                     /* Float value */
    char string_val[TOFINO2_TELEM_NAME_LEN]; /* String value */
    uint64_t timestamp;                     /* Timestamp (nanoseconds) */
    uint64_t last_update;                   /* Last update time */
    uint64_t min_value;                     /* Min observed value */
    uint64_t max_value;                     /* Max observed value */
    uint64_t sum_value;                     /* Sum for averaging */
    uint32_t count;                         /* Sample count */
    uint32_t enabled;                       /* Metric enabled */
    uint32_t valid;                         /* Value valid */
    uint32_t metadata[8];                   /* Vendor metadata */
    uint8_t path[128];                      /* Telemetry subscription path */
    uint32_t path_len;                      /* Path length */
};

/**
 * struct tofino2_telem_table - Telemetry table entry
 */
struct tofino2_telem_table {
    uint32_t id;                            /* Table ID */
    char name[TOFINO2_TELEM_NAME_LEN];      /* Table name */
    uint32_t num_entries;                   /* Table entries */
    uint32_t max_entries;                   /* Max table entries */
    uint32_t key_size;                      /* Key size */
    uint32_t value_size;                    /* Value size */
    uint64_t hits;                          /* Table hits */
    uint64_t misses;                        /* Table misses */
    uint64_t updates;                       /* Table updates */
    uint64_t deletes;                       /* Table deletes */
    uint64_t allocs;                        /* Allocations */
    uint64_t frees;                         /* Frees */
    uint64_t timestamp;                     /* Last update time */
    uint8_t enabled;                        /* Table enabled */
    uint8_t pad[3];
};

/**
 * struct tofino2_telem_snapshot - Complete telemetry snapshot
 */
struct tofino2_telem_snapshot {
    uint64_t timestamp;                     /* Snapshot timestamp */
    uint64_t uptime_ns;                     /* Device uptime */
    uint64_t boot_time;                     /* Boot time */

    /* CPU */
    uint32_t cpu_usage[16];                 /* CPU core usage % */
    uint32_t num_cpu;                       /* Number of CPU cores */
    uint64_t cpu_cycles[16];               /* CPU cycles per core */
    uint64_t cpu_instructions[16];         /* CPU instructions per core */
    uint64_t cpu_cache_miss[16];           /* Cache misses per core */
    uint64_t cpu_branch_miss[16];          /* Branch misses per core */
    uint32_t cpu_freq_mhz[16];             /* CPU frequency MHz */
    uint32_t cpu_temp[16];                 /* CPU temp C */

    /* Memory */
    uint64_t memory_total;                  /* Total memory (MB) */
    uint64_t memory_used;                   /* Used memory (MB) */
    uint64_t memory_free;                   /* Free memory (MB) */
    uint64_t memory_buffers;                /* Buffer memory (MB) */
    uint64_t memory_cached;                 /* Cached memory (MB) */
    uint64_t memory_swap_total;             /* Swap total (MB) */
    uint64_t memory_swap_used;              /* Swap used (MB) */
    uint64_t memory_swap_free;              /* Swap free (MB) */
    uint32_t memory_usage_pct;              /* Memory usage % */

    /* Temperature */
    uint32_t num_temp_sensors;              /* Number of temp sensors */
    int32_t temp_sensors[TOFINO2_TELEM_MAX_TEMP]; /* Temperatures */
    char temp_names[TOFINO2_TELEM_MAX_TEMP][TOFINO2_TELEM_NAME_LEN];

    /* Voltage */
    uint32_t num_voltage_sensors;           /* Number of voltage sensors */
    uint16_t voltage_sensors[TOFINO2_TELEM_MAX_VOLTAGE]; /* Voltages mV */
    char voltage_names[TOFINO2_TELEM_MAX_VOLTAGE][TOFINO2_TELEM_NAME_LEN];

    /* Power */
    uint32_t num_power_sensors;             /* Number of power sensors */
    uint32_t power_sensors[TOFINO2_TELEM_MAX_POWER]; /* Power mW */
    uint32_t power_limit_mw;                /* Power limit mW */
    char power_names[TOFINO2_TELEM_MAX_POWER][TOFINO2_TELEM_NAME_LEN];

    /* Fan */
    uint32_t num_fans;                      /* Number of fans */
    uint32_t fan_rpm[TOFINO2_TELEM_MAX_FANS]; /* Fan RPM */
    char fan_names[TOFINO2_TELEM_MAX_FANS][TOFINO2_TELEM_NAME_LEN];

    /* PCIe */
    uint64_t pcie_rx_bytes;                 /* PCIe RX bytes */
    uint64_t pcie_tx_bytes;                 /* PCIe TX bytes */
    uint32_t pcie_rx_errors;                /* PCIe RX errors */
    uint32_t pcie_tx_errors;                /* PCIe TX errors */
    uint32_t pcie_link_width;               /* PCIe link width */
    uint32_t pcie_link_speed;               /* PCIe link speed GT/s */
    uint32_t pcie_link_status;              /* Link status */

    /* DMA */
    uint64_t dma_completions;               /* DMA completions */
    uint64_t dma_failures;                  /* DMA failures */
    uint64_t dma_bytes;                     /* DMA bytes transferred */
    uint32_t dma_active_channels;           /* Active DMA channels */
    uint32_t dma_max_channels;              /* Max DMA channels */

    /* Interrupts */
    uint64_t irq_total;                     /* Total interrupts */
    uint64_t irq_nmi;                       /* NMI count */
    uint64_t irq_sched;                     /* Scheduler interrupts */
    uint64_t irq_timer;                     /* Timer interrupts */
    uint32_t irq_pending;                   /* Pending interrupts */
    uint32_t irq_disabled;                  /* Disabled interrupts */

    /* Packet stats */
    uint64_t pkt_in;                        /* Packets received */
    uint64_t pkt_out;                       /* Packets transmitted */
    uint64_t byte_in;                       /* Bytes received */
    uint64_t byte_out;                      /* Bytes transmitted */
    uint64_t drop_in;                       /* Drops received */
    uint64_t drop_out;                      /* Drops transmitted */
    uint64_t error_in;                      /* Errors received */
    uint64_t error_out;                     /* Errors transmitted */
    uint64_t pkt_multicast_in;              /* Multicast packets in */
    uint64_t pkt_multicast_out;             /* Multicast packets out */
    uint64_t pkt_broadcast_in;              /* Broadcast packets in */
    uint64_t pkt_broadcast_out;             /* Broadcast packets out */
    uint64_t pkt_unicast_in;                /* Unicast packets in */
    uint64_t pkt_unicast_out;               /* Unicast packets out */

    /* Error stats */
    uint64_t crc_errors;                    /* CRC errors */
    uint64_t alignment_errors;              /* Alignment errors */
    uint64_t overflow_errors;               /* Overflow errors */
    uint64_t underflow_errors;              /* Underflow errors */
    uint64_t parity_errors;                 /* Parity errors */
    uint64_t ecc_errors;                    /* ECC errors */
    uint64_t fatal_errors;                  /* Fatal errors */
    uint64_t recoverable_errors;            /* Recoverable errors */

    /* Table stats */
    uint32_t num_tables;                    /* Number of monitored tables */
    struct tofino2_telem_table tables[256]; /* Table data */

    /* Pipeline stats */
    uint64_t pipeline_latency[32];          /* Pipeline stage latency */
    uint64_t pipeline_bytes[32];            /* Pipeline stage bytes */
    uint32_t num_pipeline_stages;           /* Pipeline stages */

    /* Firmware */
    uint32_t fw_version;                    /* Firmware version */
    uint32_t fw_build;                      /* Firmware build */
    uint32_t fw_errors;                     /* Firmware errors */
    uint32_t fw_warnings;                   /* Firmware warnings */
    uint32_t fw_config_changes;             /* Config changes */
    uint8_t fw_status;                      /* Firmware status */
    uint8_t fw_update_pending;              /* Update pending */
    uint8_t fw_update_progress;             /* Update progress % */
    uint8_t pad;

    /* Health */
    uint32_t health_score;                  /* Health score 0-100 */
    uint32_t fault_count;                   /* Fault count */
    uint32_t warning_count;                 /* Warning count */
    uint32_t critical_count;                /* Critical count */
    uint8_t overall_status;                 /* Overall status */
    uint8_t predicted_failure;              /* Predicted failure */
    uint8_t maintenance_mode;               /* Maintenance mode */
    uint8_t pad2[5];
};

/**
 * struct tofino2_telem_subscription - Telemetry subscription
 */
struct tofino2_telem_subscription {
    uint32_t id;                            /* Subscription ID */
    char name[TOFINO2_TELEM_NAME_LEN];      /* Subscription name */
    uint32_t path_len;                      /* Path length */
    uint8_t path[128];                      /* Telemetry path */
    uint32_t agg_interval;                  /* Aggregation interval (s) */
    uint32_t sample_interval;               /* Sample interval (us) */
    uint32_t encoding;                      /* Encoding (0=JSON, 1=PROTO, 2=CSV) */
    uint32_t enabled;                       /* Subscription enabled */
    uint64_t samples_sent;                  /* Samples sent */
    uint64_t bytes_sent;                    /* Bytes sent */
    uint64_t errors;                        /* Errors */
    uint64_t last_sample;                   /* Last sample time */
    uint64_t last_send;                     /* Last send time */
};

/**
 * struct tofino2_telem_stream - Telemetry streaming state
 */
struct tofino2_telem_stream {
    uint32_t id;                            /* Stream ID */
    char name[TOFINO2_TELEM_NAME_LEN];      /* Stream name */
    uint32_t encoding;                      /* Output encoding */
    uint32_t interval;                      /* Send interval (ms) */
    uint32_t enabled;                       /* Stream enabled */
    uint64_t total_samples;                 /* Total samples */
    uint64_t total_bytes;                   /* Total bytes */
    uint64_t errors;                        /* Errors */
    uint64_t last_sample;                   /* Last sample time */
    uint64_t last_send;                     /* Last send time */
    struct tofino2_telem_subscription *sub; /* Associated subscription */
};

/* ============================================================================ */
/* Telemetry API                                                                  */
/* ============================================================================ */

/* Initialization */
int tofino2_telem_init(void);
void tofino2_telem_exit(void);
int tofino2_telem_snapshot_create(struct tofino2_telem_snapshot *snap);
void tofino2_telem_snapshot_free(struct tofino2_telem_snapshot *snap);
struct tofino2_telem_snapshot *tofino2_telem_snapshot_alloc(void);

/* Snapshot operations */
int tofino2_telem_capture_snapshot(struct tofino2_telem_snapshot *snap);
int tofino2_telem_copy_snapshot(const struct tofino2_telem_snapshot *src,
                                 struct tofino2_telem_snapshot *dst);
int tofino2_telem_merge_snapshots(const struct tofino2_telem_snapshot *snap1,
                                   const struct tofino2_telem_snapshot *snap2,
                                   struct tofino2_telem_snapshot *result);
int tofino2_telem_diff_snapshots(const struct tofino2_telem_snapshot *snap1,
                                  const struct tofino2_telem_snapshot *snap2,
                                  struct tofino2_telem_snapshot *diff);

/* History */
int tofino2_telem_history_record(const struct tofino2_telem_snapshot *snap);
const struct tofino2_telem_snapshot *tofino2_telem_history_get(uint32_t idx);
uint32_t tofino2_telem_history_count(void);
int tofino2_telem_history_clear(void);
int tofino2_telem_history_export(const char *filename,
                                  uint32_t start_idx, uint32_t count);
int tofino2_telem_history_import(const char *filename,
                                  uint32_t *start_idx, uint32_t *count);

/* Values */
int tofino2_telem_value_get(uint32_t id, struct tofino2_telem_value *val);
int tofino2_telem_value_set(uint32_t id, const struct tofino2_telem_value *val);
int tofino2_telem_value_update(uint32_t id, uint64_t new_value);
int tofino2_telem_value_get_all(struct tofino2_telem_value **vals,
                                 uint32_t *count);
int tofino2_telem_value_get_by_type(enum tofino2_telem_type type,
                                     struct tofino2_telem_value **vals,
                                     uint32_t *count);
int tofino2_telem_value_get_by_source(enum tofino2_telem_source src,
                                       struct tofino2_telem_value **vals,
                                       uint32_t *count);

/* Tables */
int tofino2_telem_table_get(uint32_t id,
                             struct tofino2_telem_table *table);
int tofino2_telem_table_update(uint32_t id,
                                const struct tofino2_telem_table *table);
int tofino2_telem_table_get_all(struct tofino2_telem_table **tables,
                                 uint32_t *count);

/* Subscriptions */
int tofino2_telem_subscription_create(uint32_t *sub_id,
                                       const uint8_t *path,
                                       uint32_t path_len,
                                       uint32_t interval);
int tofino2_telem_subscription_delete(uint32_t sub_id);
int tofino2_telem_subscription_get(uint32_t sub_id,
                                    struct tofino2_telem_subscription *sub);
int tofino2_telem_subscription_set(uint32_t sub_id,
                                    const struct tofino2_telem_subscription *sub);
int tofino2_telem_subscription_get_all(struct tofino2_telem_subscription **subs,
                                        uint32_t *count);

/* Streams */
int tofino2_telem_stream_create(uint32_t *stream_id,
                                 const char *name,
                                 uint32_t encoding,
                                 uint32_t interval);
int tofino2_telem_stream_delete(uint32_t stream_id);
int tofino2_telem_stream_get(uint32_t stream_id,
                              struct tofino2_telem_stream *stream);
int tofino2_telem_stream_set(uint32_t stream_id,
                              const struct tofino2_telem_stream *stream);
int tofino2_telem_stream_send(uint32_t stream_id,
                               struct tofino2_telem_snapshot *snap);
int tofino2_telem_stream_get_all(struct tofino2_telem_stream **streams,
                                  uint32_t *count);

/* Polling */
int tofino2_telem_poll_start(uint32_t interval_us);
int tofino2_telem_poll_stop(void);
int tofino2_telem_poll_set_interval(uint32_t interval_us);
uint32_t tofino2_telem_poll_get_interval(void);

/* Export/Import */
int tofino2_telem_snapshot_export_json(const struct tofino2_telem_snapshot *snap,
                                        char *buf, uint32_t buf_size);
int tofino2_telem_snapshot_import_json(struct tofino2_telem_snapshot *snap,
                                        const char *buf, uint32_t buf_size);
int tofino2_telem_snapshot_export_protobuf(const struct tofino2_telem_snapshot *snap,
                                            uint8_t *buf, uint32_t buf_size,
                                            uint32_t *out_size);
int tofino2_telem_snapshot_import_protobuf(struct tofino2_telem_snapshot *snap,
                                            const uint8_t *buf, uint32_t buf_size);

/* Debug */
int tofino2_telem_debug_dump(const struct tofino2_telem_snapshot *snap,
                              FILE *fp);
int tofino2_telem_debug_stats(uint64_t *total_samples,
                               uint64_t *total_errors);

#endif /* TOFINO2_TELEMETRY_H */
