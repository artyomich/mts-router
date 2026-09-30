/* SPDX-License-Identifier: GPL-2.0 */
/*
 * thunderx3.h - Marvell ThunderX3 ARM Server Driver Main Header
 *
 * MTS-MC-5000 Mobile Core ThunderX3 Driver
 *
 * Copyright (c) 2024 MTS Router Project
 * Author: Firmware Agent
 *
 * This driver provides kernel-space support for Marvell ThunderX3 ARM server
 * SoC. It implements CPU core management, PCIe enumeration, interrupt
 * handling, and system-level monitoring.
 */

#ifndef THUNDERX3_H
#define THUNDERX3_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/cpufreq.h>
#include <linux/thermal.h>

/* ============================================================================ */
/* Constants                                                                    */
/* ============================================================================ */

#define THUNDERX3_MAGIC 0x54583330  /* "TX30" */
#define THUNDERX3_MAX_DEVICES 2
#define THUNDERX3_MAX_CORES_PER_DEV 96
#define THUNDERX3_MAX_PORTS_PER_DEV 64
#define THUNDERX3_MAX_CHANNELS 128
#define THUNDERX3_MAX_NUMA_NODES 8
#define THUNDERX3_DRIVER_VERSION "1.0.0"
#define THUNDERX3_PCI_VENDOR_ID 0x177D
#define THUNDERX3_PCI_DEVICE_ID_BASE 0xA640
#define THUNDERX3_REG_BAR 0
#define THUNDERX3_REG_SPACE_SIZE 0x100000
#define THUNDERX3_MAX_L3_CACHES 12
#define THUNDERX3_MAX_PCIE_PORTS 8
#define THUNDERX3_MAX_INTERRUPTS 256
#define THUNDERX3_CORE_NAME_LEN 32
#define THUNDERX3_DEV_NAME_LEN 32
#define THUNDERX3_POLL_INTERVAL_HZ 10
#define THUNDERX3_MAX_TEMP_SENSORS 16
#define THUNDERX3_MAX_VOLTAGE_SENSORS 16
#define THUNDERX3_MAX_FAN_SENSORS 8

/* ============================================================================ */
/* Enums                                                                        */
/* ============================================================================ */

/**
 * enum thunderx3_state - Device operational states
 */
enum thunderx3_state {
    THUNDERX3_STATE_INIT,
    THUNDERX3_STATE_RESET,
    THUNDERX3_STATE_READY,
    THUNDERX3_STATE_PROBING,
    THUNDERX3_STATE_RUNNING,
    THUNDERX3_STATE_SUSPEND,
    THUNDERX3_STATE_RESUME,
    THUNDERX3_STATE_ERROR,
    THUNDERX3_STATE_STOPPED,
    THUNDERX3_STATE_FAULT
};

/**
 * enum thunderx3_core_type - CPU core types
 */
enum thunderx3_core_type {
    THUNDERX3_CORE_A710_BIG,
    THUNDERX3_CORE_X128_MID,
    THUNDERX3_CORE_A510_LITTLE,
    THUNDERX3_CORE_UNKNOWN
};

/**
 * enum thunderx3_cache_type - L1/L2/L3 cache types
 */
enum thunderx3_cache_type {
    THUNDERX3_CACHE_INSTRUCTION,
    THUNDERX3_CACHE_DATA,
    THUNDERX3_CACHE_UNIFIED,
    THUNDERX3_CACHE_UNKNOWN
};

/**
 * enum thunderx3_irq_type - Interrupt types
 */
enum thunderx3_irq_type {
    THUNDERX3_IRQ_GPIO,
    THUNDERX3_IRQ_GIC,
    THUNDERX3_IRQ_PCIE_MSIX,
    THUNDERX3_IRQ_PCIE_MSI,
    THUNDERX3_IRQ_PCIE_INTX,
    THUNDERX3_IRQ_TIMER,
    THUNDERX3_IRQ_UART,
    THUNDERX3_IRQ_SATA,
    THUNDERX3_IRQ_USB,
    THUNDERX3_IRQ_ETH,
    THUNDERX3_IRQ_UNKNOWN
};

/**
 * enum thunderx3_pmu_event - PMU event types
 */
enum thunderx3_pmu_event {
    THUNDERX3_PMU_CPU_CYCLES,
    THUNDERX3_PMU_CPU_INSTRUCTIONS,
    THUNDERX3_PMU_CACHE_REFERENCES,
    THUNDERX3_PMU_CACHE_MISSES,
    THUNDERX3_PMU_BRANCH_INSTRUCTIONS,
    THUNDERX3_PMU_BRANCH_MISSES,
    THUNDERX3_PMU_BUS_CYCLES,
    THUNDERX3_PMU_STALLED_CYCLES,
    THUNDERX3_PMU_POWER,
    THUNDERX3_PMU_TEMPERATURE,
    THUNDERX3_PMU_DDR_READS,
    THUNDERX3_PMU_DDR_WRITES,
    THUNDERX3_PMU_DDR_BANDWIDTH,
    THUNDERX3_PMU_PCIE_READS,
    THUNDERX3_PMU_PCIE_WRITES,
    THUNDERX3_PMU_PCIE_BANDWIDTH
};

/**
 * enum thunderx3_cxl_type - CXL (Compute Express Link) types
 */
enum thunderx3_cxl_type {
    THUNDERX3_CXL_NONE,
    THUNDERX3_CXL_TYPE1,
    THUNDERX3_CXL_TYPE2,
    THUNDERX3_CXL_TYPE3,
    THUNDERX3_CXL_UNKNOWN
};

/**
 * enum thunderx3_link_status - PCIe link status
 */
enum thunderx3_link_status {
    THUNDERX3_LINK_DOWN,
    THUNDERX3_LINK_UP,
    THUNDERX3_LINK_INIT,
    THUNDERX3_LINK_NEGOTIATING,
    THUNDERX3_LINK_DISABLED,
    THUNDERX3_LINK_ERROR
};

/* ============================================================================ */
/* Structures                                                                   */
/* ============================================================================ */

/**
 * struct thunderx3_core - CPU core configuration and state
 */
struct thunderx3_core {
    uint32_t id;                            /* Core ID */
    uint32_t cluster;                       /* Cluster ID */
    uint32_t core;                          /* Core within cluster */
    uint32_t thread;                        /* Thread ID */
    uint32_t enabled;                       /* Core enabled */
    uint32_t online;                        /* Core online */
    uint32_t frequency;                     /* Current frequency (kHz) */
    uint32_t max_frequency;                 /* Max frequency (kHz) */
    uint32_t min_frequency;                 /* Min frequency (kHz) */
    uint32_t current_frequency;             /* Current frequency (MHz) */
    uint32_t target_frequency;              /* Target frequency (MHz) */
    uint32_t type;                          /* Core type */
    int32_t temperature;                    /* Core temperature (C * 1000) */
    uint64_t instructions;                  /* Instructions retired */
    uint64_t cycles;                        /* CPU cycles */
    uint64_t cache_miss;                    /* Cache misses */
    uint64_t branch_miss;                   /* Branch mispredictions */
    uint64_t context_switches;              /* Voluntary + involuntary */
    uint64_t irq_count;                     /* Interrupt count */
    uint64_t softirq_count;                 /* Soft IRQ count */
    uint64_t syscall_count;                 /* System call count */
    uint64_t page_faults;                   /* Page faults */
    uint64_t minor_faults;                  /* Minor page faults */
    uint64_t major_faults;                  /* Major page faults */
    uint64_t idle_time;                     /* Idle time (ns) */
    uint64_t busy_time;                     /* Busy time (ns) */
    uint64_t last_update;                   /* Last update timestamp */
    uint32_t power_state;                   /* Power state */
    uint32_t power_limit_mw;                /* Power limit (mW) */
    uint32_t power_current_mw;              /* Current power (mW) */
    uint32_t l1_icache_size;                /* L1 I-cache size (KB) */
    uint32_t l1_dcache_size;                /* L1 D-cache size (KB) */
    uint32_t l2_cache_size;                 /* L2 cache size (KB) */
    uint32_t l3_cache_id;                   /* L3 cache ID */
    uint32_t l3_cache_size;                 /* L3 cache size (KB) */
    uint32_t numa_node;                     /* NUMA node */
    uint32_t pstate;                        /* Performance state */
    uint8_t pad[4];
};

/**
 * struct thunderx3_port - Network port configuration and state
 */
struct thunderx3_port {
    uint32_t id;                            /* Port ID */
    char name[32];                          /* Port name */
    uint32_t speed;                         /* Speed (Mbps) */
    uint32_t duplex;                        /* 1 = full, 0 = half */
    uint32_t status;                        /* Link status */
    uint32_t autoneg;                       /* Auto-negotiation enabled */
    uint32_t fec;                           /* FEC mode */
    uint64_t rx_bytes;                      /* RX bytes */
    uint64_t tx_bytes;                      /* TX bytes */
    uint64_t rx_packets;                    /* RX packets */
    uint64_t tx_packets;                    /* TX packets */
    uint64_t rx_errors;                     /* RX errors */
    uint64_t tx_errors;                     /* TX errors */
    uint64_t rx_drops;                      /* RX drops */
    uint64_t tx_drops;                      /* TX drops */
    uint64_t rx_crc_errors;                 /* RX CRC errors */
    uint64_t rx_frame_errors;               /* RX frame errors */
    uint64_t rx_fifo_errors;                /* RX FIFO errors */
    uint64_t tx_fifo_errors;                /* TX FIFO errors */
    uint64_t tx_carrier_errors;             /* TX carrier errors */
    uint64_t tx_heartbeat_errors;           /* TX heartbeat errors */
    uint64_t tx_aborted_errors;             /* TX aborted errors */
    uint64_t rx_compressed;                 /* RX compressed packets */
    uint64_t tx_compressed;                 /* TX compressed packets */
    uint64_t last_update;                   /* Last update timestamp */
    uint32_t mtu;                           /* MTU */
    uint32_t tx_queue_len;                  /* TX queue length */
    uint32_t rx_queues;                     /* RX queues */
    uint32_t tx_queues;                     /* TX queues */
    uint32_t mac_address[6];                /* MAC address */
    uint32_t phy_id;                        /* PHY ID */
    uint32_t phy_type;                      /* PHY type */
    uint32_t interface;                     /* Interface type */
    uint32_t link_speed;                    /* Link speed (Mbps) */
    uint32_t link_duplex;                   /* Link duplex */
    uint8_t pad[4];
};

/**
 * struct thunderx3_cache - Cache information
 */
struct thunderx3_cache {
    uint32_t id;                            /* Cache ID */
    enum thunderx3_cache_type type;         /* Cache type */
    uint32_t level;                         /* Cache level (L1/L2/L3) */
    uint32_t size;                          /* Cache size (KB) */
    uint32_t line_size;                     /* Cache line size (bytes) */
    uint32_t ways;                          /* Associativity (ways) */
    uint32_t sets;                          /* Number of sets */
    uint32_t read_hits;                     /* Read hits */
    uint32_t read_misses;                   /* Read misses */
    uint32_t write_hits;                    /* Write hits */
    uint32_t write_misses;                  /* Write misses */
    uint32_t invalidations;                 /* Invalidations */
    uint32_t prefetches;                    /* Prefetch requests */
    uint64_t last_update;                   /* Last update timestamp */
    uint8_t shared;                         /* Shared between cores */
    uint8_t core_mask[16];                  /* Cores sharing this cache */
    uint32_t num_cores;                     /* Number of sharing cores */
    uint8_t pad[4];
};

/**
 * struct thunderx3_pmc - Performance Monitoring Counter
 */
struct thunderx3_pmc {
    uint32_t id;                            /* Counter ID */
    enum thunderx3_pmu_event event;         /* Event type */
    uint64_t value;                         /* Counter value */
    uint64_t base_value;                    /* Base/reset value */
    uint64_t max_value;                     /* Max value since last read */
    uint32_t enabled;                       /* Counter enabled */
    uint32_t core_id;                       /* Associated core */
    uint32_t width;                         /* Counter width */
    uint32_t overflow_count;                /* Overflow count */
    uint64_t last_update;                   /* Last update timestamp */
    uint8_t pad[4];
};

/**
 * struct thunderx3_cxl_device - CXL (Compute Express Link) device
 */
struct thunderx3_cxl_device {
    uint32_t id;                            /* CXL device ID */
    enum thunderx3_cxl_type type;           /* CXL type */
    uint32_t status;                        /* Device status */
    uint64_t mem_base;                      /* Memory base address */
    uint64_t mem_size;                      /* Memory size */
    uint32_t mem_type;                      /* Memory type (DRAM/HBM) */
    uint32_t mem_speed;                     /* Memory speed (MT/s) */
    uint32_t mem_width;                     /* Memory width (bits) */
    uint64_t cap_base;                      /* Capability base */
    uint32_t cap_version;                   /* Capability version */
    uint32_t cap_type;                      /* Capability type */
    uint32_t cap_next;                      /* Next capability offset */
    uint32_t cap_len;                       /* Capability length */
    uint32_t cap_flags;                     /* Capability flags */
    uint32_t cap_memcap;                    /* Memory capability */
    uint32_t cap_memcap_width;              /* Memory width */
    uint32_t cap_memcap_speed;              /* Memory speed */
    uint32_t cap_memcap_type;               /* Memory type */
    uint32_t cap_memcap_size;               /* Memory size */
    uint32_t num_regions;                   /* Number of regions */
    uint64_t *region_bases;                 /* Region base addresses */
    uint64_t *region_sizes;                 /* Region sizes */
    uint8_t enabled;                        /* Device enabled */
    uint8_t pad[3];
};

/**
 * struct thunderx3_thermal - Thermal zone information
 */
struct thunderx3_thermal {
    uint32_t id;                            /* Thermal zone ID */
    char name[32];                          /* Thermal zone name */
    int32_t temperature;                    /* Current temperature (C * 1000) */
    int32_t critical_temp;                  /* Critical temperature (C * 1000) */
    int32_t passive_temp;                   /* Passive cooling temp (C * 1000) */
    int32_t active_low[8];                  /* Active cooling thresholds */
    int32_t active_high[8];                 /* Active cooling outputs */
    uint32_t num_active;                    /* Number of active points */
    uint32_t trip_count;                    /* Trip point count */
    uint32_t polling_delay;                 /* Polling delay (ms) */
    uint32_t passive_delay;                 /* Passive cooling delay (ms) */
    uint32_t hot_delay;                     /* Hot trip delay (ms) */
    uint8_t enabled;                        /* Thermal zone enabled */
    uint8_t pad[3];
};

/**
 * struct thunderx3_power - Power domain information
 */
struct thunderx3_power {
    uint32_t id;                            /* Power domain ID */
    char name[32];                          /* Power domain name */
    uint32_t voltage;                       /* Voltage (mV) */
    uint32_t current;                       /* Current (mA) */
    uint32_t power;                         /* Power (mW) */
    uint32_t energy;                        /* Energy (mWh) */
    uint32_t limit;                         /* Power limit (mW) */
    uint32_t max_limit;                     /* Max power limit (mW) */
    uint32_t min_limit;                     /* Min power limit (mW) */
    uint32_t sample_time;                   /* Sample time (ms) */
    uint32_t enabled;                       /* Power domain enabled */
    uint8_t pad[4];
};

/**
 * struct thunderx3_device - Main ThunderX3 device structure
 */
struct thunderx3_device {
    uint32_t magic;                         /* Magic number for validation */
    uint32_t id;                            /* Device ID */
    uint32_t pci_fn;                        /* PCI function number */
    struct pci_dev *pci_dev;                /* PCI device pointer */
    char name[THUNDERX3_DEV_NAME_LEN];      /* Device name */
    char serial[16];                        /* Serial number */
    enum thunderx3_state state;             /* Current state */
    uint32_t revision;                      /* Silicon revision */
    uint32_t pkg_type;                      /* Package type */
    uint32_t num_cores;                     /* Number of cores */
    uint32_t num_ports;                     /* Number of ports */
    uint32_t num_numa_nodes;                /* Number of NUMA nodes */
    uint32_t numa_node;                     /* Primary NUMA node */

    /* Resources */
    struct thunderx3_core *cores;           /* Core array */
    struct thunderx3_port *ports;           /* Port array */
    struct thunderx3_cache *l3_caches;      /* L3 caches */
    struct thunderx3_pmc *pmcs;             /* PMU counters */
    struct thunderx3_cxl_device *cxls;      /* CXL devices */
    struct thunderx3_thermal *thermals;     /* Thermal zones */
    struct thunderx3_power *powers;         /* Power domains */

    /* Memory mapping */
    void __iomem *reg_base;                 /* Register base address */
    resource_size_t reg_phys;               /* Register physical address */
    unsigned long reg_size;                 /* Register region size */

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
    struct delayed_work pm_update_work;     /* PM update work */

    /* Network interface */
    struct net_device *netdev;              /* Netdev for sysfs */
    struct net_device_stats stats;          /* Network stats */
    struct ethtool_ops ethtool_ops;         /* Ethtool operations */

    /* Sysfs and debugfs */
    struct device *dev;                     /* Device structure */
    struct dentry *debugfs_dir;             /* Debugfs directory */

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

    /* PMU data */
    struct thunderx3_pmc pmc_snapshot[32];  /* PMU snapshot */
    uint32_t pmc_count;                     /* Number of PMCs */
    uint64_t pmc_poll_interval;             /* PMU poll interval (ns) */

    /* Telemetry */
    uint64_t uptime_ns;                     /* Uptime (nanoseconds) */
    uint64_t boot_time;                     /* Boot time */
    uint32_t cpu_usage[THUNDERX3_MAX_CORES_PER_DEV]; /* CPU usage % */
    int32_t temperature[THUNDERX3_MAX_TEMP_SENSORS]; /* Temperatures */
    uint32_t num_temps;                     /* Number of temp sensors */
    uint32_t voltage[THUNDERX3_MAX_VOLTAGE_SENSORS]; /* Voltages (mV) */
    uint32_t num_voltages;                  /* Number of voltage sensors */
    uint32_t fan_rpm[THUNDERX3_MAX_FAN_SENSORS]; /* Fan RPMs */
    uint32_t num_fans;                      /* Number of fans */

    /* Private data */
    void *priv;                             /* Driver private data */
};

/* ============================================================================ */
/* IOCTL definitions                                                            */
/* ============================================================================ */

#define THUNDERX3_IOC_MAGIC 'T'

#define THUNDERX3_IOC_GET_DEV       _IOR(THUNDERX3_IOC_MAGIC, 0x00, struct thunderx3_device)
#define THUNDERX3_IOC_SET_DEV       _IOW(THUNDERX3_IOC_MAGIC, 0x01, struct thunderx3_device)
#define THUNDERX3_IOC_GET_CORE      _IOR(THUNDERX3_IOC_MAGIC, 0x10, struct thunderx3_core)
#define THUNDERX3_IOC_SET_CORE      _IOW(THUNDERX3_IOC_MAGIC, 0x11, struct thunderx3_core)
#define THUNDERX3_IOC_GET_CORES     _IOR(THUNDERX3_IOC_MAGIC, 0x12, struct thunderx3_core[])
#define THUNDERX3_IOC_GET_PORT      _IOR(THUNDERX3_IOC_MAGIC, 0x20, struct thunderx3_port)
#define THUNDERX3_IOC_SET_PORT      _IOW(THUNDERX3_IOC_MAGIC, 0x21, struct thunderx3_port)
#define THUNDERX3_IOC_GET_PORTS     _IOR(THUNDERX3_IOC_MAGIC, 0x22, struct thunderx3_port[])
#define THUNDERX3_IOC_GET_STATS     _IOR(THUNDERX3_IOC_MAGIC, 0x30, struct thunderx3_stats)
#define THUNDERX3_IOC_SET_STATS     _IOW(THUNDERX3_IOC_MAGIC, 0x31, struct thunderx3_stats)
#define THUNDERX3_IOC_GET_PMC       _IOR(THUNDERX3_IOC_MAGIC, 0x40, struct thunderx3_pmc)
#define THUNDERX3_IOC_SET_PMC       _IOW(THUNDERX3_IOC_MAGIC, 0x41, struct thunderx3_pmc)
#define THUNDERX3_IOC_GET_PMU       _IOR(THUNDERX3_IOC_MAGIC, 0x42, struct thunderx3_pmu)
#define THUNDERX3_IOC_START_PMU     _IO(THUNDERX3_IOC_MAGIC, 0x43)
#define THUNDERX3_IOC_STOP_PMU      _IO(THUNDERX3_IOC_MAGIC, 0x44)
#define THUNDERX3_IOC_GET_CXL       _IOR(THUNDERX3_IOC_MAGIC, 0x50, struct thunderx3_cxl)
#define THUNDERX3_IOC_SET_CXL       _IOW(THUNDERX3_IOC_MAGIC, 0x51, struct thunderx3_cxl)
#define THUNDERX3_IOC_GET_THERMAL   _IOR(THUNDERX3_IOC_MAGIC, 0x60, struct thunderx3_thermal)
#define THUNDERX3_IOC_GET_POWER     _IOR(THUNDERX3_IOC_MAGIC, 0x70, struct thunderx3_power)
#define THUNDERX3_IOC_SET_POWER_LIMIT _IOW(THUNDERX3_IOC_MAGIC, 0x71, uint32_t)
#define THUNDERX3_IOC_GET_FW_VERSION _IOR(THUNDERX3_IOC_MAGIC, 0x80, uint32_t)
#define THUNDERX3_IOC_LOAD_FW       _IOW(THUNDERX3_IOC_MAGIC, 0x81, struct firmware)
#define THUNDERX3_IOC_RESET         _IO(THUNDERX3_IOC_MAGIC, 0x90)
#define THUNDERX3_IOC_SOFT_RESET    _IO(THUNDERX3_IOC_MAGIC, 0x91)
#define THUNDERX3_IOC_GET_DIAG      _IOR(THUNDERX3_IOC_MAGIC, 0xA0, struct thunderx3_device)

/* ============================================================================ */
/* Function prototypes                                                          */
/* ============================================================================ */

/* thunderx3_core.c */
int thunderx3_init(void);
void thunderx3_exit(void);
int thunderx3_probe(struct pci_dev *pci_dev, const struct pci_device_id *id);
int thunderx3_remove(struct pci_dev *pci_dev);
struct thunderx3_device *thunderx3_get_device(int idx);
int thunderx3_get_device_count(void);
void thunderx3_reset_device(struct thunderx3_device *dev);
int thunderx3_validate_device(struct thunderx3_device *dev);
int thunderx3_core_enable(struct thunderx3_device *dev, uint32_t core_id);
int thunderx3_core_disable(struct thunderx3_device *dev, uint32_t core_id);
int thunderx3_core_set_frequency(struct thunderx3_device *dev,
                                  uint32_t core_id, uint32_t freq);
int thunderx3_core_get_frequency(struct thunderx3_device *dev,
                                  uint32_t core_id, uint32_t *freq);

/* thunderx3_net.c */
int thunderx3_net_init(struct thunderx3_device *dev);
void thunderx3_net_exit(struct thunderx3_device *dev);
int thunderx3_net_port_enable(struct thunderx3_device *dev,
                               uint32_t port_id);
int thunderx3_net_port_disable(struct thunderx3_device *dev,
                                uint32_t port_id);
int thunderx3_net_port_set_speed(struct thunderx3_device *dev,
                                  uint32_t port_id, uint32_t speed);
int thunderx3_net_port_get_stats(struct thunderx3_device *dev,
                                  uint32_t port_id,
                                  struct thunderx3_port *stats);
int thunderx3_net_port_link_up(struct thunderx3_device *dev,
                                uint32_t port_id);
int thunderx3_net_port_link_down(struct thunderx3_device *dev,
                                  uint32_t port_id);
int thunderx3_net_port_get_link_status(struct thunderx3_device *dev,
                                        uint32_t port_id,
                                        uint32_t *status);

/* thunderx3_pmu.c */
int thunderx3_pmu_init(struct thunderx3_device *dev);
void thunderx3_pmu_exit(struct thunderx3_device *dev);
int thunderx3_pmc_enable(struct thunderx3_device *dev,
                          uint32_t core_id,
                          enum thunderx3_pmu_event event);
int thunderx3_pmc_disable(struct thunderx3_device *dev,
                           uint32_t core_id,
                           enum thunderx3_pmu_event event);
int thunderx3_pmc_read(struct thunderx3_device *dev,
                        uint32_t core_id,
                        enum thunderx3_pmu_event event,
                        uint64_t *value);
int thunderx3_pmc_reset(struct thunderx3_device *dev,
                         uint32_t core_id,
                         enum thunderx3_pmu_event event);
int thunderx3_pmu_snapshot(struct thunderx3_device *dev,
                            struct thunderx3_pmc *pmcs,
                            uint32_t *num_pmcs);
int thunderx3_pmu_start(struct thunderx3_device *dev);
int thunderx3_pmu_stop(struct thunderx3_device *dev);

/* thunderx3_cxl.c */
int thunderx3_cxl_init(struct thunderx3_device *dev);
void thunderx3_cxl_exit(struct thunderx3_device *dev);
int thunderx3_cxl_device_enable(struct thunderx3_device *dev,
                                 uint32_t cxl_id);
int thunderx3_cxl_device_disable(struct thunderx3_device *dev,
                                  uint32_t cxl_id);
int thunderx3_cxl_device_get(struct thunderx3_device *dev,
                              uint32_t cxl_id,
                              struct thunderx3_cxl_device *cxl);
int thunderx3_cxl_region_add(struct thunderx3_device *dev,
                              uint32_t cxl_id,
                              uint64_t base, uint64_t size);
int thunderx3_cxl_region_remove(struct thunderx3_device *dev,
                                 uint32_t cxl_id, uint32_t region_id);
int thunderx3_cxl_get_memory_info(struct thunderx3_device *dev,
                                   uint64_t *total,
                                   uint64_t *free);

/* IRQ handling */
int thunderx3_irq_init(struct thunderx3_device *dev);
void thunderx3_irq_exit(struct thunderx3_device *dev);
irqreturn_t thunderx3_irq_handler(int irq, void *dev_id);
void thunderx3_irq_enable(struct thunderx3_device *dev);
void thunderx3_irq_disable(struct thunderx3_device *dev);

/* Power management */
int thunderx3_power_init(struct thunderx3_device *dev);
int thunderx3_power_suspend(struct thunderx3_device *dev);
int thunderx3_power_resume(struct thunderx3_device *dev);
int thunderx3_power_set_limit(struct thunderx3_device *dev,
                               uint32_t limit_mw);

#endif /* THUNDERX3_H */
