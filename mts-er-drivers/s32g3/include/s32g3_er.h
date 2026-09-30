/*
 * s32g3_er.h — S32G3 driver for MTS Enterprise Router
 *
 * MTS-ER-1000 Enterprise Router — Драйвер NXP S32G3
 */

#ifndef S32G3_ER_H
#define S32G3_ER_H

#include <linux/types.h>

#define ER_S32G3_MAX_PORTS 4
#define ER_S32G3_MAX_INTERFACES 64
#define ER_S32G3_MAX_VLANS 4096
#define ER_S32G3_MAX_ROUTES 8192
#define ER_S32G3_MAX_QOS_QUEUES 8

/* Port types for enterprise use */
#define ER_PORT_GBE "gbe"
#define ER_PORT_10GBE "10gbe"
#define ER_PORT_SFP "sfp"
#define ER_PORT_SFP_PLUS "sfp_plus"

/* Port modes */
#define ER_PORT_MODE_ROUTED "routed"
#define ER_PORT_MODE_BRIDGED "bridged"
#define ER_PORT_MODE_MPLS "mpls"

/* Interface types */
#define ER_IFACE_PHYSICAL "physical"
#define ER_IFACE_VIRTUAL "virtual"
#define ER_IFACE_VLAN "vlan"
#define ER_IFACE_BOND "bond"
#define ER_IFACE_TUNNEL "tunnel"

/* ==================== Port ==================== */

typedef struct {
    uint32_t port_id;
    char name[32];
    char type[16];
    uint32_t speed_mbps;
    char status[16];
    char mode[16];
    uint8_t mac[6];
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_drops;
    uint64_t tx_drops;
    double rx_power_dbm;
    double tx_power_dbm;
    double temperature_c;
} er_s32g3_port_t;

/* ==================== Interface ==================== */

typedef struct {
    uint32_t iface_id;
    char name[32];
    char type[16];
    char status[16];
    uint8_t mac[6];
    uint32_t mtu;
    uint32_t vlan_id;
    uint32_t bond_master;
    uint32_t bond_slave;
    uint32_t flags;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
} er_s32g3_iface_t;

/* ==================== Route ==================== */

typedef struct {
    uint32_t route_id;
    char destination[64];
    char gateway[64];
    char interface[32];
    uint32_t metric;
    uint32_t protocol;
    char status[16];
    uint64_t packets;
    uint64_t bytes;
} er_s32g3_route_t;

/* ==================== QoS ==================== */

typedef struct {
    uint32_t queue_id;
    uint32_t port_id;
    uint32_t priority;
    uint32_t bandwidth_pct;
    uint32_t max_rate_mbps;
    uint32_t cur_rate_mbps;
    uint32_t drops;
    uint64_t bytes;
    uint64_t packets;
} er_s32g3_qos_queue_t;

/* ==================== API ==================== */

int er_s32g3_port_init(void);
int er_s32g3_port_exit(void);
int er_s32g3_port_get(uint32_t port_id, er_s32g3_port_t *port);
int er_s32g3_port_get_all(er_s32g3_port_t *ports, uint32_t *count);
int er_s32g3_port_configure(uint32_t port_id, const er_s32g3_port_t *port);
int er_s32g3_port_set_speed(uint32_t port_id, uint32_t speed_mbps);
int er_s32g3_port_set_mac(uint32_t port_id, const uint8_t *mac);

int er_s32g3_iface_init(void);
int er_s32g3_iface_exit(void);
int er_s32g3_iface_get(uint32_t iface_id, er_s32g3_iface_t *iface);
int er_s32g3_iface_get_all(er_s32g3_iface_t *ifaces, uint32_t *count);
int er_s32g3_iface_create(const er_s32g3_iface_t *iface);
int er_s32g3_iface_delete(uint32_t iface_id);

int er_s32g3_route_get(uint32_t route_id, er_s32g3_route_t *route);
int er_s32g3_route_get_all(er_s32g3_route_t *routes, uint32_t *count);
int er_s32g3_route_add(const er_s32g3_route_t *route);
int er_s32g3_route_delete(uint32_t route_id);

int er_s32g3_qos_get(uint32_t port_id, er_s32g3_qos_queue_t *queues, uint32_t *count);
int er_s32g3_qos_set(uint32_t port_id, const er_s32g3_qos_queue_t *queues, uint32_t count);

#endif /* S32G3_ER_H */
