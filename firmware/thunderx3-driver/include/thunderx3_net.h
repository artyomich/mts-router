/* SPDX-License-Identifier: GPL-2.0 */
/*
 * thunderx3_net.h - ThunderX3 Network Subsystem Interface
 *
 * MTS-MC-5000 Mobile Core ThunderX3 Driver
 */

#ifndef THUNDERX3_NET_H
#define THUNDERX3_NET_H

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>

#define THUNDERX3_NET_MAX_PORTS 64
#define THUNDERX3_NET_MAX_QUEUES 128
#define THUNDERX3_NET_MAX_MACS 256
#define THUNDERX3_NET_VLAN_TABLE_SIZE 4096
#define THUNDERX3_NET_MTU_MIN 68
#define THUNDERX3_NET_MTU_MAX 9600
#define THUNDERX3_NET_MTU_DEFAULT 1500
#define THUNDERX3_NET_JUMBO_MTU 9600
#define THUNDERX3_NET_PORT_NAME_LEN 32

/* ============================================================================ */
/* Enums                                                                        */
/* ============================================================================ */

enum thunderx3_net_port_mode {
    THUNDERX3_NET_MODE_RGMII,
    THUNDERX3_NET_MODE_SGMII,
    THUNDERX3_NET_MODE_XGMII,
    THUNDERX3_NET_MODE_10GBASE_R,
    THUNDERX3_NET_MODE_25GBASE_R,
    THUNDERX3_NET_MODE_100GBASE_R,
    THUNDERX3_NET_MODE_200GBASE_R,
    THUNDERX3_NET_MODE_400GBASE_R,
    THUNDERX3_NET_MODE_UNKNOWN
};

enum thunderx3_net_link_speed {
    THUNDERX3_NET_SPEED_1G   = 1000,
    THUNDERX3_NET_SPEED_2_5G = 2500,
    THUNDERX3_NET_SPEED_5G   = 5000,
    THUNDERX3_NET_SPEED_10G  = 10000,
    THUNDERX3_NET_SPEED_25G  = 25000,
    THUNDERX3_NET_SPEED_40G  = 40000,
    THUNDERX3_NET_SPEED_50G  = 50000,
    THUNDERX3_NET_SPEED_100G = 100000,
    THUNDERX3_NET_SPEED_200G = 200000,
    THUNDERX3_NET_SPEED_400G = 400000,
    THUNDERX3_NET_SPEED_AUTO = 0
};

enum thunderx3_net_fec_mode {
    THUNDERX3_NET_FEC_DISABLED,
    THUNDERX3_NET_FEC_RS,
    THUNDERX3_NET_FEC_BCH,
    THUNDERX3_NET_FEC_AUTO
};

/* ============================================================================ */
/* Structures                                                                   */
/* ============================================================================ */

struct thunderx3_net_queue {
    uint32_t id;
    uint32_t type;  /* TX or RX */
    uint32_t depth;
    uint32_t filled;
    uint64_t packets;
    uint64_t bytes;
    uint64_t drops;
    uint64_t errors;
    uint32_t irq;
    uint8_t enabled;
    uint8_t pad[3];
};

struct thunderx3_net_mac {
    uint32_t id;
    uint8_t address[6];
    uint32_t vlan;
    uint32_t active;
    char name[32];
};

struct thunderx3_net_port {
    uint32_t id;
    char name[THUNDERX3_NET_PORT_NAME_LEN];
    uint32_t speed;
    uint32_t duplex;
    uint32_t status;
    uint32_t autoneg;
    uint32_t fec;
    enum thunderx3_net_port_mode mode;
    uint32_t mtu;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_drops;
    uint64_t tx_drops;
    uint64_t rx_crc_errors;
    uint64_t tx_collisions;
    uint32_t num_queues;
    struct thunderx3_net_queue *queues;
    uint32_t num_macs;
    struct thunderx3_net_mac *macs;
    uint32_t phy_id;
    uint32_t interface;
};

struct thunderx3_net_device {
    uint32_t id;
    char name[32];
    uint32_t num_ports;
    struct thunderx3_net_port *ports;
    uint32_t num_vlans;
    uint32_t vlan_table[THUNDERX3_NET_VLAN_TABLE_SIZE];
    void *priv;
    struct device *dev;
    struct mutex lock;
    struct net_device *netdev;
};

/* ============================================================================ */
/* API Functions                                                                  */
/* ============================================================================ */

int thunderx3_net_port_init(uint32_t port_id);
int thunderx3_net_port_enable(uint32_t port_id);
int thunderx3_net_port_disable(uint32_t port_id);
int thunderx3_net_port_set_mtu(uint32_t port_id, uint32_t mtu);
int thunderx3_net_port_get_mtu(uint32_t port_id, uint32_t *mtu);
int thunderx3_net_port_set_speed(uint32_t port_id, uint32_t speed);
int thunderx3_net_port_get_speed(uint32_t port_id, uint32_t *speed);
int thunderx3_net_port_set_fec(uint32_t port_id, uint32_t fec);
int thunderx3_net_port_get_fec(uint32_t port_id, uint32_t *fec);
int thunderx3_net_port_get_stats(uint32_t port_id,
                                  struct thunderx3_net_port *stats);
int thunderx3_net_port_add_mac(uint32_t port_id, const uint8_t *mac);
int thunderx3_net_port_del_mac(uint32_t port_id, const uint32_t mac_id);
int thunderx3_net_port_get_mac(uint32_t port_id, uint32_t mac_id,
                                uint8_t *mac);
int thunderx3_net_port_set_vlan(uint32_t port_id, uint32_t vlan_id);
int thunderx3_net_port_get_vlan(uint32_t port_id, uint32_t *vlan_id);
int thunderx3_net_port_link_up(uint32_t port_id);
int thunderx3_net_port_link_down(uint32_t port_id);
int thunderx3_net_port_get_link_status(uint32_t port_id, uint32_t *status);
int thunderx3_net_port_an_restart(uint32_t port_id);
int thunderx3_net_port_loopback_set(uint32_t port_id, int enable);

#endif /* THUNDERX3_NET_H */
