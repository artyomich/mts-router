/*
 * rtl960x_wdm.c — WDM (Wavelength Division Multiplexing) monitoring for RTL960x
 *
 * MTS-OLT-2000 GPON OLT — Управление WDM компонентами
 * Поддерживает: мониторинг лазеров, фотодиодов, термоэлементов
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/slab.h>

#include "rtl960x_wdm.h"

#define WDM_DRIVER_NAME "rtl960x-wdm"
#define WDM_VERSION "1.0.0"

/* ==================== WDM Component Management ==================== */

int wdm_init(struct wdm_device *wdev)
{
    if (!wdev)
        return -EINVAL;

    memset(wdev->components, 0, sizeof(wdev->components));
    wdev->num_components = 0;
    mutex_init(&wdev->lock);
    return 0;
}
EXPORT_SYMBOL(wdm_init);

void wdm_exit(struct wdm_device *wdev)
{
    if (!wdev)
        return;

    mutex_lock(&wdev->lock);
    memset(wdev->components, 0, sizeof(wdev->components));
    wdev->num_components = 0;
    mutex_unlock(&wdev->lock);
}
EXPORT_SYMBOL(wdm_exit);

int wdm_probe(struct device *dev)
{
    struct wdm_device *wdev;

    wdev = devm_kzalloc(dev, sizeof(*wdev), GFP_KERNEL);
    if (!wdev)
        return -ENOMEM;

    wdev->dev = dev;
    return wdm_init(wdev);
}
EXPORT_SYMBOL(wdm_probe);

int wdm_remove(struct device *dev)
{
    wdm_exit(dev_get_drvdata(dev));
    return 0;
}
EXPORT_SYMBOL(wdm_remove);

int wdm_get_component(struct wdm_device *wdev, uint32_t comp_id, struct wdm_component *comp)
{
    int i;

    if (!wdev || !comp)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    for (i = 0; i < wdev->num_components; i++) {
        if (wdev->components[i].id == comp_id) {
            *comp = wdev->components[i];
            mutex_unlock(&wdev->lock);
            return 0;
        }
    }
    mutex_unlock(&wdev->lock);
    return -ENOENT;
}
EXPORT_SYMBOL(wdm_get_component);

int wdm_set_component(struct wdm_device *wdev, struct wdm_component *comp)
{
    int i;

    if (!wdev || !comp)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    for (i = 0; i < wdev->num_components; i++) {
        if (wdev->components[i].id == comp->id) {
            wdev->components[i] = *comp;
            mutex_unlock(&wdev->lock);
            return 0;
        }
    }

    /* Add new component if slot available */
    if (wdev->num_components < RTL960X_WDM_MAX_COMPONENTS) {
        wdev->components[wdev->num_components++] = *comp;
        mutex_unlock(&wdev->lock);
        return 0;
    }

    mutex_unlock(&wdev->lock);
    return -ENOSPC;
}
EXPORT_SYMBOL(wdm_set_component);

int wdm_get_all_components(struct wdm_device *wdev, struct wdm_component *comps, uint32_t max)
{
    int count = 0;

    if (!wdev || !comps)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    for (int i = 0; i < wdev->num_components && count < max; i++) {
        comps[count++] = wdev->components[i];
    }
    mutex_unlock(&wdev->lock);
    return count;
}
EXPORT_SYMBOL(wdm_get_all_components);

int wdm_set_tx_power(struct wdm_device *wdev, uint32_t power)
{
    if (!wdev)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    for (int i = 0; i < wdev->num_components; i++) {
        if (wdev->components[i].type == WDM_TX_LASER) {
            wdev->components[i].tx_power_set = power;
        }
    }
    mutex_unlock(&wdev->lock);
    return 0;
}
EXPORT_SYMBOL(wdm_set_tx_power);

int wdm_get_temperature(struct wdm_device *wdev, uint32_t *temp)
{
    int found = 0;

    if (!wdev || !temp)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    for (int i = 0; i < wdev->num_components; i++) {
        if (wdev->components[i].type == WDM_TEA) {
            *temp = wdev->components[i].temperature;
            found = 1;
            break;
        }
    }
    mutex_unlock(&wdev->lock);
    return found ? 0 : -ENODATA;
}
EXPORT_SYMBOL(wdm_get_temperature);

int wdm_get_bias_current(struct wdm_device *wdev, uint32_t *bias)
{
    int found = 0;

    if (!wdev || !bias)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    for (int i = 0; i < wdev->num_components; i++) {
        if (wdev->components[i].type == WDM_BIAS) {
            *bias = wdev->components[i].bias_current;
            found = 1;
            break;
        }
    }
    mutex_unlock(&wdev->lock);
    return found ? 0 : -ENODATA;
}
EXPORT_SYMBOL(wdm_get_bias_current);

int wdm_enable(struct wdm_device *wdev)
{
    if (!wdev)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    for (int i = 0; i < wdev->num_components; i++) {
        wdev->components[i].status = 1; /* ENABLED */
    }
    mutex_unlock(&wdev->lock);
    return 0;
}
EXPORT_SYMBOL(wdm_enable);

int wdm_disable(struct wdm_device *wdev)
{
    if (!wdev)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    for (int i = 0; i < wdev->num_components; i++) {
        wdev->components[i].status = 0; /* DISABLED */
    }
    mutex_unlock(&wdev->lock);
    return 0;
}
EXPORT_SYMBOL(wdm_disable);

int wdm_get_fault_status(struct wdm_device *wdev, uint32_t *fault)
{
    int has_fault = 0;

    if (!wdev || !fault)
        return -EINVAL;

    mutex_lock(&wdev->lock);
    *fault = 0;
    for (int i = 0; i < wdev->num_components; i++) {
        if (wdev->components[i].tx_fault) {
            *fault |= (1 << i);
            has_fault = 1;
        }
    }
    mutex_unlock(&wdev->lock);
    return has_fault ? -EIO : 0;
}
EXPORT_SYMBOL(wdm_get_fault_status);

/* ==================== Module ==================== */

static int __init wdm_init_module(void)
{
    pr_info("RTL960x WDM driver loaded (version %s)\n", WDM_VERSION);
    return 0;
}

static void __exit wdm_exit_module(void)
{
    pr_info("RTL960x WDM driver unloaded\n");
}

module_init(wdm_init_module);
module_exit(wdm_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Team");
MODULE_DESCRIPTION("RTL960x WDM Driver");
MODULE_VERSION(WDM_VERSION);
