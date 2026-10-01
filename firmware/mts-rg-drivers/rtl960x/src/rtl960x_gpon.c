/*
 * rtl960x_gpon.c — GPON PHY driver for Realtek RTL960x
 *
 * MTS-RG-500 Residential Gateway — Драйвер GPON PHY
 * Управление GPON оптическим модулем через I2C/SMBus
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/of.h>

#include "rtl960x_gpon.h"

#define DRIVER_NAME "rtl960x-gpon"
#define DRIVER_VERSION "1.0.0"

/* RTL960x register map */
#define RTL960X_REG_SYS_CTRL    0x00
#define RTL960X_REG_SYS_STATUS  0x01
#define RTL960X_REG_GPON_CTRL   0x10
#define RTL960X_REG_GPON_STATUS 0x11
#define RTL960X_REG_TX_POWER    0x20
#define RTL960X_REG_RX_SENS     0x21
#define RTL960X_REG_TEMPERATURE 0x30
#define RTL960X_REG_VCC         0x31
#define RTL960X_REG_TX_BIAS     0x32
#define RTL960X_REG_ONU_TABLE   0x40
#define RTL960X_REG_OMCI_CTRL   0x50
#define RTL960X_REG_OMCI_STATUS 0x51

/* Device state */
static struct rtl960x_gpon_status gpon_status;
static struct mutex rtl960x_lock;
static struct i2c_client *rtl960x_i2c_client;

/* ==================== I2C Register Access ==================== */

static int rtl960x_i2c_read(struct i2c_client *client, uint8_t reg, uint8_t *value)
{
    int ret;

    if (!client || !value)
        return -EINVAL;

    ret = i2c_smbus_read_byte_data(client, reg);
    if (ret < 0)
        return ret;

    *value = (uint8_t)ret;
    return 0;
}

static int rtl960x_i2c_write(struct i2c_client *client, uint8_t reg, uint8_t value)
{
    if (!client)
        return -EINVAL;

    return i2c_smbus_write_byte_data(client, reg, value);
}

static int rtl960x_i2c_read_word(struct i2c_client *client, uint8_t reg, uint16_t *value)
{
    int ret;

    if (!client || !value)
        return -EINVAL;

    ret = i2c_smbus_read_word_data(client, reg);
    if (ret < 0)
        return ret;

    *value = (uint16_t)ret;
    return 0;
}

/* ==================== GPON Initialization ==================== */

int rtl960x_gpon_init(void)
{
    mutex_init(&rtl960x_lock);
    memset(&gpon_status, 0, sizeof(gpon_status));
    gpon_status.state = RTL960X_GPON_STATE_INIT;

    pr_info("RTL960x GPON driver initialized (version %s)\n", DRIVER_VERSION);
    return 0;
}

void rtl960x_gpon_exit(void)
{
    mutex_lock(&rtl960x_lock);
    gpon_status.state = RTL960X_GPON_STATE_OFF;
    mutex_unlock(&rtl960x_lock);

    pr_info("RTL960x GPON driver exited\n");
}

/* ==================== GPON Probe/Remove ==================== */

int rtl960x_gpon_probe(struct i2c_client *client)
{
    int ret;
    uint8_t reg_val;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        pr_err("RTL960x: I2C SMBus byte data not supported\n");
        return -EIO;
    }

    /* Check device presence */
    ret = rtl960x_i2c_read(client, RTL960X_REG_SYS_STATUS, &reg_val);
    if (ret) {
        pr_err("RTL960x: I2C read failed (reg 0x%02x)\n", RTL960X_REG_SYS_STATUS);
        return ret;
    }

    rtl960x_i2c_client = client;

    /* Reset GPON controller */
    rtl960x_i2c_write(client, RTL960X_REG_SYS_CTRL, 0x01);
    msleep(100);

    /* Initialize status */
    memset(&gpon_status, 0, sizeof(gpon_status));
    gpon_status.state = RTL960X_GPON_STATE_INIT;
    gpon_status.pon_id = 0;

    pr_info("RTL960x GPON probe successful\n");
    return 0;
}

int rtl960x_gpon_remove(struct i2c_client *client)
{
    rtl960x_gpon_exit();
    rtl960x_i2c_client = NULL;
    return 0;
}

/* ==================== GPON Configuration ==================== */

int rtl960x_gpon_get_config(struct rtl960x_gpon_config *cfg)
{
    uint8_t reg_val;

    if (!cfg)
        return -EINVAL;

    mutex_lock(&rtl960x_lock);

    /* Read current configuration from device */
    rtl960x_i2c_read(rtl960x_i2c_client, RTL960X_REG_TX_POWER, &reg_val);
    cfg->tx_power = reg_val;

    rtl960x_i2c_read(rtl960x_i2c_client, RTL960X_REG_RX_SENS, &reg_val);
    cfg->rx_sensitivity = reg_val;

    /* Default values */
    cfg->pon_id = 0;
    cfg->wavelength = 1490;
    cfg->split_ratio = 128;
    cfg->max_distance = 20;
    cfg->omci_version = 1;
    cfg->auto_reconnect = 1;
    cfg->reconnect_delay = 30;

    mutex_unlock(&rtl960x_lock);
    return 0;
}

int rtl960x_gpon_set_config(const struct rtl960x_gpon_config *cfg)
{
    if (!cfg)
        return -EINVAL;

    mutex_lock(&rtl960x_lock);

    /* Write configuration to device */
    rtl960x_i2c_write(rtl960x_i2c_client, RTL960X_REG_TX_POWER, cfg->tx_power);
    rtl960x_i2c_write(rtl960x_i2c_client, RTL960X_REG_RX_SENS, cfg->rx_sensitivity);

    mutex_unlock(&rtl960x_lock);
    return 0;
}

/* ==================== GPON Status ==================== */

int rtl960x_gpon_get_status(struct rtl960x_gpon_status *status)
{
    uint8_t reg_val;
    uint16_t word_val;

    if (!status)
        return -EINVAL;

    mutex_lock(&rtl960x_lock);

    /* Read status from device */
    rtl960x_i2c_read(rtl960x_i2c_client, RTL960X_REG_GPON_STATUS, &reg_val);
    status->state = reg_val;

    rtl960x_i2c_read(rtl960x_i2c_client, RTL960X_REG_TEMPERATURE, &reg_val);
    status->temperature = reg_val * 100;

    rtl960x_i2c_read_word(rtl960x_i2c_client, RTL960X_REG_VCC, &word_val);
    status->vcc = word_val;

    rtl960x_i2c_read(rtl960x_i2c_client, RTL960X_REG_TX_BIAS, &reg_val);
    status->tx_bias = reg_val;

    /* Copy static status */
    status->pon_id = gpon_status.pon_id;
    status->num_onu = gpon_status.num_onu;
    status->num_active_onu = gpon_status.num_active_onu;
    status->las_di_status = gpon_status.las_di_status;
    status->uptime = gpon_status.uptime;
    status->crc_errors = gpon_status.crc_errors;
    status->frame_errors = gpon_status.frame_errors;

    mutex_unlock(&rtl960x_lock);
    return 0;
}

/* ==================== ONU Management ==================== */

int rtl960x_gpon_get_onu(uint32_t index, struct rtl960x_onu_info *onu)
{
    if (!onu || index >= RTL960X_MAX_ONU)
        return -EINVAL;

    mutex_lock(&rtl960x_lock);

    /* Read ONU data from device table */
    /* TODO: Implement actual I2C read for ONU table */
    pr_info("RTL960x: Getting ONU %u\n", index);

    mutex_unlock(&rtl960x_lock);
    return 0;
}

int rtl960x_gpon_set_onu_state(uint32_t index, uint32_t state)
{
    if (index >= RTL960X_MAX_ONU)
        return -EINVAL;

    mutex_lock(&rtl960x_lock);

    /* Set ONU state via I2C */
    /* TODO: Implement actual I2C write */
    pr_info("RTL960x: Setting ONU %u state %u\n", index, state);

    mutex_unlock(&rtl960x_lock);
    return 0;
}

int rtl960x_gpon_get_onu_count(void)
{
    mutex_lock(&rtl960x_lock);
    uint32_t count = gpon_status.num_onu;
    mutex_unlock(&rtl960x_lock);
    return count;
}

/* ==================== GPON Control ==================== */

int rtl960x_gpon_reset(void)
{
    mutex_lock(&rtl960x_lock);

    /* Send reset command to device */
    rtl960x_i2c_write(rtl960x_i2c_client, RTL960X_REG_SYS_CTRL, 0x02);
    msleep(500);

    mutex_unlock(&rtl960x_lock);
    return 0;
}

int rtl960x_gpon_reboot(void)
{
    mutex_lock(&rtl960x_lock);

    /* Reboot GPON PHY */
    rtl960x_i2c_write(rtl960x_i2c_client, RTL960X_REG_SYS_CTRL, 0x03);

    mutex_unlock(&rtl960x_lock);
    return 0;
}

int rtl960x_gpon_read_reg(uint32_t reg, uint32_t *value)
{
    uint8_t val;
    int ret;

    if (!value || reg > 0xFF)
        return -EINVAL;

    ret = rtl960x_i2c_read(rtl960x_i2c_client, (uint8_t)reg, &val);
    if (ret)
        return ret;

    *value = val;
    return 0;
}

int rtl960x_gpon_write_reg(uint32_t reg, uint32_t value)
{
    if (reg > 0xFF)
        return -EINVAL;

    return rtl960x_i2c_write(rtl960x_i2c_client, (uint8_t)reg, (uint8_t)value);
}

/* ==================== I2C Driver ==================== */

static const struct i2c_device_id rtl960x_id[] = {
    { "rtl960x-gpon", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, rtl960x_id);

static struct i2c_driver rtl960x_gpon_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
    },
    .probe = rtl960x_gpon_probe,
    .remove = rtl960x_gpon_remove,
    .id_table = rtl960x_id,
};

/* ==================== Module ==================== */

static int __init rtl960x_gpon_init_module(void)
{
    int ret;

    ret = rtl960x_gpon_init();
    if (ret)
        return ret;

    return i2c_add_driver(&rtl960x_gpon_driver);
}

static void __exit rtl960x_gpon_exit_module(void)
{
    i2c_del_driver(&rtl960x_gpon_driver);
    rtl960x_gpon_exit();
}

module_init(rtl960x_gpon_init_module);
module_exit(rtl960x_gpon_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Team");
MODULE_DESCRIPTION("Realtek RTL960x GPON PHY Driver for MTS Router");
MODULE_VERSION(DRIVER_VERSION);
