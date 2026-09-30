// SPDX-License-Identifier: GPL-2.0
//
// thunderx3_core.c - Marvell ThunderX3 ARM Server Driver Core
//
// MTS-MC-5000 Mobile Core ThunderX3 Driver
//
// Copyright (c) 2024 MTS Router Project
//
// This is the core driver implementation for Marvell ThunderX3 ARM server
// SoC. It handles CPU core management, PCIe enumeration, interrupt handling,
// and system-level monitoring.

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
#include <linux/of_pci.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <asm/irq.h>
#include <uapi/linux/thunderx3.h>

#include "thunderx3.h"
#include "thunderx3_net.h"
#include "thunderx3_pmu.h"
#include "thunderx3_cxl.h"

#define DRIVER_NAME "thunderx3"
#define DRIVER_VERSION "1.0.0"
#define THUNDERX3_DRIVER_AUTHOR "MTS Router Firmware Agent"
#define THUNDERX3_DRIVER_DESC "Marvell ThunderX3 ARM Server Driver"

/* ============================================================================ */
/* Internal constants                                                           */
/* ============================================================================ */

#define THUNDERX3_MAX_DEVICES_GLOBAL 4
#define THUNDERX3_PCI_BAR 0
#define THUNDERX3_PCI_BAR_SIZE SZ_1M
#define THUNDERX3_DEFAULT_POLL_INTERVAL_MS 100
#define THUNDERX3_RESET_TIMEOUT_MS 5000
#define THUNDERX3_MAGIC_VALUE 0x54583330  /* "TX30" */

/* ============================================================================ */
/* Global state                                                                 */
/* ============================================================================ */

static struct thunderx3_device *thunderx3_devices[THUNDERX3_MAX_DEVICES_GLOBAL];
static int thunderx3_device_count;
static struct class *thunderx3_class;
static dev_t thunderx3_dev_no;
static struct mutex thunderx3_mutex;

/* ============================================================================ */
/* PCI device ID table                                                          */
/* ============================================================================ */

static const struct pci_device_id thunderx3_pci_ids[] = {
    { PCI_DEVICE(THUNDERX3_PCI_VENDOR_ID, THUNDERX3_PCI_DEVICE_ID_BASE) },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, thunderx3_pci_ids);

/* ============================================================================ */
/* Internal helper functions                                                    */
/* ============================================================================ */

static inline int thunderx3_reg_read(struct thunderx3_device *dev,
                                      uint32_t offset, uint32_t *value)
{
    if (!dev || !dev->reg_base || !value)
        return -EINVAL;
    
    *value = readl(dev->reg_base + offset);
    return 0;
}

static inline int thunderx3_reg_write(struct thunderx3_device *dev,
                                       uint32_t offset, uint32_t value)
{
    if (!dev || !dev->reg_base)
        return -EINVAL;
    
    writel(value, dev->reg_base + offset);
    return 0;
}

static struct thunderx3_device *thunderx3_alloc_device(void)
{
    struct thunderx3_device *dev;
    
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return NULL;
    
    dev->magic = THUNDERX3_MAGIC_VALUE;
    dev->state = THUNDERX3_STATE_INIT;
    dev->num_cores = 0;
    dev->num_ports = 0;
    dev->num_numa_nodes = 0;
    
    mutex_init(&dev->lock);
    init_timer(&dev->timer);
    dev->timer.data = (unsigned long)dev;
    
    dev->wq = alloc_workqueue("thunderx3_wq", WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
    if (!dev->wq) {
        kfree(dev);
        return NULL;
    }
    
    spin_lock_init(&dev->stat_lock);
    spin_lock_init(&dev->event_lock);
    
    return dev;
}

static void thunderx3_free_device(struct thunderx3_device *dev)
{
    if (!dev)
        return;
    
    if (dev->wq)
        destroy_workqueue(dev->wq);
    
    if (dev->cores)
        kfree(dev->cores);
    
    if (dev->ports)
        kfree(dev->ports);
    
    if (dev->l3_caches)
        kfree(dev->l3_caches);
    
    if (dev->pmcs)
        kfree(dev->pmcs);
    
    if (dev->cxls)
        kfree(dev->cxls);
    
    if (dev->thermals)
        kfree(dev->thermals);
    
    if (dev->powers)
        kfree(dev->powers);
    
    if (dev->netdev)
        unregister_netdev(dev->netdev);
    
    if (dev->reg_base)
        iounmap(dev->reg_base);
    
    if (dev->dma_buf)
        dma_free_coherent(&thunderx3_devices[0]->pci_dev->dev,
                         dev->dma_buf_size,
                         dev->dma_buf, dev->dma_handle);
    
    if (dev->fw_data)
        kfree(dev->fw_data);
    
    if (dev->debugfs_dir)
        debugfs_remove(dev->debugfs_dir);
    
    kfree(dev);
}

static int thunderx3_hw_init(struct thunderx3_device *dev)
{
    int ret;
    uint32_t val;
    
    /* Reset the SoC */
    ret = thunderx3_reg_write(dev, 0x0000, 0x1);  /* SW_RESET */
    if (ret)
        return ret;
    
    /* Wait for reset to complete */
    ret = thunderx3_reg_read_poll_timeout(dev, 0x0000, &val,
                                           !(val & 0x1), 100,
                                           THUNDERX3_RESET_TIMEOUT_MS * 1000);
    if (ret) {
        pr_err("thunderx3: SoC reset timeout\n");
        return -ETIMEDOUT;
    }
    
    /* Configure ACP (ARM Coherency Port) */
    thunderx3_reg_write(dev, 0x1000, 0x00000001);  /* ACP_CTRL */
    
    /* Enable PCIe */
    thunderx3_reg_write(dev, 0x2000, 0x00000001);  /* PCIE_EN */
    
    /* Enable interrupt controller */
    thunderx3_reg_write(dev, 0x3000, 0xFFFFFFFF);  /* IRQ_EN */
    
    /* Configure DDR controller */
    thunderx3_reg_write(dev, 0x4000, 0x00000003);  /* DDR_CTRL */
    
    dev->state = THUNDERX3_STATE_READY;
    
    pr_info("thunderx3: SoC initialized successfully\n");
    return 0;
}

static void thunderx3_hw_exit(struct thunderx3_device *dev)
{
    /* Disable everything */
    thunderx3_reg_write(dev, 0x4000, 0x00000000);  /* DDR_DIS */
    thunderx3_reg_write(dev, 0x3000, 0x00000000);  /* IRQ_DIS */
    thunderx3_reg_write(dev, 0x2000, 0x00000000);  /* PCIE_DIS */
    thunderx3_reg_write(dev, 0x1000, 0x00000000);  /* ACP_DIS */
    
    dev->state = THUNDERX3_STATE_STOPPED;
}

/* ============================================================================ */
/* PCI probe/remove                                                             */
/* ============================================================================ */

static int thunderx3_probe(struct pci_dev *pci_dev,
                           const struct pci_device_id *id)
{
    struct thunderx3_device *dev;
    int ret;
    
    pr_info("thunderx3: Probing device at %04x:%02x:%02x.%x\n",
            pci_dev->vendor, pci_dev->bus->number,
            PCI_SLOT(pci_dev->devfn), PCI_FUNC(pci_dev->devfn));
    
    if (!thunderx3_class) {
        ret = -ENODEV;
        goto err_global;
    }
    
    /* Allocate device structure */
    dev = thunderx3_alloc_device();
    if (!dev) {
        ret = -ENOMEM;
        goto err_alloc;
    }
    
    dev->pci_fn = PCI_FUNC(pci_dev->devfn);
    dev->pci_dev = pci_dev;
    snprintf(dev->name, THUNDERX3_DEV_NAME_LEN, "thunderx3-%d",
             thunderx3_device_count);
    
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
    
    dev->reg_phys = pci_resource_start(pci_dev, THUNDERX3_PCI_BAR);
    dev->reg_size = pci_resource_len(pci_dev, THUNDERX3_PCI_BAR);
    
    dev->reg_base = ioremap(dev->reg_phys, dev->reg_size);
    if (!dev->reg_base) {
        ret = -ENOMEM;
        goto err_ioremap;
    }
    
    /* Allocate DMA coherent buffer */
    dev->dma_buf_size = PAGE_SIZE * 64;
    dev->dma_buf = dma_alloc_coherent(&pci_dev->dev, dev->dma_buf_size,
                                       &dev->dma_handle, GFP_KERNEL);
    if (!dev->dma_buf) {
        ret = -ENOMEM;
        goto err_dma;
    }
    
    /* Initialize hardware */
    ret = thunderx3_hw_init(dev);
    if (ret)
        goto err_hw;
    
    /* Allocate core array */
    dev->num_cores = num_online_cpus();
    dev->cores = kcalloc(dev->num_cores, sizeof(*dev->cores), GFP_KERNEL);
    if (!dev->cores) {
        ret = -ENOMEM;
        goto err_alloc;
    }
    
    /* Allocate port array */
    dev->num_ports = THUNDERX3_MAX_PORTS_PER_DEV;
    dev->ports = kcalloc(dev->num_ports, sizeof(*dev->ports), GFP_KERNEL);
    if (!dev->ports) {
        ret = -ENOMEM;
        goto err_alloc;
    }
    
    /* Initialize PMU */
    ret = thunderx3_pmu_init(dev);
    if (ret)
        pr_warn("thunderx3: PMU init failed\n");
    
    /* Initialize CXL */
    ret = thunderx3_cxl_init(dev);
    if (ret)
        pr_warn("thunderx3: CXL init failed\n");
    
    /* Initialize network */
    ret = thunderx3_net_init(dev);
    if (ret)
        pr_warn("thunderx3: Net init failed\n");
    
    /* Initialize interrupt handling */
    ret = thunderx3_irq_init(dev);
    if (ret)
        pr_warn("thunderx3: IRQ init failed\n");
    
    /* Initialize power management */
    ret = thunderx3_power_init(dev);
    if (ret)
        pr_warn("thunderx3: Power init failed\n");
    
    /* Create debugfs entries */
    dev->debugfs_dir = debugfs_create_dir(dev->name, NULL);
    if (dev->debugfs_dir) {
        debugfs_create_u32("state", 0444, dev->debugfs_dir, &dev->state);
        debugfs_create_u32("num_cores", 0444, dev->debugfs_dir, &dev->num_cores);
        debugfs_create_u32("num_ports", 0444, dev->debugfs_dir, &dev->num_ports);
        debugfs_create_u32("fw_version", 0444, dev->debugfs_dir, &dev->fw_version);
    }
    
    /* Store device */
    mutex_lock(&thunderx3_mutex);
    thunderx3_devices[thunderx3_device_count] = dev;
    thunderx3_device_count++;
    mutex_unlock(&thunderx3_mutex);
    
    dev->state = THUNDERX3_STATE_RUNNING;
    
    pr_info("thunderx3: Device %s probed successfully\n", dev->name);
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
    thunderx3_free_device(dev);
err_alloc:
err_global:
    pr_err("thunderx3: Probe failed with error %d\n", ret);
    return ret;
}

static void thunderx3_remove(struct pci_dev *pci_dev)
{
    struct thunderx3_device *dev = NULL;
    int i;
    
    mutex_lock(&thunderx3_mutex);
    for (i = 0; i < thunderx3_device_count; i++) {
        if (thunderx3_devices[i] &&
            thunderx3_devices[i]->pci_dev == pci_dev) {
            dev = thunderx3_devices[i];
            thunderx3_devices[i] = NULL;
            thunderx3_device_count--;
            break;
        }
    }
    mutex_unlock(&thunderx3_mutex);
    
    if (!dev)
        return;
    
    dev->state = THUNDERX3_STATE_STOPPED;
    
    /* Cleanup */
    thunderx3_power_exit(dev);
    thunderx3_irq_exit(dev);
    thunderx3_net_exit(dev);
    thunderx3_cxl_exit(dev);
    thunderx3_pmu_exit(dev);
    thunderx3_hw_exit(dev);
    
    if (dev->debugfs_dir)
        debugfs_remove(dev->debugfs_dir);
    
    if (dev->reg_base)
        iounmap(dev->reg_base);
    
    if (dev->dma_buf)
        dma_free_coherent(&pci_dev->dev, dev->dma_buf_size,
                         dev->dma_buf, dev->dma_handle);
    
    pci_release_regions(pci_dev);
    pci_disable_device(pci_dev);
    
    thunderx3_free_device(dev);
    
    pr_info("thunderx3: Device removed\n");
}

/* ============================================================================ */
/* Power management                                                             */
/* ============================================================================ */

static int thunderx3_suspend(struct device *dev)
{
    struct pci_dev *pci_dev = to_pci_dev(dev);
    struct thunderx3_device *thunderx3_dev;
    int i;
    
    mutex_lock(&thunderx3_mutex);
    for (i = 0; i < thunderx3_device_count; i++) {
        thunderx3_dev = thunderx3_devices[i];
        if (thunderx3_dev && thunderx3_dev->pci_dev == pci_dev) {
            thunderx3_power_suspend(thunderx3_dev);
            break;
        }
    }
    mutex_unlock(&thunderx3_mutex);
    
    return 0;
}

static int thunderx3_resume(struct device *dev)
{
    struct pci_dev *pci_dev = to_pci_dev(dev);
    struct thunderx3_device *thunderx3_dev;
    int i;
    
    mutex_lock(&thunderx3_mutex);
    for (i = 0; i < thunderx3_device_count; i++) {
        thunderx3_dev = thunderx3_devices[i];
        if (thunderx3_dev && thunderx3_dev->pci_dev == pci_dev) {
            thunderx3_power_resume(thunderx3_dev);
            break;
        }
    }
    mutex_unlock(&thunderx3_mutex);
    
    return 0;
}

static SIMPLE_DEV_PM_OPS(thunderx3_pm_ops, thunderx3_suspend, thunderx3_resume);

/* ============================================================================ */
/* PCI driver structure                                                         */
/* ============================================================================ */

static struct pci_driver thunderx3_pci_driver = {
    .name = DRIVER_NAME,
    .id_table = thunderx3_pci_ids,
    .probe = thunderx3_probe,
    .remove = thunderx3_remove,
    .driver.pm = &thunderx3_pm_ops,
};

/* ============================================================================ */
/* Module init/exit                                                             */
/* ============================================================================ */

static int __init thunderx3_init(void)
{
    int ret;
    
    pr_info("thunderx3: Loading driver version %s\n", DRIVER_VERSION);
    
    mutex_init(&thunderx3_mutex);
    
    /* Create device class */
    thunderx3_class = class_create(DRIVER_NAME);
    if (IS_ERR(thunderx3_class)) {
        ret = PTR_ERR(thunderx3_class);
        pr_err("thunderx3: Failed to create class\n");
        return ret;
    }
    
    /* Register PCI driver */
    ret = pci_register_driver(&thunderx3_pci_driver);
    if (ret) {
        pr_err("thunderx3: Failed to register PCI driver\n");
        class_destroy(thunderx3_class);
        return ret;
    }
    
    pr_info("thunderx3: Driver loaded successfully\n");
    return 0;
}

static void __exit thunderx3_exit(void)
{
    pci_unregister_driver(&thunderx3_pci_driver);
    
    if (thunderx3_class)
        class_destroy(thunderx3_class);
    
    pr_info("thunderx3: Driver unloaded\n");
}

module_init(thunderx3_init);
module_exit(thunderx3_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(THUNDERX3_DRIVER_AUTHOR);
MODULE_DESCRIPTION(THUNDERX3_DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
