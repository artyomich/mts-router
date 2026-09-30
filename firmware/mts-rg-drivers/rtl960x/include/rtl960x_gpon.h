/*
 * rtl960x_gpon.h — GPON port interface для residential gateway
 *
 * MTS-RG-500 Residential Gateway — Упрощенный драйвер GPON
 */

#ifndef RTL960X_GPON_RG_H
#define RTL960X_GPON_RG_H

#include <linux/types.h>

#define RG_GPON_MAX_PORTS 4
#define RG_GPON_MAX_ONU 64
#define RG_GPON_MTU 1500

/* GPON port states for residential use */
enum rg_gpon_port_state {
    RG_GPON_PORT_DOWN = 0,
    RG_GPON_PORT_UP,
    RG_GPON_PORT_MAINTENANCE,
    RG_GPON_PORT_FAULT
};

/* ONU registration status */
enum rg_onu_status {
    RG_ONU_UNKNOWN = 0,
    RG_ONU_DISCONNECTED,
    RG_ONU_REGISTERED,
    RG_GPON_OPERATIONAL
};

/* GPON port configuration structure */
struct rg_gpon_port_cfg {
    uint32_t port_id;
    char name[32];
    uint32_t mode;           /* ONU mode for residential */
    int32_t tx_power;        /* dBm, -40 to +60 */
    int32_t rx_power;        /* dBm, -40 to +60 */
    uint32_t vlan_mode;      /* 0=none, 1=stack, 2=swap, 3=push, 4=pop */
    uint32_t vlan_id;
    uint8_t mac[6];
    uint32_t status;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t tx_packets;
    uint64_t rx_packets;
};

/* ONU info structure */
struct rg_onu_info {
    uint32_t onu_id;
    uint8_t mac[6];
    char serial[20];
    uint32_t state;
    int32_t rx_power;
    int32_t distance_us;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
};

/* API functions */
int rg_gpon_port_init(void);
int rg_gpon_port_exit(void);
int rg_gpon_port_configure(uint32_t port_id, const struct rg_gpon_port_cfg *cfg);
int rg_gpon_port_get_status(uint32_t port_id, struct rg_gpon_port_cfg *cfg);
int rg_gpon_port_enable(uint32_t port_id);
int rg_gpon_port_disable(uint32_t port_id);
int rg_gpon_get_onu_list(uint32_t port_id, struct rg_onu_info *onus, uint32_t max_onus, uint32_t *count);
int rg_gpon_set_vlan(uint32_t port_id, uint32_t mode, uint32_t vlan_id);
int rg_gpon_get_power_monitor(uint32_t port_id, int32_t *rx_power, int32_t *tx_power);

/* Notification callbacks */
typedef void (*rg_gpon_port_event_cb)(uint32_t port_id, uint32_t event, void *priv);
typedef void (*rg_gpon_onu_event_cb)(uint32_t port_id, uint32_t onu_id, uint32_t event, void *priv);

int rg_gpon_register_port_event_cb(rg_gpon_port_event_cb cb, void *priv);
int rg_gpon_register_onu_event_cb(rg_gpon_onu_event_cb cb, void *priv);

#endif /* RTL960X_GPON_RG_H */
