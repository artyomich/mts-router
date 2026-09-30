/* SPDX-License-Identifier: GPL-2.0 */
/*
 * tofino2_phy.h - PHY/PHY Layer Interface
 *
 * MTS-CR-9000 Core Router Tofino 2 Driver
 *
 * Copyright (c) 2024 MTS Router Project
 */

#ifndef TOFINO2_PHY_H
#define TOFINO2_PHY_H

#include <linux/types.h>

/* ============================================================================ */
/* PHY constants                                                                */
/* ============================================================================ */

#define TOFINO2_PHY_MAX_MODULES    128
#define TOFINO2_PHY_I2C_ADDR       0x34
#define TOFINO2_PHY_PAGE_MAX       16
#define TOFINO2_PHY_REG_MAX        256
#define TOFINO2_PHY_AN_MAX_FIRMWARE 32
#define TOFINO2_PHY_TX_FIR_MAX_TAPS 5
#define TOFINO2_PHY_RX_FIR_MAX_TAPS 5
#define TOFINO2_PHY_DSP_COEFF_MAX  16
#define TOFINO2_PHY_PMA_TYPE_MAX   64

/* ============================================================================ */
/* PHY enums                                                                    */
/* ============================================================================ */

/**
 * enum tofino2_phy_module_type - SFP/QSFP module types
 */
enum tofino2_phy_module_type {
    TOFINO2_PHY_MODULE_NONE = 0,
    TOFINO2_PHY_MODULE_SFP,
    TOFINO2_PHY_MODULE_SFP28,
    TOFINO2_PHY_MODULE_QSFP,
    TOFINO2_PHY_MODULE_QSFP28,
    TOFINO2_PHY_MODULE_QSFP56,
    TOFINO2_PHY_MODULE_QSFP_DD,
    TOFINO2_PHY_MODULE_OSFP,
    TOFINO2_PHY_MODULE_CFP,
    TOFINO2_PHY_MODULE_CFP2,
    TOFINO2_PHY_MODULE_CFP4,
    TOFINO2_PHY_MODULE_CFP8,
    TOFINO2_PHY_MODULE_XFP,
    TOFINO2_PHY_MODULE_XFI,
    TOFINO2_PHY_MODULE_SFI,
    TOFINO2_PHY_MODULE_UNKNOWN
};

/**
 * enum tofino2_phy_media_type - Media type
 */
enum tofino2_phy_media_type {
    TOFINO2_PHY_MEDIA_COPPER = 0,
    TOFINO2_PHY_MEDIA_SR,
    TOFINO2_PHY_MEDIA_LR,
    TOFINO2_PHY_MEDIA_ER,
    TOFINO2_PHY_MEDIA_FR,
    TOFINO2_PHY_MEDIA_DR,
    TOFINO2_PHY_MEDIA_CR,
    TOFINO2_PHY_MEDIA_AOC,
    TOFINO2_PHY_MEDIA_DAC,
    TOFINO2_PHY_MEDIA_BDM,
    TOFINO2_PHY_MEDIA_UNKNOWN
};

/**
 * enum tofino2_phy_pma_type - PMA (Physical Media Attached) types
 */
enum tofino2_phy_pma_type {
    TOFINO2_PHY_PMA_NONE = 0,
    TOFINO2_PHY_PMA_KR,
    TOFINO2_PHY_PMA_LR,
    TOFINO2_PHY_PMA_SR,
    TOFINO2_PHY_PMA_ER,
    TOFINO2_PHY_PMA_FR,
    TOFINO2_PHY_PMA_DR,
    TOFINO2_PHY_PMA_CR,
    TOFINO2_PHY_PMA_T,
    TOFINO2_PHY_PMA_FI,
    TOFINO2_PHY_PMA_XL,
    TOFINO2_PHY_PMA_XR,
    TOFINO2_PHY_PMA_25G_LR,
    TOFINO2_PHY_PMA_25G_SR,
    TOFINO2_PHY_PMA_50G_LR,
    TOFINO2_PHY_PMA_50G_SR,
    TOFINO2_PHY_PMA_100G_LR4,
    TOFINO2_PHY_PMA_100G_ER,
    TOFINO2_PHY_PMA_100G_SR10,
    TOFINO2_PHY_PMA_100G_CR4,
    TOFINO2_PHY_PMA_200G_FR,
    TOFINO2_PHY_PMA_200G_DR,
    TOFINO2_PHY_PMA_400G_LR4,
    TOFINO2_PHY_PMA_400G_FR,
    TOFINO2_PHY_PMA_400G_DR,
    TOFINO2_PHY_PMA_400G_CR8,
    TOFINO2_PHY_PMA_400G_SR16,
    TOFINO2_PHY_PMA_UNKNOWN
};

/**
 * enum tofino2_phy_link_speed - Link speed indicator
 */
enum tofino2_phy_link_speed {
    TOFINO2_PHY_SPEED_10G   = 10,
    TOFINO2_PHY_SPEED_25G   = 25,
    TOFINO2_PHY_SPEED_40G   = 40,
    TOFINO2_PHY_SPEED_50G   = 50,
    TOFINO2_PHY_SPEED_100G  = 100,
    TOFINO2_PHY_SPEED_200G  = 200,
    TOFINO2_PHY_SPEED_400G  = 400
};

/* ============================================================================ */
/* PHY structures                                                               */
/* ============================================================================ */

/**
 * struct tofino2_phy_module - SFP/QSFP module information
 */
struct tofino2_phy_module {
    uint32_t id;                            /* Module ID */
    uint32_t port_id;                       /* Associated port */
    uint8_t present;                        /* Module present */
    uint8_t supported;                      /* Supported by port */
    uint8_t power_level_dbm;                /* TX power (signed dBm) */
    uint8_t rx_power_dbm;                   /* RX power (signed dBm) */
    int8_t temperature_c;                   /* Temperature (signed C) */
    uint16_t voltage_mv;                    /* Voltage (mV) */
    uint16_t bias_current_ma;               /* Bias current (mA) */
    uint8_t tx_fault;                       /* TX fault */
    uint8_t tx_disable;                     /* TX disabled */
    uint8_t tx_disabled;                    /* TX actually disabled */
    uint8_t rx_loss;                        /* RX loss of signal */
    enum tofino2_phy_module_type type;      /* Module type */
    enum tofino2_phy_media_type media;      /* Media type */
    uint8_t ieee_1024_id[16];              /* IEEE EUI-64 */
    uint8_t serial[16];                    /* Serial number */
    uint8_t part_num[32];                  /* Part number */
    uint8_t vendor[32];                    /* Vendor name */
    uint8_t vendor_oui[3];                 /* Vendor OUI */
    uint8_t revision[8];                   /* Vendor revision */
    uint8_t date_code[6];                  /* Date code (YYMMDD) */
    uint8_t ext_id;                        /* Extended ID */
    uint8_t flags;                         /* Module flags */
    uint32_t capabilities;                 /* Capabilities bitmap */
    uint32_t supported_speeds;             /* Supported speeds */
    uint32_t connector;                    /* Connector type */
    uint8_t encoding;                      /* Encoding */
    uint8_t br;                            /* Bit rate */
    uint8_t om;                            /* Opto metrics */
    uint8_t discipl;                       /* Discrimination */
    uint8_t rate_speed_id;                 /* Rate speed ID */
    uint8_t ff[12];                       /* Flags field */
    uint8_t ccmode;                       /* CC mode */
    uint8_t spec_compl;                   /* Compliance */
    uint8_t smid[8];                      /* SMID */
    uint8_t sm[128];                      /* SMDM */
    uint8_t sm_id;                        /* SM ID */
    uint8_t sm_rev[4];                    /* SM revision */
    uint8_t padding[2];                   /* Padding */
};

/**
 * struct tofino2_phy_an - Auto-negotiation state
 */
struct tofino2_phy_an {
    uint8_t enabled;                        /* Auto-negotiation enabled */
    uint8_t restarted;                      /* Restart advertised */
    uint8_t complete;                       /* AN complete */
    uint8_t parallel_fault;                 /* Parallel fault */
    uint16_t advertised;                    /* Advertised abilities */
    uint16_t local_ability;                 /* Local ability */
    uint16_t remote_ability;               /* Remote ability */
    uint16_t negotiated;                    /* Negotiated speed */
    uint8_t page_tx[32];                   /* TX pages */
    uint8_t page_rx[32];                   /* RX pages */
    uint8_t ns_page[8];                    /* Next page */
    uint8_t lp_page[8];                    /* Link partner page */
};

/**
 * struct tofino2_phy_fir - FIR/TAP coefficients
 */
struct tofino2_phy_fir {
    int8_t taps[TOFINO2_PHY_TX_FIR_MAX_TAPS + 1]; /* TX FIR taps */
    int8_t rx_taps[TOFINO2_PHY_RX_FIR_MAX_TAPS + 1]; /* RX FIR taps */
    uint8_t num_taps;                      /* Number of active taps */
    uint8_t enabled;                       /* FIR enabled */
};

/**
 * struct tofino2_phy_dsp - DSP equalization coefficients
 */
struct tofino2_phy_dsp {
    int16_t coeffs[TOFINO2_PHY_DSP_COEFF_MAX]; /* DSP coefficients */
    uint8_t num_coeffs;                    /* Number of coefficients */
    uint8_t enabled;                       /* DSP enabled */
    uint16_t mode;                         /* DSP mode */
};

/**
 * struct tofino2_phy_pma - PMA (Physical Media Attached) info
 */
struct tofino2_phy_pma {
    uint8_t type;                          /* PMA type */
    uint8_t vendor[TOFINO2_PHY_PMA_TYPE_MAX]; /* PMA vendor string */
    uint8_t firmware_version[16];          /* Firmware version */
    uint8_t part_num[32];                 /* Part number */
    uint8_t serial[16];                   /* Serial number */
    uint8_t vendor_oui[3];               /* Vendor OUI */
    uint16_t revision;                    /* Revision */
    uint32_t capabilities;                /* Capabilities */
    uint32_t supported_speeds;            /* Supported speeds */
    uint32_t current_speed;               /* Current speed */
    uint32_t fec_capable;                 /* FEC capabilities */
    uint32_t fec_enabled;                 /* FEC enabled */
    uint32_t tx_fir_pre;                  /* TX FIR pre-tap */
    int8_t tx_fir_post[TOFINO2_PHY_TX_FIR_MAX_TAPS]; /* TX FIR post-taps */
    uint32_t rx_fir_enabled;              /* RX FIR enabled */
    int8_t rx_fir_taps[TOFINO2_PHY_RX_FIR_MAX_TAPS]; /* RX FIR taps */
    uint8_t bias_enabled;                 /* Bias current enabled */
    uint8_t laser_enabled;                /* Laser enabled */
    uint8_t tx_power;                     /* TX power level */
    uint8_t rx_power;                     /* RX power level */
    uint8_t temperature;                  /* Temperature */
    uint8_t voltage;                      /* Voltage */
    uint8_t pad[2];
};

/**
 * struct tofino2_phy_diag - Full PHY diagnostics
 */
struct tofino2_phy_diag {
    struct tofino2_phy_module module;     /* Module info */
    struct tofino2_phy_an an;             /* Auto-negotiation */
    struct tofino2_phy_fir fir;           /* FIR coefficients */
    struct tofino2_phy_dsp dsp;           /* DSP coefficients */
    struct tofino2_phy_pma pma;           /* PMA info */
    int32_t tx_power_dbm;                 /* TX power (signed) */
    int32_t rx_power_dbm;                 /* RX power (signed) */
    int32_t temperature_c;                /* Temperature (signed) */
    uint16_t voltage_mv;                  /* Voltage (mV) */
    uint16_t bias_current_ma;             /* Bias current (mA) */
    uint32_t tx_fir_pre;                  /* TX FIR pre-tap (scaled) */
    int16_t tx_fir_post[TOFINO2_PHY_TX_FIR_MAX_TAPS]; /* TX FIR post-taps */
    uint32_t rx_fir_enabled;              /* RX FIR enabled */
    int16_t rx_fir_taps[TOFINO2_PHY_RX_FIR_MAX_TAPS]; /* RX FIR taps */
    uint32_t dsdl;                        /* DSDL value */
    uint32_t dsdr;                        /* DSDR value */
    uint32_t margin_rx;                   /* RX eye margin */
    uint32_t margin_tx;                   /* TX eye margin */
    uint32_t ber_test;                    /* BER test result */
    uint32_t ber_errors;                  /* BER error count */
    uint32_t ber_test_duration;            /* BER test duration */
    uint8_t laser_locked;                 /* Laser locked */
    uint8_t bias_current;                 /* Bias current status */
    uint8_t tx_power_ctrl;                /* TX power control */
    uint8_t rx_power_ctrl;                /* RX power control */
    uint8_t temperature_ctrl;             /* Temperature control */
    uint8_t voltage_ctrl;                 /* Voltage control */
    uint8_t pad[3];
};

/* ============================================================================ */
/* PHY API Functions                                                              */
/* ============================================================================ */

/* Module detection */
int tofino2_phy_module_detect(uint32_t port_id);
int tofino2_phy_module_present(uint32_t port_id);
int tofino2_phy_module_get(uint32_t port_id,
                            struct tofino2_phy_module *module);
int tofino2_phy_module_set_tx_disable(uint32_t port_id, int disable);
int tofino2_phy_module_eeprom_read(uint32_t port_id, uint32_t offset,
                                    uint8_t *data, uint32_t len);
int tofino2_phy_module_eeprom_write(uint32_t port_id, uint32_t offset,
                                     const uint8_t *data, uint32_t len);

/* Link management */
int tofino2_phy_link_up(uint32_t port_id);
int tofino2_phy_link_down(uint32_t port_id);
int tofino2_phy_link_status(uint32_t port_id, uint32_t *status);
int tofino2_phy_get_speed(uint32_t port_id, uint32_t *speed);
int tofino2_phy_set_speed(uint32_t port_id, uint32_t speed);

/* Auto-negotiation */
int tofino2_phy_an_enable(uint32_t port_id);
int tofino2_phy_an_disable(uint32_t port_id);
int tofino2_phy_an_restart(uint32_t port_id);
int tofino2_phy_an_get_ability(uint32_t port_id, uint16_t *ability);
int tofino2_phy_an_set_ability(uint32_t port_id, uint16_t ability);
int tofino2_phy_an_get_remote_ability(uint32_t port_id, uint16_t *ability);
int tofino2_phy_an_is_complete(uint32_t port_id);

/* FEC */
int tofino2_phy_fec_enable(uint32_t port_id, int fec_type);
int tofino2_phy_fec_disable(uint32_t port_id);
int tofino2_phy_fec_get(uint32_t port_id, int *fec_type);
int tofino2_phy_fec_get_stats(uint32_t port_id,
                               uint32_t *fec_corrected,
                               uint32_t *fec_uncorrected);

/* FIR/DSP tuning */
int tofino2_phy_fir_get(uint32_t port_id, struct tofino2_phy_fir *fir);
int tofino2_phy_fir_set(uint32_t port_id, const struct tofino2_phy_fir *fir);
int tofino2_phy_dsp_get(uint32_t port_id, struct tofino2_phy_dsp *dsp);
int tofino2_phy_dsp_set(uint32_t port_id, const struct tofino2_phy_dsp *dsp);
int tofino2_phy_fir_autotune(uint32_t port_id);

/* PMA management */
int tofino2_phy_pma_get(uint32_t port_id, struct tofino2_phy_pma *pma);
int tofino2_phy_pma_set(uint32_t port_id, const struct tofino2_phy_pma *pma);
int tofino2_phy_pma_firmware_update(uint32_t port_id,
                                      const uint8_t *fw, uint32_t size);
int tofino2_phy_pma_get_firmware_info(uint32_t port_id,
                                       char *version, uint32_t ver_size);

/* Diagnostics */
int tofino2_phy_diag_get(uint32_t port_id,
                          struct tofino2_phy_diag *diag);
int tofino2_phy_diag_get_extended(uint32_t port_id,
                                   struct tofino2_phy_diag *diag,
                                   uint32_t flags);
int tofino2_phy_ber_test(uint32_t port_id, uint32_t duration_ms,
                          uint32_t *errors);
int tofino2_phy_loopback_set(uint32_t port_id, int enable, int type);
int tofino2_phy_fault_clear(uint32_t port_id);

/* Interrupts */
int tofino2_phy_irq_enable(uint32_t port_id, uint32_t mask);
int tofino2_phy_irq_disable(uint32_t port_id, uint32_t mask);
int tofino2_phy_irq_get(uint32_t port_id, uint32_t *status);
int tofino2_phy_irq_ack(uint32_t port_id, uint32_t mask);

/* I2C access */
int tofino2_phy_i2c_read(uint32_t port_id, uint8_t reg, uint8_t *val);
int tofino2_phy_i2c_write(uint32_t port_id, uint8_t reg, uint8_t val);
int tofino2_phy_i2c_read_page(uint32_t port_id, uint8_t page,
                               uint8_t reg, uint8_t *val);
int tofino2_phy_i2c_write_page(uint32_t port_id, uint8_t page,
                                uint8_t reg, uint8_t val);

#endif /* TOFINO2_PHY_H */
