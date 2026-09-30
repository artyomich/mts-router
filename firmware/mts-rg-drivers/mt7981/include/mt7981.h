/*
 * mt7981.h — Основные структуры и определения для MediaTek MT7981
 *
 * MTS-RG-500 Residential Gateway — Драйвер SoC MT7981
 * Используется в MTS-RG-500 (домашний шлюз)
 */

#ifndef MT7981_H
#define MT7981_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/netdevice.h>

#define MT7981_MODULE_NAME "mt7981"
#define MT7981_MAX_PORTS 4
#define MT7981_MAX_CHANNELS 16
#define MT7981_MAX_BSS 4
#define MT7981_MAX_CLIENTS 64
#define MT7981_MAX_WIFI_DEVS 2

/* Device states */
enum mt7981_state {
    MT7981_STATE_INIT,
    MT7981_STATE_READY,
    MT7981_STATE_RUNNING,
    MT7981_STATE_ERROR,
    MT7981_STATE_STOPPED
};

/* WiFi band */
enum mt7981_band {
    MT7981_BAND_2GHZ = 0,
    MT7981_BAND_5GHZ = 1
};

/* WiFi security */
enum mt7981_security {
    MT7981_SEC_NONE = 0,
    MT7981_SEC_WEP,
    MT7981_SEC_WPA,
    MT7981_SEC_WPA2,
    MT7981_SEC_WPA3
};

/* WiFi mode */
enum mt7981_mode {
    MT7981_MODE_AP = 0,
    MT7981_MODE_STA,
    MT7981_MODE_MONITOR
};

/* WiFi BSS */
struct mt7981_wifi_bss {
    uint32_t id;
    char ssid[33];
    uint32_t band;    /* MT7981_BAND_* */
    uint32_t channel;
    uint32_t bandwidth;   /* 20, 40, 80 MHz */
    uint32_t security;    /* MT7981_SEC_* */
    uint32_t mode;        /* MT7981_MODE_* */
    uint32_t status;
    uint32_t num_clients;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
};

/* WiFi client */
struct mt7981_wifi_client {
    uint32_t id;
    char mac[18];
    char ssid[33];
    uint32_t band;
    uint32_t channel;
    int32_t signal;     /* dBm */
    uint32_t rx_rate;   /* kbps */
    uint32_t tx_rate;   /* kbps */
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint32_t connected;
    uint64_t last_seen;
};

/* Ethernet port */
struct mt7981_eth_port {
    uint32_t id;
    char name[32];
    uint32_t speed;     /* Mbps */
    uint32_t duplex;    /* half/full */
    uint32_t status;    /* UP/DOWN */
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

/* SoC device */
struct mt7981_device {
    uint32_t id;
    char name[32];
    enum mt7981_state state;
    uint32_t num_eth_ports;
    uint32_t num_bss;
    uint32_t num_clients;
    struct mt7981_eth_port *eth_ports;
    struct mt7981_wifi_bss *bss;
    struct mt7981_wifi_client *clients;
    void *priv;
    struct device *dev;
    struct mutex lock;
    struct workqueue_struct *wq;
    struct timer_list timer;
};

/* IOCTL commands */
#define MT7981_IOC_MAGIC 'M'
#define MT7981_IOC_GET_DEV      _IOR(MT7981_IOC_MAGIC, 1, struct mt7981_device)
#define MT7981_IOC_SET_DEV      _IOW(MT7981_IOC_MAGIC, 2, struct mt7981_device)
#define MT7981_IOC_GET_WIFI     _IOR(MT7981_IOC_MAGIC, 3, struct mt7981_wifi_bss)
#define MT7981_IOC_SET_WIFI     _IOW(MT7981_IOC_MAGIC, 4, struct mt7981_wifi_bss)
#define MT7981_IOC_GET_CLIENTS  _IOR(MT7981_IOC_MAGIC, 5, struct mt7981_wifi_client[])
#define MT7981_IOC_ADD_CLIENT   _IOW(MT7981_IOC_MAGIC, 6, struct mt7981_wifi_client)
#define MT7981_IOC_DEL_CLIENT   _IOW(MT7981_IOC_MAGIC, 7, uint32_t)
#define MT7981_IOC_GET_ETH      _IOR(MT7981_IOC_MAGIC, 8, struct mt7981_eth_port)
#define MT7981_IOC_GET_TEMP     _IOR(MT7981_IOC_MAGIC, 9, uint32_t)
#define MT7981_IOC_GET_CPU      _IOR(MT7981_IOC_MAGIC, 10, uint32_t)
#define MT7981_IOC_GET_MEM      _IOR(MT7981_IOC_MAGIC, 11, uint64_t)
#define MT7981_IOC_RESET        _IO(MT7981_IOC_MAGIC, 12)

/* Function prototypes */
int mt7981_init(void);
void mt7981_exit(void);
int mt7981_probe(struct device *dev);
int mt7981_remove(struct device *dev);
struct mt7981_device *mt7981_get_device(uint32_t id);
int mt7981_add_bss(struct mt7981_device *dev, struct mt7981_wifi_bss *bss);
int mt7981_del_bss(struct mt7981_device *dev, uint32_t bss_id);
int mt7981_update_bss(struct mt7981_device *dev, struct mt7981_wifi_bss *bss);
int mt7981_add_client(struct mt7981_device *dev, struct mt7981_wifi_client *client);
int mt7981_del_client(struct mt7981_device *dev, uint32_t client_id);
int mt7981_get_clients(struct mt7981_device *dev, struct mt7981_wifi_client *clients, uint32_t max);
int mt7981_get_temperature(struct mt7981_device *dev);
int mt7981_get_cpu_usage(struct mt7981_device *dev);
int mt7981_get_memory(struct mt7981_device *dev, uint64_t *total, uint64_t *free);

#endif /* MT7981_H */
