/*
 * rtl960x_wdm.h — Драйвер WDM (Wavelength Division Multiplexing) для RTL960x
 *
 * MTS-OLT-2000 GPON OLT — Управление WDM компонентами
 */

#ifndef RTL960X_WDM_H
#define RTL960X_WDM_H

#include <linux/types.h>

#define RTL960X_WDM_MAX_COMPONENTS 16
#define RTL960X_WDM_MAX_CHANNELS 4

/* WDM component type */
enum wdm_component_type {
    WDM_TX_LASER,
    WDM_RX_LASER,
    WDM_TX_PHOTO,
    WDM_RX_PHOTO,
    WDM_TEA,
    WDM_BIAS
};

/* WDM component */
struct wdm_component {
    uint32_t id;
    enum wdm_component_type type;
    uint32_t wavelength;    /* nm */
    uint32_t power;         /* dBm * 100 */
    uint32_t status;        /* ENABLED/DISABLED/FAULT */
    uint32_t temperature;   /* Celsius * 100 */
    uint32_t bias_current;  /* mA * 100 */
    uint32_t tx_fault;
    uint32_t rx_loss;
    uint32_t tx_power_set;  /* target TX power * 100 */
    uint32_t agc_range_min;
    uint32_t agc_range_max;
};

/* WDM device */
struct wdm_device {
    uint32_t id;
    char name[32];
    struct wdm_component components[RTL960X_WDM_MAX_COMPONENTS];
    uint32_t num_components;
    void *priv;
    struct device *dev;
    struct mutex lock;
    uint32_t num_channels;
    uint32_t channel_spacing; /* GHz */
};

/* WDM IOCTL commands */
#define WDM_IOC_MAGIC 'W'
#define WDM_IOC_GET_COMP      _IOR(WDM_IOC_MAGIC, 1, struct wdm_component)
#define WDM_IOC_SET_COMP      _IOW(WDM_IOC_MAGIC, 2, struct wdm_component)
#define WDM_IOC_GET_COMPS     _IOR(WDM_IOC_MAGIC, 3, struct wdm_component[])
#define WDM_IOC_SET_POWER     _IOW(WDM_IOC_MAGIC, 4, uint32_t)
#define WDM_IOC_GET_TEMP      _IOR(WDM_IOC_MAGIC, 5, uint32_t)
#define WDM_IOC_GET_BIAS      _IOR(WDM_IOC_MAGIC, 6, uint32_t)
#define WDM_IOC_ENABLE        _IO(WDM_IOC_MAGIC, 7)
#define WDM_IOC_DISABLE       _IO(WDM_IOC_MAGIC, 8)
#define WDM_IOC_GET_FAULT     _IOR(WDM_IOC_MAGIC, 9, uint32_t)

/* Function prototypes */
int wdm_init(struct wdm_device *wdev);
void wdm_exit(struct wdm_device *wdev);
int wdm_probe(struct device *dev);
int wdm_remove(struct device *dev);
int wdm_get_component(struct wdm_device *wdev, uint32_t comp_id, struct wdm_component *comp);
int wdm_set_component(struct wdm_device *wdev, struct wdm_component *comp);
int wdm_get_all_components(struct wdm_device *wdev, struct wdm_component *comps, uint32_t max);
int wdm_set_tx_power(struct wdm_device *wdev, uint32_t power);
int wdm_get_temperature(struct wdm_device *wdev, uint32_t *temp);
int wdm_get_bias_current(struct wdm_device *wdev, uint32_t *bias);
int wdm_enable(struct wdm_device *wdev);
int wdm_disable(struct wdm_device *wdev);
int wdm_get_fault_status(struct wdm_device *wdev, uint32_t *fault);

#endif /* RTL960X_WDM_H */
