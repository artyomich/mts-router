/*
 * s32g3_er.c — S32G3 driver for MTS Enterprise Router
 *
 * MTS-ER-1000 Enterprise Router — Основной драйвер чипа S32G3
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

#include "s32g3_er.h"

#define DRIVER_VERSION "1.0.0-er"
#define DRIVER_NAME "s32g3_er"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Project");
MODULE_DESCRIPTION("S32G3 driver for enterprise router");
MODULE_VERSION(DRIVER_VERSION);

static int debug_level = 1;
module_param(debug_level, int, 0644);
MODULE_PARM_DESC(debug_level, "Debug level (0=off, 1=error, 2=info, 3=debug)");

#define er_s32g3_dbg(fmt, ...) \
    do { if (debug_level >= 3) pr_debug(fmt, ##__VA_ARGS__); } while (0)

#define er_s32g3_info(fmt, ...) \
    do { if (debug_level >= 2) pr_info(fmt, ##__VA_ARGS__); } while (0)

#define er_s32g3_err(fmt, ...) \
    do { if (debug_level >= 1) pr_err(fmt, ##__VA_ARGS__); } while (0)

/* ==================== Port Management ==================== */

static er_s32g3_port_t s32g3_ports[ER_S32G3_MAX_PORTS];
static DEFINE_MUTEX(s32g3_mutex);

int er_s32g3_port_init(void)
{
    int i;

    er_s32g3_info("Initializing S32G3 port driver v%s\n", DRIVER_VERSION);

    mutex_lock(&s32g3_mutex);
    for (i = 0; i < ER_S32G3_MAX_PORTS; i++) {
        memset(&s32g3_ports[i], 0, sizeof(er_s32g3_port_t));
        s32g3_ports[i].port_id = i;
        snprintf(s32g3_ports[i].name, sizeof(s32g3_ports[i].name),
                 "eth-%d", i);
        strcpy(s32g3_ports[i].type, ER_PORT_GBE);
        s32g3_ports[i].speed_mbps = 1000;
        strcpy(s32g3_ports[i].status, "down");
        strcpy(s32g3_ports[i].mode, ER_PORT_MODE_ROUTED);
    }
    mutex_unlock(&s32g3_mutex);

    er_s32g3_info("S32G3 port driver initialized with %d ports\n",
                   ER_S32G3_MAX_PORTS);
    return 0;
}

int er_s32g3_port_exit(void)
{
    int i;

    mutex_lock(&s32g3_mutex);
    for (i = 0; i < ER_S32G3_MAX_PORTS; i++)
        strcpy(s32g3_ports[i].status, "down");
    mutex_unlock(&s32g3_mutex);

    er_s32g3_info("S32G3 port driver unloaded\n");
    return 0;
}

int er_s32g3_port_get(uint32_t port_id, er_s32g3_port_t *port)
{
    if (port_id >= ER_S32G3_MAX_PORTS)
        return -EINVAL;
    if (!port)
        return -EINVAL;

    mutex_lock(&s32g3_mutex);
    memcpy(port, &s32g3_ports[port_id], sizeof(er_s32g3_port_t));
    mutex_unlock(&s32g3_mutex);
    return 0;
}

int er_s32g3_port_get_all(er_s32g3_port_t *ports, uint32_t *count)
{
    int i;

    if (!ports || !count)
        return -EINVAL;

    mutex_lock(&s32g3_mutex);
    for (i = 0; i < ER_S32G3_MAX_PORTS; i++)
        memcpy(&ports[i], &s32g3_ports[i], sizeof(er_s32g3_port_t));
    *count = ER_S32G3_MAX_PORTS;
    mutex_unlock(&s32g3_mutex);
    return 0;
}

int er_s32g3_port_configure(uint32_t port_id, const er_s32g3_port_t *port)
{
    if (port_id >= ER_S32G3_MAX_PORTS || !port)
        return -EINVAL;

    mutex_lock(&s32g3_mutex);
    memcpy(&s32g3_ports[port_id], port, sizeof(er_s32g3_port_t));
    mutex_unlock(&s32g3_mutex);
    return 0;
}

/* ==================== Interface Management ==================== */

static er_s32g3_iface_t s32g3_ifaces[ER_S32G3_MAX_INTERFACES];
static int iface_count = 0;

int er_s32g3_iface_init(void)
{
    er_s32g3_info("Initializing S32G3 interface driver\n");
    iface_count = 0;
    return 0;
}

int er_s32g3_iface_exit(void)
{
    er_s32g3_info("S32G3 interface driver unloaded\n");
    iface_count = 0;
    return 0;
}

int er_s32g3_iface_get(uint32_t iface_id, er_s32g3_iface_t *iface)
{
    if (iface_id >= ER_S32G3_MAX_INTERFACES || !iface)
        return -EINVAL;

    if (iface_id >= (uint32_t)iface_count)
        return -ENOENT;

    memcpy(iface, &s32g3_ifaces[iface_id], sizeof(er_s32g3_iface_t));
    return 0;
}

int er_s32g3_iface_get_all(er_s32g3_iface_t *ifaces, uint32_t *count)
{
    int i;

    if (!ifaces || !count)
        return -EINVAL;

    for (i = 0; i < iface_count; i++)
        memcpy(&ifaces[i], &s32g3_ifaces[i], sizeof(er_s32g3_iface_t));
    *count = iface_count;
    return 0;
}

int er_s32g3_iface_create(const er_s32g3_iface_t *iface)
{
    if (!iface || iface_count >= ER_S32G3_MAX_INTERFACES)
        return -EINVAL;

    memcpy(&s32g3_ifaces[iface_count], iface, sizeof(er_s32g3_iface_t));
    er_s32g3_info("Created interface %s (id=%d)\n", iface->name, iface_count);
    iface_count++;
    return 0;
}

int er_s32g3_iface_delete(uint32_t iface_id)
{
    if (iface_id >= (uint32_t)iface_count)
        return -EINVAL;

    er_s32g3_info("Deleted interface id=%d\n", iface_id);
    iface_count--;
    if (iface_id < (uint32_t)iface_count)
        memmove(&s32g3_ifaces[iface_id], &s32g3_ifaces[iface_id + 1],
                (iface_count - iface_id) * sizeof(er_s32g3_iface_t));
    return 0;
}

/* ==================== Route Management ==================== */

static er_s32g3_route_t s32g3_routes[ER_S32G3_MAX_ROUTES];
static int route_count = 0;

int er_s32g3_route_get(uint32_t route_id, er_s32g3_route_t *route)
{
    if (route_id >= ER_S32G3_MAX_ROUTES || !route)
        return -EINVAL;

    if (route_id >= (uint32_t)route_count)
        return -ENOENT;

    memcpy(route, &s32g3_routes[route_id], sizeof(er_s32g3_route_t));
    return 0;
}

int er_s32g3_route_get_all(er_s32g3_route_t *routes, uint32_t *count)
{
    int i;

    if (!routes || !count)
        return -EINVAL;

    for (i = 0; i < route_count; i++)
        memcpy(&routes[i], &s32g3_routes[i], sizeof(er_s32g3_route_t));
    *count = route_count;
    return 0;
}

int er_s32g3_route_add(const er_s32g3_route_t *route)
{
    if (!route || route_count >= ER_S32G3_MAX_ROUTES)
        return -EINVAL;

    memcpy(&s32g3_routes[route_count], route, sizeof(er_s32g3_route_t));
    er_s32g3_info("Added route to %s via %s\n", route->destination, route->gateway);
    route_count++;
    return 0;
}

int er_s32g3_route_delete(uint32_t route_id)
{
    if (route_id >= (uint32_t)route_count)
        return -EINVAL;

    route_count--;
    if (route_id < (uint32_t)route_count)
        memmove(&s32g3_routes[route_id], &s32g3_routes[route_id + 1],
                (route_count - route_id) * sizeof(er_s32g3_route_t));
    return 0;
}

/* ==================== QoS ==================== */

int er_s32g3_qos_get(uint32_t port_id, er_s32g3_qos_queue_t *queues, uint32_t *count)
{
    if (port_id >= ER_S32G3_MAX_PORTS || !queues || !count)
        return -EINVAL;

    /* Initialize default QoS queues */
    for (uint32_t i = 0; i < ER_S32G3_MAX_QOS_QUEUES; i++) {
        queues[i].queue_id = i;
        queues[i].port_id = port_id;
        queues[i].priority = i;
        queues[i].bandwidth_pct = 100 / ER_S32G3_MAX_QOS_QUEUES;
        queues[i].drops = 0;
        queues[i].bytes = 0;
        queues[i].packets = 0;
    }
    *count = ER_S32G3_MAX_QOS_QUEUES;
    return 0;
}

int er_s32g3_qos_set(uint32_t port_id, const er_s32g3_qos_queue_t *queues, uint32_t count)
{
    if (port_id >= ER_S32G3_MAX_PORTS || !queues || count > ER_S32G3_MAX_QOS_QUEUES)
        return -EINVAL;

    er_s32g3_info("Set QoS for port %u (%u queues)\n", port_id, count);
    return 0;
}

/* ==================== Init/Exit ==================== */

static int __init s32g3_er_init(void)
{
    return er_s32g3_port_init();
}

static void __exit s32g3_er_exit(void)
{
    er_s32g3_port_exit();
    er_s32g3_iface_exit();
}

module_init(s32g3_er_init);
module_exit(s32g3_er_exit);
