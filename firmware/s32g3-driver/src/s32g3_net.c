// SPDX-License-Identifier: GPL-2.0
//
// s32g3_net.c - S32G3 Network Subsystem Implementation
//
// MTS-MB-3000 Mobile Backhaul S32G3 Driver

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/mutex.h>
#include <linux/phy.h>

#include "s32g3.h"
#include "s32g3_net.h"

#define DRIVER_NAME "s32g3_net"
#define NET_MAGIC 0x534E4554  /* "SNET" */

static struct s32g3_net_device *net_dev;

int s32g3_net_port_init(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    struct s32g3_net_port *port = &net_dev->ports[port_id];
    port->speed = 100000000;
    port->duplex = 1;
    port->status = 1;
    port->mtu = S32G3_NET_MTU_DEFAULT;
    port->fec = 1;
    port->autoneg = 1;
    port->mode = S32G3_NET_MODE_SGMII;
    
    return 0;
}

int s32g3_net_port_enable(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    net_dev->ports[port_id].status = 1;
    return 0;
}

int s32g3_net_port_disable(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    net_dev->ports[port_id].status = 0;
    return 0;
}

int s32g3_net_port_set_mtu(uint32_t port_id, uint32_t mtu)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    if (mtu < S32G3_NET_MTU_MIN || mtu > S32G3_NET_MTU_MAX)
        return -EINVAL;
    net_dev->ports[port_id].mtu = mtu;
    return 0;
}

int s32g3_net_port_get_mtu(uint32_t port_id, uint32_t *mtu)
{
    if (!net_dev || port_id >= net_dev->num_ports || !mtu)
        return -EINVAL;
    *mtu = net_dev->ports[port_id].mtu;
    return 0;
}

int s32g3_net_port_set_speed(uint32_t port_id, uint32_t speed)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    net_dev->ports[port_id].speed = speed;
    return 0;
}

int s32g3_net_port_get_speed(uint32_t port_id, uint32_t *speed)
{
    if (!net_dev || port_id >= net_dev->num_ports || !speed)
        return -EINVAL;
    *speed = net_dev->ports[port_id].speed;
    return 0;
}

int s32g3_net_port_get_stats(uint32_t port_id,
                              struct s32g3_net_port *stats)
{
    if (!net_dev || port_id >= net_dev->num_ports || !stats)
        return -EINVAL;
    *stats = net_dev->ports[port_id];
    return 0;
}

int s32g3_net_port_add_mac(uint32_t port_id, const uint8_t *mac)
{
    if (!net_dev || port_id >= net_dev->num_ports || !mac)
        return -EINVAL;
    
    struct s32g3_net_port *port = &net_dev->ports[port_id];
    if (port->num_macs >= 128)
        return -ENOSPC;
    
    if (!port->macs) {
        port->macs = kcalloc(128, sizeof(*port->macs), GFP_KERNEL);
        if (!port->macs)
            return -ENOMEM;
    }
    
    struct s32g3_net_mac *m = &port->macs[port->num_macs];
    m->id = port->num_macs;
    memcpy(m->address, mac, 6);
    m->active = 1;
    strscpy(m->name, "default", sizeof(m->name));
    port->num_macs++;
    
    return 0;
}

int s32g3_net_port_get_mac(uint32_t port_id, uint32_t mac_id,
                            uint8_t *mac)
{
    if (!net_dev || port_id >= net_dev->num_ports || !mac)
        return -EINVAL;
    
    struct s32g3_net_port *port = &net_dev->ports[port_id];
    if (mac_id >= port->num_macs)
        return -EINVAL;
    
    memcpy(mac, port->macs[mac_id].address, 6);
    return 0;
}

int s32g3_net_port_link_up(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    net_dev->ports[port_id].status = 1;
    return 0;
}

int s32g3_net_port_link_down(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    net_dev->ports[port_id].status = 0;
    return 0;
}

int s32g3_net_port_get_link_status(uint32_t port_id, uint32_t *status)
{
    if (!net_dev || port_id >= net_dev->num_ports || !status)
        return -EINVAL;
    *status = net_dev->ports[port_id].status;
    return 0;
}

int s32g3_net_init(void)
{
    net_dev = kzalloc(sizeof(*net_dev), GFP_KERNEL);
    if (!net_dev)
        return -ENOMEM;
    
    net_dev->id = 0;
    strscpy(net_dev->name, "s32g3-net", sizeof(net_dev->name));
    net_dev->num_ports = 32;
    net_dev->ports = kcalloc(32, sizeof(*net_dev->ports), GFP_KERNEL);
    if (!net_dev->ports) {
        kfree(net_dev);
        net_dev = NULL;
        return -ENOMEM;
    }
    
    pr_info("s32g3_net: Network subsystem initialized\n");
    return 0;
}

void s32g3_net_exit(void)
{
    if (net_dev) {
        kfree(net_dev->ports);
        kfree(net_dev);
        net_dev = NULL;
    }
    pr_info("s32g3_net: Network subsystem exited\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Firmware Agent");
MODULE_DESCRIPTION("S32G3 Network Subsystem");
MODULE_VERSION("1.0.0");
