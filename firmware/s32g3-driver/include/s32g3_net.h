/* SPDX-License-Identifier: GPL-2.0 */
/*
 * s32g3_net.h - NXP S32G3 Ethernet Network Driver Interface
 *
 * MTS-MB-3000 Mobile Backhaul S32G3 Driver
 */

#ifndef S32G3_NET_H
#define S32G3_NET_H

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>

#define S32G3_NET_MAX_PORTS 32
#define S32G3_NET_MAX_QUEUES 64
#define S32G3_NET_MAX_MACS 128
#define S32G3_NET_VLAN_TABLE_SIZE 4096
#define S32G3_NET_MTU_MIN 68
#define S32G3_NET_MTU_MAX 1518
#define S32G3_NET_MTU_DEFAULT 1500
#define S32G3_NET_PORT_NAME_LEN 32

enum s32g3_net_port_mode {
    S32G3_NET_MODE_RGMII,
    S32G3_NET_MODE_SGMII,
    S32G3_NET_MODE_XGMII,
    S32G3_NET_MODE_10GBASE_R,
    S32G3_NET_MODE_25GBASE_R,
    S32G3_NET_MODE_100GBASE_R,
    S32G3_NET_MODE_UNKNOWN
};

struct s32g3_net_queue {
    uint32_t id;
    uint32_t type;
    uint32_t depth;
    uint64_t packets;
    uint64_t bytes;
    uint64_t drops;
    uint64_t errors;
    uint32_t irq;
    uint8_t enabled;
    uint8_t pad[3];
};

struct s32g3_net_mac {
    uint32_t id;
    uint8_t address[6];
    uint32_t vlan;
    uint32_t active;
    char name[32];
};

struct s32g3_net_port {
    uint32_t id;
    char name[S32G3_NET_PORT_NAME_LEN];
    uint32_t speed;
    uint32_t duplex;
    uint32_t status;
    uint32_t autoneg;
    uint32_t fec;
    enum s32g3_net_port_mode mode;
    uint32_t mtu;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_drops;
    uint64_t tx_drops;
    uint32_t num_queues;
    struct s32g3_net_queue *queues;
    uint32_t num_macs;
    struct s32g3_net_mac *macs;
    uint32_t phy_id;
    uint32_t interface;
};

struct s32g3_net_device {
    uint32_t id;
    char name[32];
    uint32_t num_ports;
    struct s32g3_net_port *ports;
    uint32_t num_vlans;
    uint32_t vlan_table[S32G3_NET_VLAN_TABLE_SIZE];
    void *priv;
    struct device *dev;
    struct mutex lock;
    struct net_device *netdev;
};

/* API */
int s32g3_net_port_init(uint32_t port_id);
int s32g3_net_port_enable(uint32_t port_id);
int s32g3_net_port_disable(uint32_t port_id);
int s32g3_net_port_set_mtu(uint32_t port_id, uint32_t mtu);
int s32g3_net_port_get_mtu(uint32_t port_id, uint32_t *mtu);
int s32g3_net_port_set_speed(uint32_t port_id, uint32_t speed);
int s32g3_net_port_get_speed(uint32_t port_id, uint32_t *speed);
int s32g3_net_port_get_stats(uint32_t port_id,
                              struct s32g3_net_port *stats);
int s32g3_net_port_add_mac(uint32_t port_id, const uint8_t *mac);
int s32g3_net_port_get_mac(uint32_t port_id, uint32_t mac_id,
                            uint8_t *mac);
int s32g3_net_port_link_up(uint32_t port_id);
int s32g3_net_port_link_down(uint32_t port_id);
int s32g3_net_port_get_link_status(uint32_t port_id, uint32_t *status);
int s32g3_net_init(void);
void s32g3_net_exit(void);

#endif /* S32G3_NET_H */
