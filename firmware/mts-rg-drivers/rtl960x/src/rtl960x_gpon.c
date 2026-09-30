/*
 * rtl960x_gpon.c — GPON port management for residential gateway
 *
 * MTS-RG-500 Residential Gateway — Упрощенный драйвер GPON ONU
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "rtl960x_gpon.h"

#define DRIVER_VERSION "1.0.0-rg"
#define DRIVER_NAME "rtl960x_gpon_rg"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Project");
MODULE_DESCRIPTION("RTL960x GPON driver for residential gateway");
MODULE_VERSION(DRIVER_VERSION);

/* Module parameters */
static int max_ports = RG_GPON_MAX_PORTS;
module_param(max_ports, int, 0444);
MODULE_PARM_DESC(max_ports, "Maximum number of GPON ports");

static int debug_level = 1;
module_param(debug_level, int, 0644);
MODULE_PARM_DESC(debug_level, "Debug level (0=off, 1=error, 2=info, 3=debug)");

#define rg_gpon_dbg(fmt, ...) \
    do { if (debug_level >= 3) pr_debug(fmt, ##__VA_ARGS__); } while (0)

#define rg_gpon_info(fmt, ...) \
    do { if (debug_level >= 2) pr_info(fmt, ##__VA_ARGS__); } while (0)

#define rg_gpon_err(fmt, ...) \
    do { if (debug_level >= 1) pr_err(fmt, ##__VA_ARGS__); } while (0)

/* ==================== Port Management ==================== */

static struct rg_gpon_port_cfg gpon_ports[RG_GPON_MAX_PORTS];
static DEFINE_MUTEX(gpon_mutex);

static int rg_gpon_validate_mac(const uint8_t *mac)
{
    /* Check for multicast or broadcast */
    if (mac[0] & 0x01)
        return -EINVAL;
    /* Check for unicast zero MAC */
    if (mac[0] == 0 && mac[1] == 0 && mac[2] == 0 &&
        mac[3] == 0 && mac[4] == 0 && mac[5] == 0)
        return -EINVAL;
    return 0;
}

int rg_gpon_port_configure(uint32_t port_id, const struct rg_gpon_port_cfg *cfg)
{
    int ret;

    if (port_id >= RG_GPON_MAX_PORTS)
        return -EINVAL;

    mutex_lock(&gpon_mutex);

    /* Validate MAC address */
    if (cfg->mac[0] || cfg->mac[1] || cfg->mac[2] ||
        cfg->mac[3] || cfg->mac[4] || cfg->mac[5]) {
        ret = rg_gpon_validate_mac(cfg->mac);
        if (ret) {
            rg_gpon_err("Invalid MAC address on port %u\n", port_id);
            mutex_unlock(&gpon_mutex);
            return ret;
        }
    }

    /* Copy configuration */
    memcpy(&gpon_ports[port_id], cfg, sizeof(struct rg_gpon_port_cfg));
    gpon_ports[port_id].status = RG_GPON_PORT_UP;

    rg_gpon_info("Port %u configured: mode=%u vlan=%u\n",
                 port_id, cfg->mode, cfg->vlan_id);

    mutex_unlock(&gpon_mutex);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_port_configure);

int rg_gpon_port_get_status(uint32_t port_id, struct rg_gpon_port_cfg *cfg)
{
    if (port_id >= RG_GPON_MAX_PORTS)
        return -EINVAL;

    mutex_lock(&gpon_mutex);
    memcpy(cfg, &gpon_ports[port_id], sizeof(struct rg_gpon_port_cfg));
    mutex_unlock(&gpon_mutex);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_port_get_status);

int rg_gpon_port_enable(uint32_t port_id)
{
    if (port_id >= RG_GPON_MAX_PORTS)
        return -EINVAL;

    mutex_lock(&gpon_mutex);
    gpon_ports[port_id].status = RG_GPON_PORT_UP;
    rg_gpon_info("Port %u enabled\n", port_id);
    mutex_unlock(&gpon_mutex);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_port_enable);

int rg_gpon_port_disable(uint32_t port_id)
{
    if (port_id >= RG_GPON_MAX_PORTS)
        return -EINVAL;

    mutex_lock(&gpon_mutex);
    gpon_ports[port_id].status = RG_GPON_PORT_DOWN;
    rg_gpon_info("Port %u disabled\n", port_id);
    mutex_unlock(&gpon_mutex);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_port_disable);

/* ==================== ONU Management ==================== */

int rg_gpon_get_onu_list(uint32_t port_id, struct rg_onu_info *onus,
                         uint32_t max_onus, uint32_t *count)
{
    if (port_id >= RG_GPON_MAX_PORTS)
        return -EINVAL;
    if (!onus || !count)
        return -EINVAL;

    /* In residential mode, typically 0-2 ONUs connected */
    *count = 0;
    rg_gpon_dbg("Querying ONU list for port %u\n", port_id);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_get_onu_list);

int rg_gpon_set_vlan(uint32_t port_id, uint32_t mode, uint32_t vlan_id)
{
    if (port_id >= RG_GPON_MAX_PORTS)
        return -EINVAL;
    if (mode > 4)
        return -EINVAL;

    mutex_lock(&gpon_mutex);
    gpon_ports[port_id].vlan_mode = mode;
    gpon_ports[port_id].vlan_id = vlan_id;
    rg_gpon_info("Port %u VLAN: mode=%u id=%u\n", port_id, mode, vlan_id);
    mutex_unlock(&gpon_mutex);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_set_vlan);

int rg_gpon_get_power_monitor(uint32_t port_id, int32_t *rx_power, int32_t *tx_power)
{
    if (port_id >= RG_GPON_MAX_PORTS)
        return -EINVAL;

    /* Return simulated power levels for residential use */
    if (rx_power)
        *rx_power = -250; /* -25.0 dBm */
    if (tx_power)
        *tx_power = +100; /* +10.0 dBm */
    return 0;
}
EXPORT_SYMBOL(rg_gpon_get_power_monitor);

/* ==================== Event Callbacks ==================== */

static rg_gpon_port_event_cb port_event_cb;
static void *port_event_priv;
static rg_gpon_onu_event_cb onu_event_cb;
static void *onu_event_priv;

int rg_gpon_register_port_event_cb(rg_gpon_port_event_cb cb, void *priv)
{
    mutex_lock(&gpon_mutex);
    port_event_cb = cb;
    port_event_priv = priv;
    mutex_unlock(&gpon_mutex);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_register_port_event_cb);

int rg_gpon_register_onu_event_cb(rg_gpon_onu_event_cb cb, void *priv)
{
    mutex_lock(&gpon_mutex);
    onu_event_cb = cb;
    onu_event_priv = priv;
    mutex_unlock(&gpon_mutex);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_register_onu_event_cb);

/* ==================== Init/Exit ==================== */

int rg_gpon_port_init(void)
{
    int i;

    rg_gpon_info("Initializing RTL960x GPON driver v%s\n", DRIVER_VERSION);

    mutex_lock(&gpon_mutex);
    for (i = 0; i < RG_GPON_MAX_PORTS; i++) {
        memset(&gpon_ports[i], 0, sizeof(struct rg_gpon_port_cfg));
        gpon_ports[i].status = RG_GPON_PORT_DOWN;
        snprintf(gpon_ports[i].name, sizeof(gpon_ports[i].name),
                 "gpon-%d", i);
    }
    mutex_unlock(&gpon_mutex);

    rg_gpon_info("GPON driver initialized with %d ports\n", max_ports);
    return 0;
}
EXPORT_SYMBOL(rg_gpon_port_init);

int rg_gpon_port_exit(void)
{
    int i;

    mutex_lock(&gpon_mutex);
    for (i = 0; i < RG_GPON_MAX_PORTS; i++) {
        gpon_ports[i].status = RG_GPON_PORT_DOWN;
    }
    mutex_unlock(&gpon_mutex);

    rg_gpon_info("GPON driver unloaded\n");
    return 0;
}
EXPORT_SYMBOL(rg_gpon_port_exit);

module_init(rg_gpon_port_init);
module_exit(rg_gpon_port_exit);
