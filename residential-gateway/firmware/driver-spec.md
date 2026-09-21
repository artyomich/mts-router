# MTS-RG-500 — Драйверы MediaTek MT7981 + Realtek RTL960x

## 1. Структура драйверов

```
mts-rg-drivers/
├── mt7981/
│   ├── include/
│   │   ├── mt7981.h
│   │   ├── mt7981_net.h
│   │   └── mt7981_wifi.h
│   └── src/
│       ├── mt7981_core.c
│       ├── mt7981_net.c
│       └── mt7981_wifi.c
├── rtl960x/
│   ├── include/
│   │   ├── rtl960x.h
│   │   ├── rtl960x_gpon.h
│   │   └── rtl960x_omci.h
│   └── src/
│       ├── rtl960x_core.c
│       ├── rtl960x_gpon.c
│       └── rtl960x_omci.c
├── Makefile
└── README.md
```

## 2. mt7981_net.h — Драйвер Ethernet

```c
#ifndef MT7981_NET_H
#define MT7981_NET_H

#include <linux/types.h>

#define MT7981_NET_MAX_PORTS 4
#define MT7981_NET_MAX_CHANNELS 16

struct mt7981_net_port {
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

struct mt7981_net_device {
    uint32_t id;
    char name[32];
    uint32_t num_ports;
    struct mt7981_net_port *ports;
    void *priv;
    struct device *dev;
    struct mutex lock;
    struct net_device *netdev;
};

int mt7981_net_init(void);
void mt7981_net_exit(void);
int mt7981_net_probe(struct device *dev);
int mt7981_net_remove(struct device *dev);
int mt7981_net_open(struct net_device *netdev);
int mt7981_net_stop(struct net_device *netdev);
int mt7981_net_xmit(struct sk_buff *skb, struct net_device *netdev);

#endif /* MT7981_NET_H */
```

## 3. mt7981_wifi.h — Драйвер WiFi 6

```c
#ifndef MT7981_WIFI_H
#define MT7981_WIFI_H

#include <linux/types.h>

#define MT7981_WIFI_MAX_BSS 4
#define MT7981_WIFI_MAX_CLIENTS 64

struct mt7981_wifi_bss {
    uint32_t id;
    char ssid[33];
    uint32_t band; /* 2.4GHz or 5GHz */
    uint32_t channel;
    uint32_t bandwidth; /* 20, 40, 80 MHz */
    uint32_t security; /* none, wep, wpa, wpa2, wpa3 */
    uint32_t mode; /* ap, sta, monitor */
    uint32_t status;
    uint32_t num_clients;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
};

struct mt7981_wifi_client {
    uint32_t id;
    char mac[18];
    char ssid[33];
    uint32_t band;
    uint32_t channel;
    uint32_t signal;
    uint32_t rx_rate;
    uint32_t tx_rate;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint32_t connected;
    uint64_t last_seen;
};

struct mt7981_wifi_device {
    uint32_t id;
    char name[32];
    uint32_t num_bss;
    struct mt7981_wifi_bss *bss;
    uint32_t num_clients;
    struct mt7981_wifi_client *clients;
    void *priv;
    struct device *dev;
    struct mutex lock;
};

int mt7981_wifi_init(void);
void mt7981_wifi_exit(void);
int mt7981_wifi_probe(struct device *dev);
int mt7981_wifi_remove(struct device *dev);
int mt7981_wifi_add_bss(struct mt7981_wifi_bss *bss);
int mt7981_wifi_del_bss(uint32_t bss_id);
int mt7981_wifi_update_bss(struct mt7981_wifi_bss *bss);
int mt7981_wifi_add_client(struct mt7981_wifi_client *client);
int mt7981_wifi_del_client(uint32_t client_id);
int mt7981_wifi_get_clients(struct mt7981_wifi_client *clients,
                            uint32_t max_clients);

#endif /* MT7981_WIFI_H */