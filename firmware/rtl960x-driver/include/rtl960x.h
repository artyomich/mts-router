/*
 * rtl960x.h — Основные структуры и определения для драйвера Realtek RTL960x
 *
 * MTS-OLT-2000 GPON OLT — Драйвер GPON line interface
 * Используется в MTS-OLT-2000 и MTS-RG-500
 */

#ifndef RTL960X_H
#define RTL960X_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/netdevice.h>

#define RTL960X_MODULE_NAME "rtl960x"
#define RTL960X_MAX_PORTS 16
#define RTL960X_MAX_ONU 128
#define RTL960X_MAX_WDM 64
#define RTL960X_MAX_WAVELENGTH 4
#define RTL960X_IOCTL_MAX 16

/* Device states */
enum rtl960x_state {
    RTL960X_STATE_INIT,
    RTL960X_STATE_READY,
    RTL960X_STATE_RUNNING,
    RTL960X_STATE_ERROR,
    RTL960X_STATE_STOPPED
};

/* GPON port configuration */
struct rtl960x_gpon_port {
    uint32_t id;
    char name[32];
    uint32_t status;       /* UP/DOWN/MAINTENANCE */
    uint32_t mode;         /* ONU/OLT */
    uint32_t tx_power;     /* dBm */
    uint32_t rx_power;     /* dBm */
    uint32_t bias_current; /* mA */
    uint32_t temperature;  /* Celsius * 100 */
    uint32_t num_onu;
    uint32_t max_onu;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t tx_errors;
    uint64_t rx_errors;
};

/* ONU configuration */
struct rtl960x_onu {
    uint32_t id;
    char serial[16];
    char mac[18];
    uint32_t pon_port;
    uint32_t status;       /* REGISTERED/DISABLED/DETECTING */
    uint32_t power_level;  /* 0-63 (6dB steps) */
    int32_t  distance;     /* nanoseconds */
    uint32_t vlan;
    uint32_t qos_profile;
    uint32_t bandwidth_up;   /* kbps */
    uint32_t bandwidth_down; /* kbps */
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t last_seen;
    uint64_t created;
};

/* WDM component */
struct rtl960x_wdm {
    uint32_t id;
    uint32_t wavelength;     /* nm */
    uint32_t power;          /* dBm * 100 */
    uint32_t status;
    uint32_t temperature;    /* Celsius * 100 */
    uint32_t bias_current;   /* mA * 100 */
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
    struct rtl960x_gpon_port *ports;
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
#define RTL960X_IOC_GET_DEV     _IOR(RTL960X_IOC_MAGIC, 1, struct rtl960x_device)
#define RTL960X_IOC_SET_DEV     _IOW(RTL960X_IOC_MAGIC, 2, struct rtl960x_device)
#define RTL960X_IOC_GET_ONU     _IOR(RTL960X_IOC_MAGIC, 3, struct rtl960x_onu)
#define RTL960X_IOC_SET_ONU     _IOW(RTL960X_IOC_MAGIC, 4, struct rtl960x_onu)
#define RTL960X_IOC_GET_WDM     _IOR(RTL960X_IOC_MAGIC, 5, struct rtl960x_wdm)
#define RTL960X_IOC_SET_WDM     _IOW(RTL960X_IOC_MAGIC, 6, struct rtl960x_wdm)
#define RTL960X_IOC_GET_GPON    _IOR(RTL960X_IOC_MAGIC, 7, struct rtl960x_gpon_port)
#define RTL960X_IOC_SET_GPON    _IOW(RTL960X_IOC_MAGIC, 8, struct rtl960x_gpon_port)
#define RTL960X_IOC_GET_ONUS    _IOR(RTL960X_IOC_MAGIC, 9, struct rtl960x_onu[])
#define RTL960X_IOC_ADD_ONU     _IOW(RTL960X_IOC_MAGIC, 10, struct rtl960x_onu)
#define RTL960X_IOC_DEL_ONU     _IOW(RTL960X_IOC_MAGIC, 11, uint32_t)
#define RTL960X_IOC_RESET       _IO(RTL960X_IOC_MAGIC, 12)
#define RTL960X_IOC_GET_STATS   _IOR(RTL960X_IOC_MAGIC, 13, struct net_device_stats)
#define RTL960X_IOC_SET_POWER   _IOW(RTL960X_IOC_MAGIC, 14, uint32_t)

/* Function prototypes */
int rtl960x_init(void);
void rtl960x_exit(void);
int rtl960x_probe(struct device *dev);
int rtl960x_remove(struct device *dev);
int rtl960x_open(struct net_device *netdev);
int rtl960x_stop(struct net_device *netdev);
int rtl960x_xmit(struct sk_buff *skb, struct net_device *netdev);
int rtl960x_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/* Helper functions */
struct rtl960x_device *rtl960x_get_device(uint32_t id);
int rtl960x_add_onu(struct rtl960x_device *dev, struct rtl960x_onu *onu);
int rtl960x_remove_onu(struct rtl960x_device *dev, uint32_t onu_id);
int rtl960x_update_onu_status(struct rtl960x_device *dev, uint32_t onu_id, uint32_t status);
int rtl960x_get_onu_count(struct rtl960x_device *dev);
int rtl960x_set_tx_power(struct rtl960x_device *dev, uint32_t port_id, uint32_t power);
int rtl960x_get_rx_power(struct rtl960x_device *dev, uint32_t port_id);
int rtl960x_get_temperature(struct rtl960x_device *dev);
int rtl960x_get_bias_current(struct rtl960x_device *dev);

#endif /* RTL960X_H */
