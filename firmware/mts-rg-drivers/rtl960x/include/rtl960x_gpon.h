/*
 * rtl960x_gpon.h — GPON PHY driver for Realtek RTL960x
 *
 * MTS-RG-500 Residential Gateway — Драйвер GPON PHY
 * Используется для управления GPON оптическим модулем
 */

#ifndef RTL960X_GPON_H
#define RTL960X_GPON_H

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

#define RTL960X_MODULE_NAME "rtl960x-gpon"
#define RTL960X_I2C_ADDR 0x40
#define RTL960X_MAX_ONU 128
#define RTL960X_MAX_PON_PORTS 1

/* GPON state */
enum rtl960x_gpon_state {
    RTL960X_GPON_STATE_OFF = 0,
    RTL960X_GPON_STATE_INIT,
    RTL960X_GPON_STATE_DISCOVERY,
    RTL960X_GPON_STATE_OPERATING,
    RTL960X_GPON_STATE_ERROR
};

/* ONU state */
enum rtl960x_onu_state {
    RTL960X_ONU_STATE_UNKNOWN = 0,
    RTL960X_ONU_STATE_D0,       /* Dormant */
    RTL960X_ONU_STATE_D1,       /* Registered (serial number) */
    RTL960X_ONU_STATE_D2,       /* Assigned (R_TT) */
    RTL960X_ONU_STATE_D3,       /* Active */
    RTL960X_ONU_STATE_DISABLED
};

/* GPON parameters */
struct rtl960x_gpon_config {
    uint32_t pon_id;
    uint32_t tx_power;       /* dBm */
    uint32_t rx_sensitivity; /* dBm */
    uint32_t wavelength;     /* nm (1490/1550) */
    uint32_t split_ratio;    /* 1:128, 1:256 */
    uint32_t max_distance;   /* km */
    uint32_t omci_version;
    uint32_t auto_reconnect;
    uint32_t reconnect_delay;
};

/* ONU info */
struct rtl960x_onu_info {
    uint32_t index;
    char serial[17];
    char mac[18];
    char vendor[17];
    uint32_t state;
    uint32_t power_level;    /* dBm */
    uint32_t distance;       /* meters (R_TT) */
    uint32_t tx_bias;        /* mA */
    uint32_t tx_bias_nom;    /* mA */
    uint32_t temperature;    /* Celsius * 100 */
    uint32_t vcc;            /* mV */
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t last_seen;
};

/* GPON status */
struct rtl960x_gpon_status {
    uint32_t state;
    uint32_t pon_id;
    uint32_t num_onu;
    uint32_t num_active_onu;
    uint32_t tx_power;       /* dBm */
    uint32_t las_di_status;  /* Laser diagnostics */
    uint32_t temperature;    /* Celsius * 100 */
    uint32_t vcc;            /* mV */
    uint32_t tx_bias;        /* mA */
    uint64_t uptime;         /* seconds */
    uint64_t crc_errors;
    uint64_t frame_errors;
    uint64_t丢包计数;
};

/* IOCTL commands */
#define RTL960X_IOC_MAGIC 'R'
#define RTL960X_IOC_GET_CONFIG    _IOR(RTL960X_IOC_MAGIC, 1, struct rtl960x_gpon_config)
#define RTL960X_IOC_SET_CONFIG    _IOW(RTL960X_IOC_MAGIC, 2, struct rtl960x_gpon_config)
#define RTL960X_IOC_GET_STATUS    _IOR(RTL960X_IOC_MAGIC, 3, struct rtl960x_gpon_status)
#define RTL960X_IOC_GET_ONU       _IOR(RTL960X_IOC_MAGIC, 4, struct rtl960x_onu_info)
#define RTL960X_IOC_SET_ONU_STATE _IOW(RTL960X_IOC_MAGIC, 5, uint32_t)
#define RTL960X_IOC_GET_ONU_COUNT _IOR(RTL960X_IOC_MAGIC, 6, uint32_t)
#define RTL960X_IOC_RESET         _IO(RTL960X_IOC_MAGIC, 7)
#define RTL960X_IOC_REBOOT        _IO(RTL960X_IOC_MAGIC, 8)
#define RTL960X_IOC_READ_REG      _IOR(RTL960X_IOC_MAGIC, 9, uint32_t)
#define RTL960X_IOC_WRITE_REG     _IOW(RTL960X_IOC_MAGIC, 10, uint32_t)

/* Function prototypes */
int rtl960x_gpon_init(void);
void rtl960x_gpon_exit(void);
int rtl960x_gpon_probe(struct i2c_client *client);
int rtl960x_gpon_remove(struct i2c_client *client);
int rtl960x_gpon_get_config(struct rtl960x_gpon_config *cfg);
int rtl960x_gpon_set_config(const struct rtl960x_gpon_config *cfg);
int rtl960x_gpon_get_status(struct rtl960x_gpon_status *status);
int rtl960x_gpon_get_onu(uint32_t index, struct rtl960x_onu_info *onu);
int rtl960x_gpon_set_onu_state(uint32_t index, uint32_t state);
int rtl960x_gpon_get_onu_count(void);
int rtl960x_gpon_reset(void);
int rtl960x_gpon_reboot(void);
int rtl960x_gpon_read_reg(uint32_t reg, uint32_t *value);
int rtl960x_gpon_write_reg(uint32_t reg, uint32_t value);

#endif /* RTL960X_GPON_H */
