# MTS-MC-5000 — Драйверы Marvell ThunderX3

## 1. Структура драйвера

```
thunderx3-driver/
├── include/
│   ├── thunderx3.h
│   ├── thunderx3_net.h
│   ├── thunderx3_pmu.h
│   └── thunderx3_cxl.h
├── src/
│   ├── thunderx3_core.c
│   ├── thunderx3_net.c
│   ├── thunderx3_pmu.c
│   └── thunderx3_cxl.c
├── Makefile
└── README.md
```

## 2. thunderx3.h — Основные структуры

```c
#ifndef THUNDERX3_H
#define THUNDERX3_H

#include <linux/types.h>

#define THUNDERX3_MAX_DEVICES 2
#define THUNDERX3_MAX_CORES 96
#define THUNDERX3_MAX_PORTS 64
#define THUNDERX3_MAX_CHANNELS 128

/* Device states */
enum thunderx3_state {
    THUNDERX3_STATE_INIT,
    THUNDERX3_STATE_READY,
    THUNDERX3_STATE_RUNNING,
    THUNDERX3_STATE_ERROR,
    THUNDERX3_STATE_STOPPED
};

/* Core configuration */
struct thunderx3_core {
    uint32_t id;
    uint32_t cluster;
    uint32_t core;
    uint32_t thread;
    uint32_t enabled;
    uint32_t frequency;
    uint32_t temperature;
    uint64_t instructions;
    uint64_t cycles;
    uint64_t cache_miss;
    uint64_t branch_miss;
};

/* Port configuration */
struct thunderx3_port {
    uint32_t id;
    char name[32];
    uint32_t speed;
    uint32_t duplex;
    uint32_t status;
    uint32_t autoneg;
    uint32_t fec;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_drops;
    uint64_t tx_drops;
};

/* Device */
struct thunderx3_device {
    uint32_t id;
    char name[32];
    enum thunderx3_state state;
    uint32_t num_cores;
    uint32_t num_ports;
    struct thunderx3_core *cores;
    struct thunderx3_port *ports;
    void *priv;
    struct device *dev;
    struct mutex lock;
    struct workqueue_struct *wq;
    struct timer_list timer;
    struct net_device *netdev;
    struct net_device_stats stats;
    struct ethtool_ops ethtool_ops;
};

/* IOCTL commands */
#define THUNDERX3_IOC_MAGIC 'T'
#define THUNDERX3_IOC_GET_DEV _IOR(THUNDERX3_IOC_MAGIC, 1, struct thunderx3_device)
#define THUNDERX3_IOC_SET_DEV _IOW(THUNDERX3_IOC_MAGIC, 2, struct thunderx3_device)
#define THUNDERX3_IOC_GET_CORE _IOR(THUNDERX3_IOC_MAGIC, 3, struct thunderx3_core)
#define THUNDERX3_IOC_SET_CORE _IOW(THUNDERX3_IOC_MAGIC, 4, struct thunderx3_core)
#define THUNDERX3_IOC_GET_PORT _IOR(THUNDERX3_IOC_MAGIC, 5, struct thunderx3_port)
#define THUNDERX3_IOC_SET_PORT _IOW(THUNDERX3_IOC_MAGIC, 6, struct thunderx3_port)
#define THUNDERX3_IOC_GET_STATS _IOR(THUNDERX3_IOC_MAGIC, 7, struct thunderx3_stats)
#define THUNDERX3_IOC_SET_STATS _IOW(THUNDERX3_IOC_MAGIC, 8, struct thunderx3_stats)
#define THUNDERX3_IOC_GET_PMC _IOR(THUNDERX3_IOC_MAGIC, 9, struct thunderx3_pmc)
#define THUNDERX3_IOC_SET_PMC _IOW(THUNDERX3_IOC_MAGIC, 10, struct thunderx3_pmc)
#define THUNDERX3_IOC_GET_CXL _IOR(THUNDERX3_IOC_MAGIC, 11, struct thunderx3_cxl)
#define THUNDERX3_IOC_SET_CXL _IOW(THUNDERX3_IOC_MAGIC, 12, struct thunderx3_cxl)

/* Function prototypes */
int thunderx3_init(void);
void thunderx3_exit(void);
int thunderx3_probe(struct device *dev);
int thunderx3_remove(struct device *dev);
int thunderx3_open(struct net_device *netdev);
int thunderx3_stop(struct net_device *netdev);
int thunderx3_xmit(struct sk_buff *skb, struct net_device *netdev);
int thunderx3_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif /* THUNDERX3_H */