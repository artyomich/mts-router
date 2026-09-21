# MTS-OLT-2000 — Драйверы Realtek RTL960x

## 1. Структура драйвера

```
rtl960x-driver/
├── include/
│   ├── rtl960x.h
│   ├── rtl960x_gpon.h
│   ├── rtl960x_omci.h
│   └── rtl960x_wdm.h
├── src/
│   ├── rtl960x_core.c
│   ├── rtl960x_gpon.c
│   ├── rtl960x_omci.c
│   └── rtl960x_wdm.c
├── Makefile
└── README.md
```

## 2. rtl960x.h — Основные структуры

```c
#ifndef RTL960X_H
#define RTL960X_H

#include <linux/types.h>

#define RTL960X_MAX_PORTS 16
#define RTL960X_MAX_ONU 128
#define RTL960X_MAX_WDM 64

/* Device states */
enum rtl960x_state {
    RTL960X_STATE_INIT,
    RTL960X_STATE_READY,
    RTL960X_STATE_RUNNING,
    RTL960X_STATE_ERROR,
    RTL960X_STATE_STOPPED
};

/* ONU configuration */
struct rtl960x_onu {
    uint32_t id;
    char serial[16];
    char mac[18];
    uint32_t pon_port;
    uint32_t status;
    uint32_t power_level;
    uint32_t distance;
    uint32_t vlan;
    uint32_t qos_profile;
    uint32_t bandwidth_up;
    uint32_t bandwidth_down;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t last_seen;
    uint64_t created;
};

/* WDM configuration */
struct rtl960x_wdm {
    uint32_t id;
    uint32_t wavelength;
    uint32_t power;
    uint32_t status;
    uint32_t temperature;
    uint32_t bias_current;
    uint32_t tx_fault;
    uint32_t rx_loss;
};

/* Device */
struct rtl960x_device {
    uint32_t id;
    char name[32];
    enum rtl960x_state state;
    uint32_t num_ports;
    uint32_t num_onu;
    uint32_t num_wdm;
    struct rtl960x_onu *onus;
    struct rtl960x_wdm *wdm;
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
#define RTL960X_IOC_MAGIC 'R'
#define RTL960X_IOC_GET_DEV _IOR(RTL960X_IOC_MAGIC, 1, struct rtl960x_device)
#define RTL960X_IOC_SET_DEV _IOW(RTL960X_IOC_MAGIC, 2, struct rtl960x_device)
#define RTL960X_IOC_GET_ONU _IOR(RTL960X_IOC_MAGIC, 3, struct rtl960x_onu)
#define RTL960X_IOC_SET_ONU _IOW(RTL960X_IOC_MAGIC, 4, struct rtl960x_onu)
#define RTL960X_IOC_GET_WDM _IOR(RTL960X_IOC_MAGIC, 5, struct rtl960x_wdm)
#define RTL960X_IOC_SET_WDM _IOW(RTL960X_IOC_MAGIC, 6, struct rtl960x_wdm)
#define RTL960X_IOC_GET_GPON _IOR(RTL960X_IOC_MAGIC, 7, struct rtl960x_gpon)
#define RTL960X_IOC_SET_GPON _IOW(RTL960X_IOC_MAGIC, 8, struct rtl960x_gpon)

/* Function prototypes */
int rtl960x_init(void);
void rtl960x_exit(void);
int rtl960x_probe(struct device *dev);
int rtl960x_remove(struct device *dev);
int rtl960x_open(struct net_device *netdev);
int rtl960x_stop(struct net_device *netdev);
int rtl960x_xmit(struct sk_buff *skb, struct net_device *netdev);
int rtl960x_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif /* RTL960X_H */