// SPDX-License-Identifier: GPL-2.0
//
// s32g3_core.c - NXP S32G3 SoC Driver Core
//
// MTS-MB-3000 Mobile Backhaul S32G3 Driver
//
// Copyright (c) 2024 MTS Router Project

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/thermal.h>
#include <linux/hwmon.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/iopoll.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "s32g3.h"

#define DRIVER_NAME "s32g3"
#define DRIVER_VERSION "1.0.0"
#define S32G3_DRIVER_AUTHOR "MTS Router Firmware Agent"
#define S32G3_DRIVER_DESC "NXP S32G3 Automotive SoC Driver"

#define S32G3_MAX_DEVICES_GLOBAL 2
#define S32G3_DEFAULT_POLL_INTERVAL_MS 100
#define S32G3_RESET_TIMEOUT_MS 5000
#define S32G3_MAGIC_VALUE 0x53333247  /* "S3G2" */

static struct s32g3_device *s32g3_devices[S32G3_MAX_DEVICES_GLOBAL];
static int s32g3_device_count;
static struct class *s32g3_class;
static struct mutex s32g3_mutex;

static inline int s32g3_reg_read(struct s32g3_device *dev,
                                  uint32_t offset, uint32_t *value)
{
    if (!dev || !dev->reg_base || !value)
        return -EINVAL;
    *value = readl(dev->reg_base + offset);
    return 0;
}

static inline int s32g3_reg_write(struct s32g3_device *dev,
                                   uint32_t offset, uint32_t value)
{
    if (!dev || !dev->reg_base)
        return -EINVAL;
    writel(value, dev->reg_base + offset);
    return 0;
}

static struct s32g3_device *s32g3_alloc_device(void)
{
    struct s32g3_device *dev;
    
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return NULL;
    
    dev->magic = S32G3_MAGIC_VALUE;
    dev->state = S32G3_STATE_INIT;
    dev->num_cores = 0;
    dev->num_ports = 0;
    
    mutex_init(&dev->lock);
    init_timer(&dev->timer);
    dev->timer.data = (unsigned long)dev;
    
    dev->wq = alloc_workqueue("s32g3_wq", WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
    if (!dev->wq) {
        kfree(dev);
        return NULL;
    }
    
    spin_lock_init(&dev->stat_lock);
    
    return dev;
}

static void s32g3_free_device(struct s32g3_device *dev)
{
    if (!dev)
        return;
    
    if (dev->wq)
        destroy_workqueue(dev->wq);
    
    if (dev->cores)
        kfree(dev->cores);
    
    if (dev->ports)
        kfree(dev->ports);
    
    if (dev->ptp)
        kfree(dev->ptp);
    
    if (dev->sync)
        kfree(dev->sync);
    
    if (dev->netdev)
        unregister_netdev(dev->netdev);
    
    if (dev->reg_base)
        iounmap(dev->reg_base);
    
    if (dev->dma_buf)
        dma_free_coherent(&dev->dev->dev, dev->dma_buf_size,
                         dev->dma_buf, dev->dma_handle);
    
    if (dev->debugfs_dir)
        debugfs_remove(dev->debugfs_dir);
    
    kfree(dev);
}

static int s32g3_hw_init(struct s32g3_device *dev)
{
    int ret;
    uint32_t val;
    
    ret = s32g3_reg_write(dev, 0x0000, 0x1);
    if (ret)
        return ret;
    
    ret = readl_poll_timeout(dev->reg_base, val, !(val & 0x1),
                             100, S32G3_RESET_TIMEOUT_MS * 1000);
    if (ret) {
        pr_err("s32g3: SoC reset timeout\n");
        return -ETIMEDOUT;
    }
    
    s32g3_reg_write(dev, 0x1000, 0x00000001);
    s32g3_reg_write(dev, 0x2000, 0x00000001);
    s32g3_reg_write(dev, 0x3000, 0xFFFFFFFF);
    
    dev->state = S32G3_STATE_READY;
    pr_info("s32g3: SoC initialized successfully\n");
    return 0;
}

static void s32g3_hw_exit(struct s32g3_device *dev)
{
    s32g3_reg_write(dev, 0x3000, 0x00000000);
    s32g3_reg_write(dev, 0x2000, 0x00000000);
    s32g3_reg_write(dev, 0x1000, 0x00000000);
    dev->state = S32G3_STATE_STOPPED;
}

static int s32g3_platform_probe(struct platform_device *pdev)
{
    struct s32g3_device *dev;
    struct resource *res;
    int ret;
    
    pr_info("s32g3: Probing device %s\n", dev_name(&pdev->dev));
    
    dev = s32g3_alloc_device();
    if (!dev)
        return -ENOMEM;
    
    dev->dev = &pdev->dev;
    snprintf(dev->name, sizeof(dev->name), "s32g3-%d", s32g3_device_count);
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    dev->reg_phys = res->start;
    dev->reg_size = resource_size(res);
    
    dev->reg_base = ioremap(dev->reg_phys, dev->reg_size);
    if (!dev->reg_base) {
        ret = -ENOMEM;
        goto err_ioremap;
    }
    
    dev->dma_buf_size = PAGE_SIZE * 64;
    dev->dma_buf = dma_alloc_coherent(&pdev->dev, dev->dma_buf_size,
                                       &dev->dma_handle, GFP_KERNEL);
    if (!dev->dma_buf) {
        ret = -ENOMEM;
        goto err_dma;
    }
    
    ret = s32g3_hw_init(dev);
    if (ret)
        goto err_hw;
    
    dev->num_cores = num_online_cpus();
    dev->cores = kcalloc(dev->num_cores, sizeof(*dev->cores), GFP_KERNEL);
    if (!dev->cores) {
        ret = -ENOMEM;
        goto err_alloc;
    }
    
    dev->num_ports = S32G3_MAX_PORTS;
    dev->ports = kcalloc(dev->num_ports, sizeof(*dev->ports), GFP_KERNEL);
    if (!dev->ports) {
        ret = -ENOMEM;
        goto err_alloc;
    }
    
    ret = s32g3_ptp_init();
    if (ret)
        pr_warn("s32g3: PTP init failed\n");
    
    ret = s32g3_sync_init();
    if (ret)
        pr_warn("s32g3: Sync init failed\n");
    
    ret = s32g3_sec_init();
    if (ret)
        pr_warn("s32g3: Sec init failed\n");
    
    ret = s32g3_net_init();
    if (ret)
        pr_warn("s32g3: Net init failed\n");
    
    dev->debugfs_dir = debugfs_create_dir(dev->name, NULL);
    if (dev->debugfs_dir) {
        debugfs_create_u32("state", 0444, dev->debugfs_dir, &dev->state);
        debugfs_create_u32("num_cores", 0444, dev->debugfs_dir, &dev->num_cores);
    }
    
    mutex_lock(&s32g3_mutex);
    s32g3_devices[s32g3_device_count] = dev;
    s32g3_device_count++;
    mutex_unlock(&s32g3_mutex);
    
    dev->state = S32G3_STATE_RUNNING;
    pr_info("s32g3: Device %s probed successfully\n", dev->name);
    return 0;
    
err_alloc:
err_hw:
    dma_free_coherent(&pdev->dev, dev->dma_buf_size, dev->dma_buf,
                      dev->dma_handle);
err_dma:
    iounmap(dev->reg_base);
err_ioremap:
    s32g3_free_device(dev);
    return ret;
}

static int s32g3_platform_remove(struct platform_device *pdev)
{
    struct s32g3_device *dev = NULL;
    int i;
    
    mutex_lock(&s32g3_mutex);
    for (i = 0; i < s32g3_device_count; i++) {
        if (s32g3_devices[i] && s32g3_devices[i]->dev == &pdev->dev) {
            dev = s32g3_devices[i];
            s32g3_devices[i] = NULL;
            s32g3_device_count--;
            break;
        }
    }
    mutex_unlock(&s32g3_mutex);
    
    if (!dev)
        return 0;
    
    dev->state = S32G3_STATE_STOPPED;
    
    s32g3_net_exit();
    s32g3_sec_exit();
    s32g3_sync_exit();
    s32g3_ptp_exit();
    s32g3_hw_exit(dev);
    
    if (dev->debugfs_dir)
        debugfs_remove(dev->debugfs_dir);
    
    if (dev->reg_base)
        iounmap(dev->reg_base);
    
    if (dev->dma_buf)
        dma_free_coherent(&pdev->dev, dev->dma_buf_size,
                         dev->dma_buf, dev->dma_handle);
    
    s32g3_free_device(dev);
    pr_info("s32g3: Device removed\n");
    return 0;
}

static const struct of_device_id s32g3_of_match[] = {
    { .compatible = "nxp,s32g3" },
    { }
};
MODULE_DEVICE_TABLE(of, s32g3_of_match);

static struct platform_driver s32g3_platform_driver = {
    .probe = s32g3_platform_probe,
    .remove = s32g3_platform_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = s32g3_of_match,
    },
};

static int __init s32g3_init(void)
{
    int ret;
    
    pr_info("s32g3: Loading driver version %s\n", DRIVER_VERSION);
    
    mutex_init(&s32g3_mutex);
    
    s32g3_class = class_create(DRIVER_NAME);
    if (IS_ERR(s32g3_class))
        return PTR_ERR(s32g3_class);
    
    ret = platform_driver_register(&s32g3_platform_driver);
    if (ret) {
        class_destroy(s32g3_class);
        return ret;
    }
    
    pr_info("s32g3: Driver loaded successfully\n");
    return 0;
}

static void __exit s32g3_exit(void)
{
    platform_driver_unregister(&s32g3_platform_driver);
    
    if (s32g3_class)
        class_destroy(s32g3_class);
    
    pr_info("s32g3: Driver unloaded\n");
}

module_init(s32g3_init);
module_exit(s32g3_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(S32G3_DRIVER_AUTHOR);
MODULE_DESCRIPTION(S32G3_DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
