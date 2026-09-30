/*
 * rtl960x_core.c — Core driver for Realtek RTL960x GPON OLT
 *
 * MTS-OLT-2000 GPON OLT — Основной драйвер чипа RTL960x
 * Поддерживает: GPON ports, ONU management, WDM monitoring
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/ptp_clock_kernel.h>
#include <uapi/linux/rtl960x.h>

#include "rtl960x.h"
#include "rtl960x_gpon.h"
#include "rtl960x_omci.h"
#include "rtl960x_wdm.h"

#define DRIVER_VERSION "1.0.0"
#define DRIVER_NAME "rtl960x"
#define RTL960X_MAX_DEVICES 4

/* Module parameters */
static int max_devices = RTL960X_MAX_DEVICES;
module_param(max_devices, int, 0444);
MODULE_PARM_DESC(max_devices, "Maximum number of RTL960x devices");

static int poll_interval = 1000; /* ms */
module_param(poll_interval, int, 0444);
MODULE_PARM_DESC(poll_interval, "Polling interval in milliseconds");

/* Global device list */
static struct rtl960x_device *rtl960x_devices[RTL960X_MAX_DEVICES];
static int rtl960x_device_count = 0;
static DEFINE_MUTEX(rtl960x_mutex);

/* Debug */
static int debug_level = 1;
module_param(debug_level, int, 0644);
MODULE_PARM_DESC(debug_level, "Debug level (0=off, 1=error, 2=info, 3=debug)");

#define rtl960x_dbg(dev, fmt, ...) \
    do { \
        if (debug_level >= 3) \
            dev_dbg((dev)->dev, fmt, ##__VA_ARGS__); \
    } while (0)

#define rtl960x_info(dev, fmt, ...) \
    do { \
        if (debug_level >= 2) \
            dev_info((dev)->dev, fmt, ##__VA_ARGS__); \
    } while (0)

#define rtl960x_err(dev, fmt, ...) \
    do { \
        if (debug_level >= 1) \
            dev_err((dev)->dev, fmt, ##__VA_ARGS__); \
    } while (0)

/* ==================== Device Management ==================== */

struct rtl960x_device *rtl960x_get_device(uint32_t id)
{
    struct rtl960x_device *dev;
    int i;

    mutex_lock(&rtl960x_mutex);
    for (i = 0; i < rtl960x_device_count; i++) {
        dev = rtl960x_devices[i];
        if (dev && dev->id == id) {
            mutex_unlock(&rtl960x_mutex);
            return dev;
        }
    }
    mutex_unlock(&rtl960x_mutex);
    return NULL;
}
EXPORT_SYMBOL(rtl960x_get_device);

static int rtl960x_add_device(struct rtl960x_device *dev)
{
    int i;

    mutex_lock(&rtl960x_mutex);
    for (i = 0; i < rtl960x_device_count; i++) {
        if (rtl960x_devices[i] && rtl960x_devices[i]->id == dev->id) {
            mutex_unlock(&rtl960x_mutex);
            return -EEXIST;
        }
    }
    if (rtl960x_device_count >= RTL960X_MAX_DEVICES) {
        mutex_unlock(&rtl960x_mutex);
        return -ENOSPC;
    }
    rtl960x_devices[rtl960x_device_count++] = dev;
    mutex_unlock(&rtl960x_mutex);
    return 0;
}

static void rtl960x_remove_device(uint32_t id)
{
    int i;

    mutex_lock(&rtl960x_mutex);
    for (i = 0; i < rtl960x_device_count; i++) {
        if (rtl960x_devices[i] && rtl960x_devices[i]->id == id) {
            rtl960x_devices[i] = NULL;
            memmove(&rtl960x_devices[i], &rtl960x_devices[i + 1],
                (rtl960x_device_count - i - 1) * sizeof(struct rtl960x_device *));
            rtl960x_device_count--;
            break;
        }
    }
    mutex_unlock(&rtl960x_mutex);
}

/* ==================== ONU Management ==================== */

int rtl960x_add_onu(struct rtl960x_device *dev, struct rtl960x_onu *onu)
{
    int i;

    if (!dev || !onu)
        return -EINVAL;

    mutex_lock(&dev->lock);

    /* Find free slot */
    for (i = 0; i < RTL960X_MAX_ONU; i++) {
        if (!dev->onus || dev->onus[i].id == 0) {
            if (dev->onus) {
                dev->onus[i].id = onu->id;
                memcpy(dev->onus[i].serial, onu->serial, sizeof(onu->serial));
                memcpy(dev->onus[i].mac, onu->mac, sizeof(onu->mac));
                dev->onus[i].pon_port = onu->pon_port;
                dev->onus[i].status = onu->status;
                dev->onus[i].power_level = onu->power_level;
                dev->onus[i].distance = onu->distance;
                dev->onus[i].vlan = onu->vlan;
                dev->onus[i].qos_profile = onu->qos_profile;
                dev->onus[i].bandwidth_up = onu->bandwidth_up;
                dev->onus[i].bandwidth_down = onu->bandwidth_down;
                dev->onus[i].last_seen = ktime_get_ns();
                dev->onus[i].created = dev->onus[i].last_seen;
                dev->num_onu++;
                rtl960x_dbg(dev, "ONU added: id=%u, mac=%s\n", onu->id, onu->mac);
                mutex_unlock(&dev->lock);
                return 0;
            }
            break;
        }
    }

    mutex_unlock(&dev->lock);
    return -ENOSPC;
}
EXPORT_SYMBOL(rtl960x_add_onu);

int rtl960x_remove_onu(struct rtl960x_device *dev, uint32_t onu_id)
{
    int i;

    if (!dev)
        return -EINVAL;

    mutex_lock(&dev->lock);
    for (i = 0; i < RTL960X_MAX_ONU; i++) {
        if (dev->onus && dev->onus[i].id == onu_id) {
            dev->onus[i].id = 0;
            dev->num_onu--;
            rtl960x_dbg(dev, "ONU removed: id=%u\n", onu_id);
            mutex_unlock(&dev->lock);
            return 0;
        }
    }
    mutex_unlock(&dev->lock);
    return -ENOENT;
}
EXPORT_SYMBOL(rtl960x_remove_onu);

int rtl960x_update_onu_status(struct rtl960x_device *dev, uint32_t onu_id, uint32_t status)
{
    int i;

    if (!dev)
        return -EINVAL;

    mutex_lock(&dev->lock);
    for (i = 0; i < RTL960X_MAX_ONU; i++) {
        if (dev->onus && dev->onus[i].id == onu_id) {
            dev->onus[i].status = status;
            dev->onus[i].last_seen = ktime_get_ns();
            mutex_unlock(&dev->lock);
            return 0;
        }
    }
    mutex_unlock(&dev->lock);
    return -ENOENT;
}
EXPORT_SYMBOL(rtl960x_update_onu_status);

int rtl960x_get_onu_count(struct rtl960x_device *dev)
{
    if (!dev)
        return -EINVAL;
    return dev->num_onu;
}
EXPORT_SYMBOL(rtl960x_get_onu_count);

/* ==================== Hardware Monitoring ==================== */

int rtl960x_set_tx_power(struct rtl960x_device *dev, uint32_t port_id, uint32_t power)
{
    if (!dev || port_id >= dev->num_ports)
        return -EINVAL;

    mutex_lock(&dev->lock);
    if (dev->ports) {
        dev->ports[port_id].tx_power = power;
    }
    mutex_unlock(&dev->lock);
    rtl960x_dbg(dev, "TX power set: port=%u, power=%u\n", port_id, power);
    return 0;
}
EXPORT_SYMBOL(rtl960x_set_tx_power);

int rtl960x_get_rx_power(struct rtl960x_device *dev, uint32_t port_id)
{
    int power = -EINVAL;

    if (!dev || port_id >= dev->num_ports)
        return power;

    mutex_lock(&dev->lock);
    if (dev->ports) {
        power = dev->ports[port_id].rx_power;
    }
    mutex_unlock(&dev->lock);
    return power;
}
EXPORT_SYMBOL(rtl960x_get_rx_power);

int rtl960x_get_temperature(struct rtl960x_device *dev)
{
    int temp = -EINVAL;

    if (!dev)
        return temp;

    mutex_lock(&dev->lock);
    if (dev->ports && dev->num_ports > 0) {
        temp = dev->ports[0].temperature;
    }
    mutex_unlock(&dev->lock);
    return temp;
}
EXPORT_SYMBOL(rtl960x_get_temperature);

int rtl960x_get_bias_current(struct rtl960x_device *dev)
{
    int bias = -EINVAL;

    if (!dev)
        return bias;

    mutex_lock(&dev->lock);
    if (dev->ports && dev->num_ports > 0) {
        bias = dev->ports[0].bias_current;
    }
    mutex_unlock(&dev->lock);
    return bias;
}
EXPORT_SYMBOL(rtl960x_get_bias_current);

/* ==================== Timer Handler ==================== */

static void rtl960x_timer_handler(struct timer_list *t)
{
    struct rtl960x_device *dev = from_timer(dev, t, timer);
    int i;

    rtl960x_dbg(dev, "Polling devices...\n");

    mutex_lock(&dev->lock);
    for (i = 0; i < dev->num_ports; i++) {
        if (dev->ports) {
            /* Update port statistics from hardware */
            dev->ports[i].rx_bytes = dev->ports[i].rx_bytes;
            dev->ports[i].tx_bytes = dev->ports[i].tx_bytes;
        }
    }

    /* Update ONU last_seen timestamps */
    if (dev->onus) {
        for (i = 0; i < RTL960X_MAX_ONU; i++) {
            if (dev->onus[i].id > 0) {
                uint64_t now = ktime_get_ns();
                if (now - dev->onus[i].last_seen > 60000000000ULL) { /* 60s */
                    dev->onus[i].status = 0; /* DISCONNECTED */
                }
            }
        }
    }
    mutex_unlock(&dev->lock);

    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(poll_interval));
}

/* ==================== Platform Driver ==================== */

static int rtl960x_probe(struct platform_device *pdev)
{
    struct rtl960x_device *dev;
    int ret;

    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->id = pdev->id;
    snprintf(dev->name, sizeof(dev->name), "rtl960x-%u", pdev->id);
    dev->state = RTL960X_STATE_INIT;
    dev->dev = &pdev->dev;
    mutex_init(&dev->lock);

    /* Allocate port array */
    dev->ports = devm_kcalloc(&pdev->dev, RTL960X_MAX_PORTS,
                   sizeof(struct rtl960x_gpon_port), GFP_KERNEL);
    if (dev->ports)
        dev->num_ports = RTL960X_MAX_PORTS;

    /* Allocate ONU array */
    dev->onus = devm_kcalloc(&pdev->dev, RTL960X_MAX_ONU,
                  sizeof(struct rtl960x_onu), GFP_KERNEL);

    /* Create workqueue */
    dev->wq = alloc_workqueue("rtl960x_wq", WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
    if (!dev->wq)
        return -ENOMEM;

    /* Setup timer */
    timer_setup(&dev->timer, rtl960x_timer_handler, 0);
    setup_timer(&dev->timer, rtl960x_timer_handler, (unsigned long)dev);
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(poll_interval));

    /* Register device */
    ret = rtl960x_add_device(dev);
    if (ret) {
        destroy_workqueue(dev->wq);
        return ret;
    }

    dev->state = RTL960X_STATE_READY;
    dev_info(&pdev->dev, "RTL960x GPON OLT driver loaded (version %s)\n", DRIVER_VERSION);

    platform_set_drvdata(pdev, dev);
    return 0;
}

static int rtl960x_remove(struct platform_device *pdev)
{
    struct rtl960x_device *dev = platform_get_drvdata(pdev);

    if (dev) {
        del_timer_sync(&dev->timer);
        destroy_workqueue(dev->wq);
        rtl960x_remove_device(dev->id);
        dev->state = RTL960X_STATE_STOPPED;
        dev_info(&pdev->dev, "RTL960x GPON OLT driver removed\n");
    }

    platform_set_drvdata(pdev, NULL);
    return 0;
}

static const struct platform_device_id rtl960x_ids[] = {
    { "rtl960x-gpon", 0 },
    { }
};
MODULE_DEVICE_TABLE(platform, rtl960x_ids);

static struct platform_driver rtl960x_driver = {
    .probe  = rtl960x_probe,
    .remove = rtl960x_remove,
    .id_table = rtl960x_ids,
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
    },
};

/* ==================== Module Init/Exit ==================== */

static int __init rtl960x_init_module(void)
{
    int ret;

    pr_info("RTL960x GPON OLT driver initializing (max_devices=%d, poll_interval=%d ms)\n",
        max_devices, poll_interval);

    ret = platform_driver_register(&rtl960x_driver);
    if (ret) {
        pr_err("Failed to register platform driver: %d\n", ret);
        return ret;
    }

    pr_info("RTL960x driver registered successfully\n");
    return 0;
}

static void __exit rtl960x_exit_module(void)
{
    platform_driver_unregister(&rtl960x_driver);
    pr_info("RTL960x driver unloaded\n");
}

module_init(rtl960x_init_module);
module_exit(rtl960x_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Team");
MODULE_DESCRIPTION("Realtek RTL960x GPON OLT Driver");
MODULE_VERSION(DRIVER_VERSION);
