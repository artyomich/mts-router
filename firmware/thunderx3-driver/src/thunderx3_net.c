// SPDX-License-Identifier: GPL-2.0
//
// thunderx3_net.c - ThunderX3 Network Subsystem Implementation
//
// MTS-MC-5000 Mobile Core ThunderX3 Driver
//
// Copyright (c) 2024 MTS Router Project

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/mutex.h>
#include <linux/phy.h>
#include <linux/i2c.h>

#include "thunderx3.h"
#include "thunderx3_net.h"

#define DRIVER_NAME "thunderx3_net"
#define NET_MAGIC 0x4E455433  /* "NET3" */

/* ============================================================================ */
/* Internal state                                                               */
/* ============================================================================ */

static struct thunderx3_net_device *net_dev;

/* ============================================================================ */
/* Network port management                                                      */
/* ============================================================================ */

int thunderx3_net_port_init(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    struct thunderx3_net_port *port = &net_dev->ports[port_id];
    port->speed = THUNDERX3_NET_SPEED_100G;
    port->duplex = 1;
    port->status = 1;
    port->mtu = THUNDERX3_NET_MTU_DEFAULT;
    port->fec = THUNDERX3_NET_FEC_AUTO;
    port->autoneg = 1;
    port->mode = THUNDERX3_NET_MODE_100GBASE_R;
    
    return 0;
}

int thunderx3_net_port_enable(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    net_dev->ports[port_id].status = 1;
    return 0;
}

int thunderx3_net_port_disable(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    net_dev->ports[port_id].status = 0;
    return 0;
}

int thunderx3_net_port_set_mtu(uint32_t port_id, uint32_t mtu)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    if (mtu < THUNDERX3_NET_MTU_MIN || mtu > THUNDERX3_NET_MTU_MAX)
        return -EINVAL;
    
    net_dev->ports[port_id].mtu = mtu;
    return 0;
}

int thunderx3_net_port_get_mtu(uint32_t port_id, uint32_t *mtu)
{
    if (!net_dev || port_id >= net_dev->num_ports || !mtu)
        return -EINVAL;
    
    *mtu = net_dev->ports[port_id].mtu;
    return 0;
}

int thunderx3_net_port_set_speed(uint32_t port_id, uint32_t speed)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    net_dev->ports[port_id].speed = speed;
    return 0;
}

int thunderx3_net_port_get_speed(uint32_t port_id, uint32_t *speed)
{
    if (!net_dev || port_id >= net_dev->num_ports || !speed)
        return -EINVAL;
    
    *speed = net_dev->ports[port_id].speed;
    return 0;
}

int thunderx3_net_port_set_fec(uint32_t port_id, uint32_t fec)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    net_dev->ports[port_id].fec = fec;
    return 0;
}

int thunderx3_net_port_get_fec(uint32_t port_id, uint32_t *fec)
{
    if (!net_dev || port_id >= net_dev->num_ports || !fec)
        return -EINVAL;
    
    *fec = net_dev->ports[port_id].fec;
    return 0;
}

int thunderx3_net_port_get_stats(uint32_t port_id,
                                  struct thunderx3_net_port *stats)
{
    if (!net_dev || port_id >= net_dev->num_ports || !stats)
        return -EINVAL;
    
    *stats = net_dev->ports[port_id];
    return 0;
}

int thunderx3_net_port_add_mac(uint32_t port_id, const uint8_t *mac)
{
    if (!net_dev || port_id >= net_dev->num_ports || !mac)
        return -EINVAL;
    
    struct thunderx3_net_port *port = &net_dev->ports[port_id];
    
    if (port->num_macs >= THUNDERX3_NET_MAX_MACS)
        return -ENOSPC;
    
    if (!port->macs) {
        port->macs = kcalloc(THUNDERX3_NET_MAX_MACS,
                              sizeof(*port->macs), GFP_KERNEL);
        if (!port->macs)
            return -ENOMEM;
    }
    
    struct thunderx3_net_mac *m = &port->macs[port->num_macs];
    m->id = port->num_macs;
    memcpy(m->address, mac, 6);
    m->active = 1;
    strscpy(m->name, "default", sizeof(m->name));
    port->num_macs++;
    
    return 0;
}

int thunderx3_net_port_del_mac(uint32_t port_id, const uint32_t mac_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    struct thunderx3_net_port *port = &net_dev->ports[port_id];
    
    if (mac_id >= port->num_macs)
        return -EINVAL;
    
    port->macs[mac_id].active = 0;
    return 0;
}

int thunderx3_net_port_get_mac(uint32_t port_id, uint32_t mac_id,
                                uint8_t *mac)
{
    if (!net_dev || port_id >= net_dev->num_ports || !mac)
        return -EINVAL;
    
    struct thunderx3_net_port *port = &net_dev->ports[port_id];
    
    if (mac_id >= port->num_macs)
        return -EINVAL;
    
    memcpy(mac, port->macs[mac_id].address, 6);
    return 0;
}

int thunderx3_net_port_set_vlan(uint32_t port_id, uint32_t vlan_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    if (vlan_id >= THUNDERX3_NET_VLAN_TABLE_SIZE)
        return -EINVAL;
    
    net_dev->vlan_table[vlan_id] = port_id;
    return 0;
}

int thunderx3_net_port_get_vlan(uint32_t port_id, uint32_t *vlan_id)
{
    if (!net_dev || port_id >= net_dev->num_ports || !vlan_id)
        return -EINVAL;
    
    *vlan_id = 0;
    return 0;
}

int thunderx3_net_port_link_up(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    net_dev->ports[port_id].status = 1;
    return 0;
}

int thunderx3_net_port_link_down(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    net_dev->ports[port_id].status = 0;
    return 0;
}

int thunderx3_net_port_get_link_status(uint32_t port_id, uint32_t *status)
{
    if (!net_dev || port_id >= net_dev->num_ports || !status)
        return -EINVAL;
    
    *status = net_dev->ports[port_id].status;
    return 0;
}

int thunderx3_net_port_an_restart(uint32_t port_id)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    net_dev->ports[port_id].autoneg = 1;
    return 0;
}

int thunderx3_net_port_loopback_set(uint32_t port_id, int enable)
{
    if (!net_dev || port_id >= net_dev->num_ports)
        return -EINVAL;
    
    return 0;
}

/* ============================================================================ */
/* Network init/exit                                                            */
/* ============================================================================ */

int thunderx3_net_init(struct thunderx3_device *dev)
{
    if (!dev)
        return -EINVAL;
    
    net_dev = kzalloc(sizeof(*net_dev), GFP_KERNEL);
    if (!net_dev)
        return -ENOMEM;
    
    net_dev->id = 0;
    strscpy(net_dev->name, "thunderx3-net", sizeof(net_dev->name));
    net_dev->num_ports = dev->num_ports;
    net_dev->ports = dev->ports;
    
    pr_info("thunderx3_net: Network subsystem initialized\n");
    return 0;
}

void thunderx3_net_exit(struct thunderx3_device *dev)
{
    if (net_dev) {
        kfree(net_dev->macs);
        kfree(net_dev);
        net_dev = NULL;
    }
    
    pr_info("thunderx3_net: Network subsystem exited\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Firmware Agent");
MODULE_DESCRIPTION("ThunderX3 Network Subsystem");
MODULE_VERSION("1.0.0");
