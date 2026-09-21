# MTS-MB-3000 — Драйверы NXP S32G3

## 1. Структура драйвера

```
s32g3-driver/
├── include/
│   ├── s32g3.h
│   ├── s32g3_net.h
│   ├── s32g3_sec.h
│   ├── s32g3_ptp.h
│   └── s32g3_sync.h
├── src/
│   ├── s32g3_core.c
│   ├── s32g3_net.c
│   ├── s32g3_sec.c
│   ├── s32g3_ptp.c
│   └── s32g3_sync.c
├── Makefile
└── README.md
```

## 2. s32g3.h — Основные структуры

```c
#ifndef S32G3_H
#define S32G3_H

#include <linux/types.h>

#define S32G3_MAX_DEVICES 1
#define S32G3_MAX_CORES 8
#define S32G3_MAX_PORTS 32
#define S32G3_MAX_CHANNELS 64

/* Device states */
enum s32g3_state {
    S32G3_STATE_INIT,
    S32G3_STATE_READY,
    S32G3_STATE_RUNNING,
    S32G3_STATE_ERROR,
    S32G3_STATE_STOPPED
};

/* Core configuration */
struct s32g3_core {
    uint32_t id;
    uint32_t type; /* A53 or A72 */
    uint32_t enabled;
    uint32_t frequency;
    uint32_t temperature;
    uint64_t instructions;
    uint64_t cycles;
};

/* Port configuration */
struct s32g3_port {
    uint32_t id;
    char name[32];
    uint32_t speed;
    uint32_t duplex;
    uint32_t status;
    uint32_t autoneg;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_drops;
    uint64_t tx_drops;
};

/* PTP configuration */
struct s32g3_ptp {
    uint32_t id;
    uint32_t mode; /* grandmaster, boundary, ordinary */
    uint32_t domain;
    uint32_t port_id;
    uint64_t current_time;
    uint64_t last_sync;
    uint64_t last_follow_up;
    uint64_t last_pdelay_req;
    uint64_t last_pdelay_resp;
    uint64_t last_sync_resp;
    uint64_t last_delay_req;
    uint64_t last_delay_resp;
    uint64_t last_delay_resp_ack;
    uint64_t last_announce;
    uint64_t last_sync_interval;
    uint64_t last_follow_up_interval;
    uint64_t last_pdelay_req_interval;
    uint64_t last_pdelay_resp_interval;
    uint64_t last_sync_resp_interval;
    uint64_t last_delay_req_interval;
    uint64_t last_delay_resp_interval;
    uint64_t last_delay_resp_ack_interval;
    uint64_t last_announce_interval;
    uint64_t grandmaster_priority1;
    uint64_t grandmaster_priority2;
    uint64_t grandmaster_clock_quality;
    uint64_t grandmaster_clock_class;
    uint64_t grandmaster_clock_accuracy;
    uint64_t grandmaster_clock_variance;
    uint64_t grandmaster_time_source;
    uint64_t grandmaster_time_source_id;
    uint64_t grandmaster_time_source_name;
    uint64_t grandmaster_time_source_description;
    uint64_t grandmaster_time_source_protocol;
    uint64_t grandmaster_time_source_type;
    uint64_t grandmaster_time_source_UTC_offset;
    uint64_t grandmaster_time_source_UTC_offset_valid;
    uint64_t grandmaster_time_source_UTC_offset_source;
    uint64_t grandmaster_time_source_UTC_offset_source_valid;
    uint64_t grandmaster_time_source_UTC_offset_source_protocol;
    uint64_t grandmaster_time_source_UTC_offset_source_type;
    uint64_t grandmaster_time_source_UTC_offset_source_name;
    uint64_t grandmaster_time_source_UTC_offset_source_description;
    uint64_t grandmaster_time_source_UTC_offset_source_protocol;
    uint64_t grandmaster_time_source_UTC_offset_source_type;
};

/* SyncE configuration */
struct s32g3_sync {
    uint32_t id;
    uint32_t mode; /* master, slave, transparent */
    uint32_t port_id;
    uint32_t frequency;
    uint32_t accuracy;
    uint32_t holdover;
    uint32_t holdover_accuracy;
    uint32_t holdover_duration;
    uint32_t holdover_type;
    uint32_t holdover_source;
    uint32_t holdover_source_id;
    uint32_t holdover_source_name;
    uint32_t holdover_source_description;
    uint32_t holdover_source_protocol;
    uint32_t holdover_source_type;
    uint32_t holdover_source_UTC_offset;
    uint32_t holdover_source_UTC_offset_valid;
    uint32_t holdover_source_UTC_offset_source;
    uint32_t holdover_source_UTC_offset_source_valid;
    uint32_t holdover_source_UTC_offset_source_protocol;
    uint32_t holdover_source_UTC_offset_source_type;
};

/* Device */
struct s32g3_device {
    uint32_t id;
    char name[32];
    enum s32g3_state state;
    uint32_t num_cores;
    uint32_t num_ports;
    struct s32g3_core *cores;
    struct s32g3_port *ports;
    struct s32g3_ptp *ptp;
    struct s32g3_sync *sync;
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
#define S32G3_IOC_MAGIC 'S'
#define S32G3_IOC_GET_DEV _IOR(S32G3_IOC_MAGIC, 1, struct s32g3_device)
#define S32G3_IOC_SET_DEV _IOW(S32G3_IOC_MAGIC, 2, struct s32g3_device)
#define S32G3_IOC_GET_CORE _IOR(S32G3_IOC_MAGIC, 3, struct s32g3_core)
#define S32G3_IOC_SET_CORE _IOW(S32G3_IOC_MAGIC, 4, struct s32g3_core)
#define S32G3_IOC_GET_PORT _IOR(S32G3_IOC_MAGIC, 5, struct s32g3_port)
#define S32G3_IOC_SET_PORT _IOW(S32G3_IOC_MAGIC, 6, struct s32g3_port)
#define S32G3_IOC_GET_PTP _IOR(S32G3_IOC_MAGIC, 7, struct s32g3_ptp)
#define S32G3_IOC_SET_PTP _IOW(S32G3_IOC_MAGIC, 8, struct s32g3_ptp)
#define S32G3_IOC_GET_SYNC _IOR(S32G3_IOC_MAGIC, 9, struct s32g3_sync)
#define S32G3_IOC_SET_SYNC _IOW(S32G3_IOC_MAGIC, 10, struct s32g3_sync)
#define S32G3_IOC_GET_SEC _IOR(S32G3_IOC_MAGIC, 11, struct s32g3_sec)
#define S32G3_IOC_SET_SEC _IOW(S32G3_IOC_MAGIC, 12, struct s32g3_sec)

/* Function prototypes */
int s32g3_init(void);
void s32g3_exit(void);
int s32g3_probe(struct device *dev);
int s32g3_remove(struct device *dev);
int s32g3_open(struct net_device *netdev);
int s32g3_stop(struct net_device *netdev);
int s32g3_xmit(struct sk_buff *skb, struct net_device *netdev);
int s32g3_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif /* S32G3_H */