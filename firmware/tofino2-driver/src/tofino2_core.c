// SPDX-License-Identifier: GPL-2.0
//
// tofino2_core.c - Intel Tofino 2 ASIC Driver Core
//
// MTS-CR-9000 Core Router Tofino 2 Driver
//
// Copyright (c) 2024 MTS Router Project
//
// This is the core driver implementation for Intel Tofino 2 P4-programmable
// Ethernet switching ASIC. It handles device initialization, PCI enumeration,
// register access, and device lifecycle management.

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/iopoll.h>
#include <linux/of.h>
#include <linux/of_pci.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <asm/irq.h>

#include "tofino2.h"
#include "tofino2_p4.h"
#include "tofino2_phy.h"
#include "tofino2_ctrl.h"
#include "tofino2_telemetry.h"

#define DRIVER_NAME "tofino2"
#define DRIVER_VERSION "1.0.0"
#define TOFINO2_DRIVER_AUTHOR "MTS Router Firmware Agent"
#define TOFINO2_DRIVER_DESC "Intel Tofino 2 P4-programmable Switch Driver"

/* ============================================================================ */
/* Internal constants                                                           */
/* ============================================================================ */

#define TOFINO2_MAX_DEVICES_GLOBAL 8
#define TOFINO2_PCI_BAR 0
#define TOFINO2_PCI_BAR_SIZE SZ_4M
#define TOFINO2_FIRMWARE_DIR "tofino2-fw"
#define TOFINO2_DEFAULT_POLL_INTERVAL_MS 100
#define TOFINO2_RESET_TIMEOUT_MS 5000
#define TOFINO2_PROBE_TIMEOUT_MS 1000
#define TOFINO2_MAGIC_VALUE 0x54464E32  /* "TFN2" */

/* ============================================================================ */
/* Global state                                                                 */
/* ============================================================================ */

static struct tofino2_device *tofino2_devices[TOFINO2_MAX_DEVICES_GLOBAL];
static int tofino2_device_count;
static struct class *tofino2_class;
static dev_t tofino2_dev_no;
static struct mutex tofino2_mutex;

/* ============================================================================ */
/* PCI device ID table                                                          */
/* ============================================================================ */

static const struct pci_device_id tofino2_pci_ids[] = {
    { PCI_DEVICE(TOFINO2_PCI_VENDOR_ID, TOFINO2_PCI_DEVICE_ID) },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, tofino2_pci_ids);

/* ============================================================================ */
/* Internal helper functions                                                    */
/* ============================================================================ */

static inline int tofino2_reg_read(struct tofino2_device *dev,
                                    uint32_t offset, uint32_t *value)
{
    if (!dev || !dev->reg_base || !value)
        return -EINVAL;
    
    *value = readl(dev->reg_base + offset);
    return 0;
}

static inline int tofino2_reg_write(struct tofino2_device *dev,
                                     uint32_t offset, uint32_t value)
{
    if (!dev || !dev->reg_base)
        return -EINVAL;
    
    writel(value, dev->reg_base + offset);
    return 0;
}

static inline int tofino2_reg_read_poll_timeout(struct tofino2_device *dev,
                                                 uint32_t offset,
                                                 uint32_t *value,
                                                 int cond,
                                                 int poll_us,
                                                 int timeout_us)
{
    return readl_poll_timeout(dev->reg_base + offset, *value, cond,
                              poll_us, timeout_us);
}

static struct tofino2_device *tofino2_alloc_device(void)
{
    struct tofino2_device *dev;
    
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return NULL;
    
    dev->magic = TOFINO2_MAGIC_VALUE;
    dev->state = TOFINO2_STATE_INIT;
    dev->num_ports = 0;
    dev->num_fabric_ports = 0;
    dev->num_pipes = 0;
    dev->num_tables = 0;
    dev->num_actions = 0;
    dev->num_externs = 0;
    
    mutex_init(&dev->lock);
    init_timer(&dev->timer);
    dev->timer.data = (unsigned long)dev;
    
    dev->wq = alloc_workqueue("tofino2_wq", WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
    if (!dev->wq) {
        kfree(dev);
        return NULL;
    }
    
    spin_lock_init(&dev->stat_lock);
    spin_lock_init(&dev->event_lock);
    
    return dev;
}

static void tofino2_free_device(struct tofino2_device *dev)
{
    if (!dev)
        return;
    
    if (dev->wq)
        destroy_workqueue(dev->wq);
    
    if (dev->ports)
        kfree(dev->ports);
    
    if (dev->fabric)
        kfree(dev->fabric);
    
    if (dev->pipelines)
        kfree(dev->pipelines);
    
    if (dev->tables)
        kfree(dev->tables);
    
    if (dev->entries)
        kfree(dev->entries);
    
    if (dev->counters)
        kfree(dev->counters);
    
    if (dev->meters)
        kfree(dev->meters);
    
    if (dev->dma)
        kfree(dev->dma);
    
    if (dev->netdev)
        unregister_netdev(dev->netdev);
    
    if (dev->reg_base)
        iounmap(dev->reg_base);
    
    if (dev->dma_buf)
        dma_free_coherent(&tofino2_devices[0]->pci_dev->dev,
                         dev->dma_buf_size,
                         dev->dma_buf, dev->dma_handle);
    
    if (dev->fw_data)
        kfree(dev->fw_data);
    
    if (dev->debugfs_dir)
        debugfs_remove(dev->debugfs_dir);
    
    kfree(dev);
}

static int tofino2_hw_init(struct tofino2_device *dev)
{
    int ret;
    uint32_t val;
    
    /* Reset the ASIC */
    ret = tofino2_reg_write(dev, 0x0000, 0x1);  /* SW_RESET */
    if (ret)
        return ret;
    
    /* Wait for reset to complete */
    ret = tofino2_reg_read_poll_timeout(dev, 0x0000, &val,
                                         !(val & 0x1), 100,
                                         TOFINO2_RESET_TIMEOUT_MS * 1000);
    if (ret) {
        pr_err("tofino2: ASIC reset timeout\n");
        return -ETIMEDOUT;
    }
    
    /* Configure PLL */
    tofino2_reg_write(dev, 0x1000, 0x00000001);  /* PLL_CTRL */
    tofino2_reg_write(dev, 0x1004, 0x0000000F);  /* PLL_CFG */
    
    /* Enable clocks */
    tofino2_reg_write(dev, 0x2000, 0xFFFFFFFF);  /* CLK_EN */
    
    /* Configure PCIe */
    tofino2_reg_write(dev, 0x3000, 0x00000003);  /* PCIe_CTRL */
    
    /* Initialize DMA */
    tofino2_reg_write(dev, 0x4000, 0x00000001);  /* DMA_RST */
    tofino2_reg_write(dev, 0x4004, 0x00000001);  /* DMA_EN */
    
    /* Enable interrupts */
    tofino2_reg_write(dev, 0x5000, 0xFFFFFFFF);  /* IRQ_EN */
    
    /* Configure switch fabric */
    tofino2_reg_write(dev, 0x6000, 0x00000001);  /* FABRIC_EN */
    
    dev->state = TOFINO2_STATE_READY;
    
    pr_info("tofino2: ASIC initialized successfully\n");
    return 0;
}

static void tofino2_hw_exit(struct tofino2_device *dev)
{
    /* Disable everything */
    tofino2_reg_write(dev, 0x6000, 0x00000000);  /* FABRIC_DIS */
    tofino2_reg_write(dev, 0x5000, 0x00000000);  /* IRQ_DIS */
    tofino2_reg_write(dev, 0x4004, 0x00000000);  /* DMA_DIS */
    tofino2_reg_write(dev, 0x2000, 0x00000000);  /* CLK_DIS */
    tofino2_reg_write(dev, 0x1000, 0x00000000);  /* PLL_DIS */
    
    dev->state = TOFINO2_STATE_STOPPED;
}

/* ============================================================================ */
/* PCI probe/remove                                                             */
/* ============================================================================ */

static int tofino2_probe(struct pci_dev *pci_dev,
                         const struct pci_device_id *id)
{
    struct tofino2_device *dev;
    int ret;
    
    pr_info("tofino2: Probing device at %04x:%02x:%02x.%x\n",
            pci_dev->vendor, pci_dev->bus->number,
            PCI_SLOT(pci_dev->devfn), PCI_FUNC(pci_dev->devfn));
    
    /* Initialize global state */
    if (!tofino2_class) {
        ret = -ENODEV;
        goto err_global;
    }
    
    /* Allocate device structure */
    dev = tofino2_alloc_device();
    if (!dev) {
        ret = -ENOMEM;
        goto err_alloc;
    }
    
    dev->pci_fn = PCI_FUNC(pci_dev->devfn);
    dev->pci_dev = pci_dev;
    snprintf(dev->name, TOFINO2_DEV_NAME_LEN, "tofino2-%d",
             tofino2_device_count);
    
    /* PCI subsystem setup */
    ret = pci_enable_device(pci_dev);
    if (ret)
        goto err_pci_enable;
    
    ret = pci_set_dma_mask(pci_dev, DMA_BIT_MASK(64));
    if (ret) {
        ret = pci_set_dma_mask(pci_dev, DMA_BIT_MASK(32));
        if (ret)
            goto err_pci_enable;
    }
    
    pci_set_master(pci_dev);
    
    /* Map BAR0 */
    ret = pci_request_regions(pci_dev, DRIVER_NAME);
    if (ret)
        goto err_regions;
    
    dev->reg_phys = pci_resource_start(pci_dev, TOFINO2_PCI_BAR);
    dev->reg_size = pci_resource_len(pci_dev, TOFINO2_PCI_BAR);
    
    dev->reg_base = ioremap(dev->reg_phys, dev->reg_size);
    if (!dev->reg_base) {
        ret = -ENOMEM;
        goto err_ioremap;
    }
    
    /* Allocate DMA coherent buffer */
    dev->dma_buf_size = TOFINO2_DMA_BUF_SIZE;
    dev->dma_buf = dma_alloc_coherent(&pci_dev->dev, dev->dma_buf_size,
                                       &dev->dma_handle, GFP_KERNEL);
    if (!dev->dma_buf) {
        ret = -ENOMEM;
        goto err_dma;
    }
    
    /* Initialize hardware */
    ret = tofino2_hw_init(dev);
    if (ret)
        goto err_hw;
    
    /* Allocate port array */
    dev->num_ports = TOFINO2_MAX_PORTS_PER_DEV;
    dev->ports = kcalloc(dev->num_ports, sizeof(*dev->ports), GFP_KERNEL);
    if (!dev->ports) {
        ret = -ENOMEM;
        goto err_alloc;
    }
    
    /* Allocate fabric port array */
    dev->num_fabric_ports = TOFINO2_MAX_FABRIC_PORTS;
    dev->fabric = kcalloc(dev->num_fabric_ports, sizeof(*dev->fabric),
                          GFP_KERNEL);
    if (!dev->fabric) {
        ret = -ENOMEM;
        goto err_alloc;
    }
    
    /* Initialize telemetry */
    ret = tofino2_telem_init();
    if (ret)
        pr_warn("tofino2: Telemetry init failed\n");
    
    /* Initialize interrupt handling */
    ret = tofino2_irq_init(dev);
    if (ret)
        pr_warn("tofino2: IRQ init failed, continuing without interrupts\n");
    
    /* Initialize power management */
    ret = tofino2_power_init(dev);
    if (ret)
        pr_warn("tofino2: Power init failed\n");
    
    /* Create debugfs entries */
    dev->debugfs_dir = debugfs_create_dir(dev->name, NULL);
    if (dev->debugfs_dir) {
        debugfs_create_u32("state", 0444, dev->debugfs_dir, &dev->state);
        debugfs_create_u32("num_ports", 0444, dev->debugfs_dir,
                          &dev->num_ports);
        debugfs_create_u32("num_tables", 0444, dev->debugfs_dir,
                          &dev->num_tables);
        debugfs_create_u32("fw_version", 0444, dev->debugfs_dir,
                          &dev->fw_version);
    }
    
    /* Store device */
    mutex_lock(&tofino2_mutex);
    tofino2_devices[tofino2_device_count] = dev;
    tofino2_device_count++;
    mutex_unlock(&tofino2_mutex);
    
    dev->state = TOFINO2_STATE_RUNNING;
    
    pr_info("tofino2: Device %s probed successfully\n", dev->name);
    return 0;
    
err_hw:
    dma_free_coherent(&pci_dev->dev, dev->dma_buf_size, dev->dma_buf,
                      dev->dma_handle);
err_dma:
    iounmap(dev->reg_base);
err_ioremap:
    pci_release_regions(pci_dev);
err_regions:
    pci_disable_device(pci_dev);
err_pci_enable:
    tofino2_free_device(dev);
err_alloc:
err_global:
    pr_err("tofino2: Probe failed with error %d\n", ret);
    return ret;
}

static void tofino2_remove(struct pci_dev *pci_dev)
{
    struct tofino2_device *dev = NULL;
    int i;
    
    /* Find our device */
    mutex_lock(&tofino2_mutex);
    for (i = 0; i < tofino2_device_count; i++) {
        if (tofino2_devices[i] &&
            tofino2_devices[i]->pci_dev == pci_dev) {
            dev = tofino2_devices[i];
            tofino2_devices[i] = NULL;
            tofino2_device_count--;
            break;
        }
    }
    mutex_unlock(&tofino2_mutex);
    
    if (!dev)
        return;
    
    dev->state = TOFINO2_STATE_STOPPED;
    
    /* Cleanup */
    tofino2_irq_exit(dev);
    tofino2_power_exit(dev);
    tofino2_telem_exit(dev);
    tofino2_hw_exit(dev);
    
    if (dev->debugfs_dir)
        debugfs_remove(dev->debugfs_dir);
    
    if (dev->reg_base)
        iounmap(dev->reg_base);
    
    if (dev->dma_buf)
        dma_free_coherent(&pci_dev->dev, dev->dma_buf_size,
                         dev->dma_buf, dev->dma_handle);
    
    pci_release_regions(pci_dev);
    pci_disable_device(pci_dev);
    
    tofino2_free_device(dev);
    
    pr_info("tofino2: Device removed\n");
}

/* ============================================================================ */
/* Power management                                                             */
/* ============================================================================ */

static int tofino2_suspend(struct device *dev)
{
    struct pci_dev *pci_dev = to_pci_dev(dev);
    struct tofino2_device *tofino2_dev;
    int i;
    
    mutex_lock(&tofino2_mutex);
    for (i = 0; i < tofino2_device_count; i++) {
        tofino2_dev = tofino2_devices[i];
        if (tofino2_dev && tofino2_dev->pci_dev == pci_dev) {
            tofino2_power_suspend(tofino2_dev);
            break;
        }
    }
    mutex_unlock(&tofino2_mutex);
    
    return 0;
}

static int tofino2_resume(struct device *dev)
{
    struct pci_dev *pci_dev = to_pci_dev(dev);
    struct tofino2_device *tofino2_dev;
    int i;
    
    mutex_lock(&tofino2_mutex);
    for (i = 0; i < tofino2_device_count; i++) {
        tofino2_dev = tofino2_devices[i];
        if (tofino2_dev && tofino2_dev->pci_dev == pci_dev) {
            tofino2_power_resume(tofino2_dev);
            break;
        }
    }
    mutex_unlock(&tofino2_mutex);
    
    return 0;
}

static SIMPLE_DEV_PM_OPS(tofino2_pm_ops, tofino2_suspend, tofino2_resume);

/* ============================================================================ */
/* PCI driver structure                                                         */
/* ============================================================================ */

static struct pci_driver tofino2_pci_driver = {
    .name = DRIVER_NAME,
    .id_table = tofino2_pci_ids,
    .probe = tofino2_probe,
    .remove = tofino2_remove,
    .driver.pm = &tofino2_pm_ops,
};

/* ============================================================================ */
/* Module init/exit                                                             */
/* ============================================================================ */

static int __init tofino2_init(void)
{
    int ret;
    
    pr_info("tofino2: Loading driver version %s\n", DRIVER_VERSION);
    
    mutex_init(&tofino2_mutex);
    
    /* Create device class */
    tofino2_class = class_create(DRIVER_NAME);
    if (IS_ERR(tofino2_class)) {
        ret = PTR_ERR(tofino2_class);
        pr_err("tofino2: Failed to create class\n");
        return ret;
    }
    
    /* Register PCI driver */
    ret = pci_register_driver(&tofino2_pci_driver);
    if (ret) {
        pr_err("tofino2: Failed to register PCI driver\n");
        class_destroy(tofino2_class);
        return ret;
    }
    
    pr_info("tofino2: Driver loaded successfully\n");
    return 0;
}

static void __exit tofino2_exit(void)
{
    pci_unregister_driver(&tofino2_pci_driver);
    
    if (tofino2_class)
        class_destroy(tofino2_class);
    
    pr_info("tofino2: Driver unloaded\n");
}

module_init(tofino2_init);
module_exit(tofino2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(TOFINO2_DRIVER_AUTHOR);
MODULE_DESCRIPTION(TOFINO2_DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
