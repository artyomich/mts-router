/* SPDX-License-Identifier: GPL-2.0 */
/*
 * tofino2.h - Intel Tofino 2 ASIC Driver Main Header
 *
 * MTS-CR-9000 Core Router Tofino 2 Driver
 *
 * Copyright (c) 2024 MTS Router Project
 * Author: Firmware Agent
 *
 * This driver provides kernel-space support for Intel Tofino 2 P4-programmable
 * Ethernet switching ASIC. It implements port management, P4 pipeline control,
 * counter/meter management, and telemetry interfaces.
 */

#ifndef TOFINO2_H
#define TOFINO2_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

/* ============================================================================ */
/* Constants                                                                    */
/* ============================================================================ */

#define TOFINO2_MAGIC 0x54464E32  /* "TFN2" */
#define TOFINO2_MAX_DEVICES 4
#define TOFINO2_MAX_PORTS_PER_DEV 128
#define TOFINO2_MAX_PIPES 256
#define TOFINO2_MAX_TABLES 4096
#define TOFINO2_MAX_ENTRIES_PER_TABLE 0x100000
#define TOFINO2_MAX_COUNTERS 8192
#define TOFINO2_MAX_METERS 4096
#define TOFINO2_DRIVER_VERSION "1.0.0"
#define TOFINO2_PCI_VENDOR_ID 0x8086
#define TOFINO2_PCI_DEVICE_ID 0x2690
#define TOFINO2_REG_BAR 0
#define TOFINO2_REG_SPACE_SIZE 0x400000
#define TOFINO2_MAX_DMA_CHANNELS 64
#define TOFINO2_DMA_BUF_SIZE PAGE_SIZE * 64
#define TOFINO2_MAX_FABRIC_PORTS 8
#define TOFINO2_PORT_NAME_LEN 32
#define TOFINO2_DEV_NAME_LEN 32
#define TOFINO2_PIPE_NAME_LEN 64
#define TOFINO2_TABLE_NAME_LEN 64
#define TOFINO2_POLL_INTERVAL_HZ 10
#define TOFINO2_FEC_NONE 0
#define TOFINO2_FEC_RS 1
#define TOFINO2_FEC_BCH 2
#define TOFINO2_MAX_RX_QUEUES 64
#define TOFINO2_MAX_TX_QUEUES 64

/* ============================================================================ */
/* Enums                                                                        */
/* ============================================================================ */

/**
 * enum tofino2_state - Device operational states
 */
enum tofino2_state {
    TOFINO2_STATE_INIT,
    TOFINO2_STATE_RESET,
    TOFINO2_STATE_READY,
    TOFINO2_STATE_PROBING,
    TOFINO2_STATE_RUNNING,
    TOFINO2_STATE_SUSPEND,
    TOFINO2_STATE_RESUME,
    TOFINO2_STATE_ERROR,
    TOFINO2_STATE_STOPPED,
    TOFINO2_STATE_FAULT
};

/**
 * enum tofino2_port_type - Physical port connector types
 */
enum tofino2_port_type {
    TOFINO2_PORT_SFP,
    TOFINO2_PORT_SFP28,
    TOFINO2_PORT_QSFP,
    TOFINO2_PORT_QSFP28,
    TOFINO2_PORT_QSFP56,
    TOFINO2_PORT_QSFP_DD,
    TOFINO2_PORT_OSFP,
    TOFINO2_PORT_XFP,
    TOFINO2_PORT_XFI,
    TOFINO2_PORT_SFI,
    TOFINO2_PORT_HGMII,
    TOFINO2_PORT_XLBI,
    TOFINO2_PORT_CFP,
    TOFINO2_PORT_CFP2,
    TOFINO2_PORT_CFP4,
    TOFINO2_PORT_CFP8,
    TOFINO2_PORT_QSGMII,
    TOFINO2_PORT_UNKNOWN
};

/**
 * enum tofino2_port_speed - Supported data rates
 */
enum tofino2_port_speed {
    TOFINO2_SPEED_10M   = 10000,
    TOFINO2_SPEED_100M  = 100000,
    TOFINO2_SPEED_1G    = 1000000,
    TOFINO2_SPEED_2_5G  = 2500000,
    TOFINO2_SPEED_5G    = 5000000,
    TOFINO2_SPEED_10G   = 10000000,
    TOFINO2_SPEED_25G   = 25000000,
    TOFINO2_SPEED_40G   = 40000000,
    TOFINO2_SPEED_50G   = 50000000,
    TOFINO2_SPEED_100G  = 100000000,
    TOFINO2_SPEED_200G  = 200000000,
    TOFINO2_SPEED_400G  = 400000000,
    TOFINO2_SPEED_AUTO  = 0
};

/**
 * enum tofino2_port_mode - Logical port operation modes
 */
enum tofino2_port_mode {
    TOFINO2_MODE_MEDIA,
    TOFINO2_MODE_NATIVE,
    TOFINO2_MODE_SPAN,
    TOFINO2_MODE_LOOPBACK
};

/**
 * enum tofino2_fec_mode - Forward error correction modes
 */
enum tofino2_fec_mode {
    TOFINO2_FEC_DISABLED,
    TOFINO2_FEC_RS,
    TOFINO2_FEC_BCH,
    TOFINO2_FEC_AUTO
};

/**
 * enum tofino2_pipeline_stage - P4 pipeline processing stages
 */
enum tofino2_pipeline_stage {
    TOFINO2_STAGE_DEMAP,
    TOFINO2_STAGE_PARSE_VLAN,
    TOFINO2_STAGE_L2_LOOKUP,
    TOFINO2_STAGE_QOS_IN,
    TOFINO2_STAGE_ACL,
    TOFINO2_STAGE_METER,
    TOFINO2_STAGE_ROUTE_LOOKUP,
    TOFINO2_STAGE_MPLS_LOOKUP,
    TOFINO2_STAGE_SRV6_PROCESS,
    TOFINO2_STAGE_ADJACENCY,
    TOFINO2_STAGE_QOS_OUT,
    TOFINO2_STAGE_DEMAP_OUT,
    TOFINO2_STAGE_MAX
};

/**
 * enum tofino2_table_type - P4 table classification
 */
enum tofino2_table_type {
    TOFINO2_TABLE_TCAM,
    TOFINO2_TABLE_LPM,
    TOFINO2_TABLE_DIRECT_MAP,
    TOFINO2_TABLE_INDIRECT_MAP,
    TOFINO2_TABLE_COUNTER,
    TOFINO2_TABLE_METER,
    TOFINO2_TABLE_REGISTER
};

/**
 * enum tofino2_counter_type - Counter types
 */
enum tofino2_counter_type {
    TOFINO2_COUNTER_64BIT,
    TOFINO2_COUNTER_128BIT,
    TOFINO2_COUNTER_PORT,
    TOFINO2_COUNTER_SYSTEM,
    TOFINO2_COUNTER_DIGEST
};

/**
 * enum tofino2_meter_color - Color-aware action mode
 */
enum tofino2_meter_color {
    TOFINO2_COLOR_GREEN,
    TOFINO2_COLOR_YELLOW,
    TOFINO2_COLOR_RED,
    TOFINO2_COLOR_UNCOLORED
};

/**
 * enum tofino2_link_status - PHY link status codes
 */
enum tofino2_link_status {
    TOFINO2_LINK_DOWN,
    TOFINO2_LINK_UP,
    TOFINO2_LINK_INIT,
    TOFINO2_LINK_AUTONEG,
    TOFINO2_LINK_FAULT
};

/**
 * enum tofino2_tx_mode - TX queue operation modes
 */
enum tofino2_tx_mode {
    TOFINO2_TX_MODE_DMA,
    TOFINO2_TX_MODE_POLL,
    TOFINO2_TX_MODE_INTERRUPT
};

/**
 * enum tofino2_rx_mode - RX queue operation modes
 */
enum tofino2_rx_mode {
    TOFINO2_RX_MODE_DMA,
    TOFINO2_RX_MODE_POLL,
    TOFINO2_RX_MODE_INTERRUPT
};

/* ============================================================================ */
/* Structures                                                                   */
/* ============================================================================ */

/**
 * struct tofino2_port - Port configuration and state
 */
struct tofino2_port {
    uint32_t id;                            /* Physical port ID */
    uint32_t index;                         /* Port index in device */
    char name[TOFINO2_PORT_NAME_LEN];       /* Port name */
    enum tofino2_port_type type;            /* Connector type */
    enum tofino2_port_speed speed;          /* Current speed */
    enum tofino2_port_mode mode;            /* Operation mode */
    enum tofino2_fec_mode fec;              /* FEC mode */
    uint8_t duplex;                         /* 1 = full, 0 = half */
    uint8_t autoneg;                        /* Auto-negotiation enabled */
    uint8_t pause;                          /* Flow control pause */
    uint8_t loopback;                       /* Loopback mode */
    enum tofino2_link_status link_status;   /* Link status */
    uint8_t enabled;                        /* Port enabled */
    uint8_t pad[3];

    /* PHY diagnostics */
    int32_t tx_power_dbm;                   /* TX laser power (dBm) */
    int32_t rx_power_dbm;                   /* RX laser power (dBm) */
    int32_t temperature_c;                  /* Module temperature */
    uint32_t voltage_mv;                    /* Module voltage (mV) */
    uint32_t bias_current_ma;               /* Laser bias current (mA) */
    uint8_t tx_fault;                       /* TX fault indicator */
    uint8_t rx_loss;                        /* RX loss of signal */
    uint8_t present;                        /* Module present */
    uint8_t pad2;

    /* Statistics */
    uint64_t rx_bytes;
    uint64_t rx_packets;
    uint64_t rx_errors;
    uint64_t rx_drops;
    uint64_t rx_crc_errors;
    uint64_t rx_oversize;
    uint64_t rx_undersize;
    uint64_t rx_mii_errors;
    uint64_t tx_bytes;
    uint64_t tx_packets;
    uint64_t tx_errors;
    uint64_t tx_drops;
    uint64_t tx_urgent;
    uint64_t tx_collisions;
    uint64_t tx_deferred;
    uint64_t tx_late_collision;
    uint64_t last_update;

    /* TX/RX queues */
    uint32_t tx_queues;
    uint32_t rx_queues;
    uint32_t *tx_qmap;
    uint32_t *rx_qmap;
};

/**
 * struct tofino2_pipeline - P4 pipeline configuration
 */
struct tofino2_pipeline {
    uint32_t id;                            /* Pipeline ID */
    char name[TOFINO2_PIPE_NAME_LEN];       /* Pipeline name */
    uint32_t num_tables;                    /* Number of tables */
    uint32_t num_actions;                   /* Number of actions */
    uint32_t num_externs;                   /* Number of extern objects */
    uint32_t max_entries;                   /* Max entries per table */
    uint32_t config_hash;                   /* Configuration hash */
    uint32_t program_length;                /* P4 program size (bytes) */
    uint8_t *program;                       /* P4 program binary */
    uint8_t compiled;                       /* Pipeline compiled flag */
    uint8_t loaded;                         /* Pipeline loaded flag */
    uint8_t pad[2];
};

/**
 * struct tofino2_table - P4 table definition
 */
struct tofino2_table {
    uint32_t id;                            /* Table ID */
    uint32_t pipeline_id;                   /* Owning pipeline */
    char name[TOFINO2_TABLE_NAME_LEN];      /* Table name */
    enum tofino2_table_type type;           /* Table type */
    uint32_t key_size;                      /* Key size in bytes */
    uint32_t action_size;                   /* Action data size */
    uint32_t max_entries;                   /* Maximum entries */
    uint32_t current_entries;               /* Current entries */
    uint32_t default_action;                /* Default action ID */
    uint8_t constant_fields;                /* Has constant fields */
    uint8_t size_optimize;                  /* Size optimization enabled */
    uint8_t preallocate;                    /* Pre-allocate entries */
    uint8_t pad;
};

/**
 * struct tofino2_table_entry - Table entry for TCAM/LPM/Map tables
 */
struct tofino2_table_entry {
    uint32_t table_id;                      /* Table ID */
    uint32_t entry_id;                      /* Entry ID */
    uint32_t key_len;                       /* Key length */
    uint8_t *key;                           /* Match key */
    uint32_t action_id;                     /* Action ID */
    uint32_t action_data_len;               /* Action data length */
    uint8_t *action_data;                   /* Action data */
    uint32_t priority;                      /* TCAM priority */
    uint32_t counter_id;                    /* Associated counter */
    uint32_t meter_id;                      /* Associated meter */
    uint64_t timestamp;                     /* Entry timestamp */
    uint32_t hit_count;                     /* Match hit count */
    uint32_t miss_count;                    /* Miss count */
    uint32_t age;                           /* Entry age (seconds) */
    uint8_t valid;                          /* Entry valid */
    uint8_t metadata[32];                   /* User metadata */
};

/**
 * struct tofino2_counter - Counter definition and state
 */
struct tofino2_counter {
    uint32_t id;                            /* Counter ID */
    enum tofino2_counter_type type;         /* Counter type */
    uint64_t value;                         /* Current counter value */
    uint64_t base_value;                    /* Base/reset value */
    uint64_t max_value;                     /* Max value since last read */
    uint64_t timestamp;                     /* Last update timestamp */
    uint8_t enabled;                        /* Counter enabled */
    uint8_t pad[3];
};

/**
 * struct tofino2_meter - Rate meter definition
 */
struct tofino2_meter {
    uint32_t id;                            /* Meter ID */
    enum tofino2_meter_color color_mode;    /* Color mode */
    uint32_t cir;                           /* Committed info rate (bps) */
    uint32_t pir;                           /* Peak info rate (bps) */
    uint32_t cbs;                           /* Committed burst size (bytes) */
    uint32_t pbs;                           /* Peak burst size (bytes) */
    uint64_t last_update;                   /* Last update timestamp */
    uint32_t current_credits;               /* Current token credits */
    uint8_t enabled;                        /* Meter enabled */
    uint8_t pad[3];
};

/**
 * struct tofino2_telemetry - Telemetry data aggregation
 */
struct tofino2_telemetry {
    uint64_t timestamp;                     /* Telemetry timestamp */
    uint64_t uptime;                        /* Device uptime (seconds) */
    uint32_t cpu_usage;                     /* CPU usage percentage */
    uint32_t memory_usage;                  /* Memory usage percentage */
    uint32_t memory_free;                   /* Free memory (MB) */
    uint32_t temperature_core;              /* Core temperature (C) */
    uint32_t temperature_board;             /* Board temperature (C) */
    uint32_t voltage_core_mv;               /* Core voltage (mV) */
    uint32_t voltage_aux_mv;                /* Auxiliary voltage (mV) */
    uint32_t fan_rpm[8];                    /* Fan RPMs */
    uint32_t num_fans;                      /* Number of fans */
    uint64_t pkt_in;                        /* Packets received */
    uint64_t pkt_out;                       /* Packets transmitted */
    uint64_t byte_in;                       /* Bytes received */
    uint64_t byte_out;                      /* Bytes transmitted */
    uint64_t drop_in;                       /* Drops received */
    uint64_t drop_out;                      /* Drops transmitted */
    uint64_t error_in;                      /* Errors received */
    uint64_t error_out;                     /* Errors transmitted */
    uint32_t irq_count;                     /* IRQ count */
    uint32_t dma_completions;               /* DMA completions */
    uint32_t dma_failures;                  /* DMA failures */
    uint32_t config_changes;                /* Configuration changes */
    uint32_t firmware_errors;               /* Firmware error count */
    uint32_t firmware_warn;                 /* Firmware warning count */
};

/**
 * struct tofino2_dma_channel - DMA channel configuration
 */
struct tofino2_dma_channel {
    uint32_t id;                            /* Channel ID */
    uint32_t direction;                     /* DMA direction */
    uint32_t status;                        /* Channel status */
    uint64_t src_addr;                      /* Source address */
    uint64_t dst_addr;                      /* Destination address */
    uint32_t length;                        /* Transfer length */
    uint32_t completed;                     /* Bytes completed */
    uint32_t irq;                           /* Associated IRQ */
    uint8_t enabled;                        /* Channel enabled */
    uint8_t pad[3];
};

/**
 * struct tofino2_fabric_port - Fabric connection port
 */
struct tofino2_fabric_port {
    uint32_t id;                            /* Fabric port ID */
    uint32_t card_id;                       /* Line card ID */
    uint32_t port_index;                    /* Port index on card */
    enum tofino2_port_speed speed;          /* Fabric speed */
    enum tofino2_link_status link;          /* Link status */
    uint64_t rx_bytes;                      /* Fabric RX bytes */
    uint64_t tx_bytes;                      /* Fabric TX bytes */
    uint64_t rx_errors;                     /* Fabric RX errors */
    uint64_t tx_errors;                     /* Fabric TX errors */
};

/**
 * struct tofino2_device - Main Tofino 2 device structure
 */
struct tofino2_device {
    uint32_t magic;                         /* Magic number for validation */
    uint32_t id;                            /* Device ID */
    uint32_t pci_fn;                        /* PCI function number */
    uint32_t pci_dev;                       /* PCI device pointer */
    char name[TOFINO2_DEV_NAME_LEN];        /* Device name */
    char serial[16];                        /* Serial number */
    enum tofino2_state state;               /* Current state */
    uint32_t revision;                      /* Silicon revision */
    uint32_t pkg_type;                      /* Package type */
    uint32_t num_ports;                     /* Number of ports */
    uint32_t num_fabric_ports;              /* Number of fabric ports */
    uint32_t num_pipes;                     /* Number of pipelines */

    /* Resources */
    struct tofino2_port *ports;             /* Port array */
    struct tofino2_fabric_port *fabric;     /* Fabric ports */
    struct tofino2_pipeline *pipelines;     /* Pipeline array */
    struct tofino2_table *tables;           /* Table array */
    struct tofino2_table_entry **entries;   /* Entries per table */
    struct tofino2_counter *counters;       /* Counters array */
    struct tofino2_meter *meters;           /* Meters array */
    struct tofino2_dma_channel *dma;        /* DMA channels */

    /* Memory mapping */
    void __iomem *reg_base;                 /* Register base address */
    resource_size_t reg_phys;               /* Register physical address */
    unsigned long reg_size;                 /* Register region size */
    void *mmio_base;                        /* MMIO base for tables */
    unsigned long mmio_size;                /* MMIO size */

    /* DMA coherency */
    dma_addr_t dma_handle;                  /* DMA mapping handle */
    void *dma_buf;                          /* DMA coherent buffer */
    uint32_t dma_buf_size;                  /* DMA buffer size */

    /* Synchronization */
    struct mutex lock;                      /* Main device lock */
    struct rw_semaphore sem;               /* Read-write semaphore */
    spinlock_t stat_lock;                   /* Statistics lock */
    spinlock_t event_lock;                  /* Event queue lock */

    /* Work processing */
    struct workqueue_struct *wq;            /* Event workqueue */
    struct work_struct reset_work;          /* Reset work */
    struct work_struct poll_work;           /* Polling work */
    struct timer_list timer;                /* Polling timer */
    struct delayed_work fw_update_work;     /* Firmware update work */

    /* Network interface */
    struct net_device *netdev;              /* Netdev for sysfs */
    struct net_device_stats stats;          /* Network stats */
    struct ethtool_ops ethtool_ops;         /* Ethtool operations */

    /* Sysfs and debugfs */
    struct device *dev;                     /* Device structure */
    struct dentry *debugfs_dir;             /* Debugfs directory */
    struct seq_file *debugfs_seq;           /* Debugfs seq file */

    /* Interrupt */
    int irq;                                /* Main IRQ */
    int *irq_vectors;                       /* IRQ vector array */
    uint32_t num_irq_vectors;               /* Number of IRQ vectors */

    /* Firmware */
    uint32_t fw_version;                    /* Firmware version */
    uint32_t fw_build;                      /* Firmware build number */
    uint8_t *fw_data;                       /* Loaded firmware data */
    uint32_t fw_size;                       /* Firmware size */
    uint8_t fw_loaded;                      /* Firmware loaded flag */

    /* Telemetry */
    struct tofino2_telemetry telemetry;     /* Current telemetry */
    struct tofino2_telemetry *telem_hist;   /* Telemetry history */
    uint32_t telem_hist_size;               /* History size */
    uint32_t telem_hist_idx;                /* Current history index */
    uint32_t telem_interval;                /* Telemetry interval (seconds) */

    /* Event queue */
    uint32_t *event_queue;                  /* Event ring buffer */
    uint32_t event_head;                    /* Ring head pointer */
    uint32_t event_tail;                    /* Ring tail pointer */
    uint32_t event_size;                    /* Ring buffer size */
    uint32_t event_count;                   /* Pending events */

    /* Power management */
    uint32_t power_state;                   /* Power state */
    uint32_t power_limit_mw;                /* Power limit (mW) */
    uint32_t power_current_mw;              /* Current power (mW) */

    /* Private data */
    void *priv;                             /* Driver private data */
};

/* ============================================================================ */
/* IOCTL definitions                                                            */
/* ============================================================================ */

#define TOFINO2_IOC_MAGIC 'T'

#define TOFINO2_IOC_GET_DEV       _IOR(TOFINO2_IOC_MAGIC, 0x00, struct tofino2_device)
#define TOFINO2_IOC_SET_DEV       _IOW(TOFINO2_IOC_MAGIC, 0x01, struct tofino2_device)
#define TOFINO2_IOC_GET_PORT      _IOR(TOFINO2_IOC_MAGIC, 0x10, struct tofino2_port)
#define TOFINO2_IOC_SET_PORT      _IOW(TOFINO2_IOC_MAGIC, 0x11, struct tofino2_port)
#define TOFINO2_IOC_GET_PORTS     _IOR(TOFINO2_IOC_MAGIC, 0x12, struct tofino2_port[])
#define TOFINO2_IOC_GET_PIPE      _IOR(TOFINO2_IOC_MAGIC, 0x20, struct tofino2_pipeline)
#define TOFINO2_IOC_SET_PIPE      _IOW(TOFINO2_IOC_MAGIC, 0x21, struct tofino2_pipeline)
#define TOFINO2_IOC_COMPILE_PIPE  _IOW(TOFINO2_IOC_MAGIC, 0x22, struct tofino2_pipeline)
#define TOFINO2_IOC_LOAD_PIPE     _IOW(TOFINO2_IOC_MAGIC, 0x23, struct tofino2_pipeline)
#define TOFINO2_IOC_UNLOAD_PIPE   _IOW(TOFINO2_IOC_MAGIC, 0x24, uint32_t)
#define TOFINO2_IOC_GET_TABLE     _IOR(TOFINO2_IOC_MAGIC, 0x30, struct tofino2_table)
#define TOFINO2_IOC_SET_TABLE     _IOW(TOFINO2_IOC_MAGIC, 0x31, struct tofino2_table)
#define TOFINO2_IOC_GET_ENTRY     _IOR(TOFINO2_IOC_MAGIC, 0x40, struct tofino2_table_entry)
#define TOFINO2_IOC_SET_ENTRY     _IOW(TOFINO2_IOC_MAGIC, 0x41, struct tofino2_table_entry)
#define TOFINO2_IOC_DEL_ENTRY     _IOW(TOFINO2_IOC_MAGIC, 0x42, uint32_t)
#define TOFINO2_IOC_GET_COUNTER   _IOR(TOFINO2_IOC_MAGIC, 0x50, struct tofino2_counter)
#define TOFINO2_IOC_GET_COUNTERS  _IOR(TOFINO2_IOC_MAGIC, 0x51, struct tofino2_counter[])
#define TOFINO2_IOC_RESET_COUNTER _IOW(TOFINO2_IOC_MAGIC, 0x52, uint32_t)
#define TOFINO2_IOC_GET_METER     _IOR(TOFINO2_IOC_MAGIC, 0x60, struct tofino2_meter)
#define TOFINO2_IOC_SET_METER     _IOW(TOFINO2_IOC_MAGIC, 0x61, struct tofino2_meter)
#define TOFINO2_IOC_GET_TELEMETRY _IOR(TOFINO2_IOC_MAGIC, 0x70, struct tofino2_telemetry)
#define TOFINO2_IOC_GET_PORT_STATS _IOR(TOFINO2_IOC_MAGIC, 0x71, struct tofino2_port)
#define TOFINO2_IOC_GET_FW_VERSION _IOR(TOFINO2_IOC_MAGIC, 0x80, uint32_t)
#define TOFINO2_IOC_LOAD_FW       _IOW(TOFINO2_IOC_MAGIC, 0x81, struct firmware)
#define TOFINO2_IOC_RESET         _IO(TOFINO2_IOC_MAGIC, 0x90)
#define TOFINO2_IOC_SOFT_RESET    _IO(TOFINO2_IOC_MAGIC, 0x91)
#define TOFINO2_IOC_GET_DIAG      _IOR(TOFINO2_IOC_MAGIC, 0xA0, struct tofino2_telemetry)
#define TOFINO2_IOC_SET_POWER_LIMIT _IOW(TOFINO2_IOC_MAGIC, 0xB0, uint32_t)
#define TOFINO2_IOC_GET_DMA       _IOR(TOFINO2_IOC_MAGIC, 0xC0, struct tofino2_dma_channel)
#define TOFINO2_IOC_SET_DMA       _IOW(TOFINO2_IOC_MAGIC, 0xC1, struct tofino2_dma_channel)

/* ============================================================================ */
/* Inline helpers                                                               */
/* ============================================================================ */

static inline int tofino2_state_valid(enum tofino2_state state)
{
    return (state >= TOFINO2_STATE_INIT && state <= TOFINO2_STATE_FAULT);
}

static inline int tofino2_port_speed_valid(enum tofino2_port_speed speed)
{
    return (speed >= TOFINO2_SPEED_10M && speed <= TOFINO2_SPEED_400G);
}

static inline const char *tofino2_state_str(enum tofino2_state state)
{
    switch (state) {
    case TOFINO2_STATE_INIT:      return "INIT";
    case TOFINO2_STATE_RESET:     return "RESET";
    case TOFINO2_STATE_READY:     return "READY";
    case TOFINO2_STATE_PROBING:   return "PROBING";
    case TOFINO2_STATE_RUNNING:   return "RUNNING";
    case TOFINO2_STATE_SUSPEND:   return "SUSPEND";
    case TOFINO2_STATE_RESUME:    return "RESUME";
    case TOFINO2_STATE_ERROR:     return "ERROR";
    case TOFINO2_STATE_STOPPED:   return "STOPPED";
    case TOFINO2_STATE_FAULT:     return "FAULT";
    default:                      return "UNKNOWN";
    }
}

/* ============================================================================ */
/* Function prototypes                                                          */
/* ============================================================================ */

/* tofino2_core.c */
int tofino2_init(void);
void tofino2_exit(void);
int tofino2_probe(struct pci_dev *pci_dev, const struct pci_device_id *id);
int tofino2_remove(struct pci_dev *pci_dev);
struct tofino2_device *tofino2_get_device(int idx);
int tofino2_get_device_count(void);
void tofino2_reset_device(struct tofino2_device *dev);
int tofino2_validate_device(struct tofino2_device *dev);

/* tofino2_p4.c */
int tofino2_pipeline_compile(struct tofino2_device *dev,
                              struct tofino2_pipeline *pipe);
int tofino2_pipeline_load(struct tofino2_device *dev,
                         struct tofino2_pipeline *pipe);
int tofino2_pipeline_unload(struct tofino2_device *dev, uint32_t pipe_id);
int tofino2_table_add(struct tofino2_device *dev, uint32_t pipe_id,
                      struct tofino2_table *table);
int tofino2_table_delete(struct tofino2_device *dev, uint32_t table_id);
int tofino2_table_entry_add(struct tofino2_device *dev,
                            struct tofino2_table_entry *entry);
int tofino2_table_entry_delete(struct tofino2_device *dev,
                               uint32_t table_id, uint32_t entry_id);
int tofino2_table_entry_lookup(struct tofino2_device *dev,
                               struct tofino2_table_entry *entry,
                               struct tofino2_table_entry *result);
int tofino2_table_counters_get(struct tofino2_device *dev,
                                uint32_t table_id,
                                uint32_t *hits, uint32_t *misses);

/* tofino2_phy.c */
int tofino2_port_init(struct tofino2_device *dev, uint32_t port_id);
int tofino2_port_enable(struct tofino2_device *dev, uint32_t port_id);
int tofino2_port_disable(struct tofino2_device *dev, uint32_t port_id);
int tofino2_port_set_speed(struct tofino2_device *dev, uint32_t port_id,
                           enum tofino2_port_speed speed);
int tofino2_port_set_fec(struct tofino2_device *dev, uint32_t port_id,
                         enum tofino2_fec_mode fec);
int tofino2_port_get_link_status(struct tofino2_device *dev,
                                  uint32_t port_id,
                                  enum tofino2_link_status *status);
int tofino2_port_get_diag(struct tofino2_device *dev,
                           uint32_t port_id,
                           struct tofino2_port *diag);
int tofino2_port_set_loopback(struct tofino2_device *dev,
                               uint32_t port_id, int enable);
int tofino2_port_an_restart(struct tofino2_device *dev, uint32_t port_id);

/* tofino2_ctrl.c */
int tofino2_counter_allocate(struct tofino2_device *dev,
                              enum tofino2_counter_type type,
                              uint32_t *counter_id);
int tofino2_counter_free(struct tofino2_device *dev, uint32_t counter_id);
int tofino2_counter_read(struct tofino2_device *dev, uint32_t counter_id,
                          uint64_t *value);
int tofino2_counter_reset(struct tofino2_device *dev, uint32_t counter_id);
int tofino2_meter_allocate(struct tofino2_device *dev,
                            enum tofino2_meter_color color_mode,
                            uint32_t cir, uint32_t pir,
                            uint32_cbs, uint32_t pbs,
                            uint32_t *meter_id);
int tofino2_meter_free(struct tofino2_device *dev, uint32_t meter_id);
int tofino2_meter_configure(struct tofino2_device *dev,
                             struct tofino2_meter *meter);
int tofino2_meter_check(struct tofino2_device *dev, uint32_t meter_id,
                         uint32_t packet_len,
                         enum tofino2_meter_color *color);
int tofino2_dma_channel_allocate(struct tofino2_device *dev,
                                  uint32_t *channel_id);
int tofino2_dma_channel_free(struct tofino2_device *dev, uint32_t channel_id);
int tofino2_dma_start(struct tofino2_device *dev,
                      struct tofino2_dma_channel *chan);
int tofino2_dma_poll(struct tofino2_device *dev, uint32_t channel_id,
                      int timeout_ms);

/* tofino2_telemetry.c */
int tofino2_telemetry_init(struct tofino2_device *dev);
void tofino2_telemetry_exit(struct tofino2_device *dev);
int tofino2_telemetry_read(struct tofino2_device *dev,
                            struct tofino2_telemetry *telem);
void tofino2_telemetry_update(struct tofino2_device *dev);
int tofino2_telemetry_record(struct tofino2_device *dev);
const struct tofino2_telemetry *tofino2_telemetry_snapshot(
    struct tofino2_device *dev, uint32_t idx);
int tofino2_port_stats_read(struct tofino2_device *dev,
                             struct tofino2_port *stats);

/* IRQ handling */
int tofino2_irq_init(struct tofino2_device *dev);
void tofino2_irq_exit(struct tofino2_device *dev);
irqreturn_t tofino2_irq_handler(int irq, void *dev_id);
void tofino2_irq_enable(struct tofino2_device *dev);
void tofino2_irq_disable(struct tofino2_device *dev);

/* Power management */
int tofino2_power_init(struct tofino2_device *dev);
int tofino2_power_suspend(struct tofino2_device *dev);
int tofino2_power_resume(struct tofino2_device *dev);
int tofino2_power_set_limit(struct tofino2_device *dev, uint32_t limit_mw);

/* Sysfs attributes */
extern struct device_attribute tofino2_device_attrs[];
extern struct device_attribute tofino2_port_attrs[];

#endif /* TOFINO2_H */
