# MTS-ER-1000 — Драйверы NXP S32G + Broadcom TomTom

## 1. Структура драйверов

```
mts-er-drivers/
├── s32g3/
│   ├── include/
│   │   ├── s32g3.h
│   │   ├── s32g3_net.h
│   │   └── s32g3_sec.h
│   └── src/
│       ├── s32g3_core.c
│       ├── s32g3_net.c
│       └── s32g3_sec.c
├── tomtom/
│   ├── include/
│   │   ├── tomtom.h
│   │   ├── tomtom_asic.h
│   │   └── tomtom_phy.h
│   └── src/
│       ├── tomtom_core.c
│       ├── tomtom_asic.c
│       └── tomtom_phy.c
├── Makefile
└── README.md
```

## 2. s32g3_net.h — Драйвер Ethernet

```c
#ifndef S32G3_NET_H
#define S32G3_NET_H

#include <linux/types.h>

#define S32G3_NET_MAX_PORTS 32
#define S32G3_NET_MAX_CHANNELS 64

struct s32g3_net_port {
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

struct s32g3_net_device {
    uint32_t id;
    char name[32];
    uint32_t num_ports;
    struct s32g3_net_port *ports;
    void *priv;
    struct device *dev;
    struct mutex lock;
    struct net_device *netdev;
};

int s32g3_net_init(void);
void s32g3_net_exit(void);
int s32g3_net_probe(struct device *dev);
int s32g3_net_remove(struct device *dev);
int s32g3_net_open(struct net_device *netdev);
int s32g3_net_stop(struct net_device *netdev);
int s32g3_net_xmit(struct sk_buff *skb, struct net_device *netdev);

#endif /* S32G3_NET_H */
```

## 3. tomtom_asic.h — Драйвер ASIC

```c
#ifndef TOMTOM_ASIC_H
#define TOMTOM_ASIC_H

#include <linux/types.h>

#define TOMTOM_MAX_PORTS 64
#define TOMTOM_MAX_TABLES 4096
#define TOMTOM_MAX_ENTRIES 1048576

struct tomtom_asic_table {
    uint32_t id;
    char name[64];
    uint32_t type;
    uint32_t key_size;
    uint32_t action_size;
    uint32_t max_entries;
    uint32_t current_entries;
    uint32_t hit_rate;
    uint32_t miss_rate;
};

struct tomtom_asic_device {
    uint32_t id;
    char name[32];
    uint32_t num_tables;
    struct tomtom_asic_table *tables;
    void *priv;
    struct device *dev;
    struct mutex lock;
};

int tomtom_asic_init(void);
void tomtom_asic_exit(void);
int tomtom_asic_probe(struct device *dev);
int tomtom_asic_remove(struct device *dev);
int tomtom_asic_add_table(struct tomtom_asic_table *table);
int tomtom_asic_del_table(uint32_t table_id);
int tomtom_asic_add_entry(uint32_t table_id, uint8_t *key,
                          uint8_t *action, uint32_t priority);
int tomtom_asic_del_entry(uint32_t table_id, uint8_t *key);
int tomtom_asic_lookup(uint32_t table_id, uint8_t *key,
                       uint8_t *action);

#endif /* TOMTOM_ASIC_H */