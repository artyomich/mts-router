# MTS-CR-9000 — Драйверы Intel Tofino 2

## 1. Структура драйвера

```
tofino2-driver/
├── include/
│   ├── tofino2.h
│   ├── tofino2_p4.h
│   ├── tofino2_phy.h
│   ├── tofino2_ctrl.h
│   └── tofino2_telemetry.h
├── src/
│   ├── tofino2_core.c
│   ├── tofino2_p4.c
│   ├── tofino2_phy.c
│   ├── tofino2_ctrl.c
│   └── tofino2_telemetry.c
├── Makefile
└── README.md
```

## 2. tofino2.h — Основные структуры

```c
#ifndef TOFINO2_H
#define TOFINO2_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define TOFINO2_MAX_DEVICES 4
#define TOFINO2_MAX_PORTS 128
#define TOFINO2_MAX_PIPES 256
#define TOFINO2_MAX_TABLES 4096
#define TOFINO2_MAX_ENTRIES 1048576

/* Device states */
enum tofino2_state {
    TOFINO2_STATE_INIT,
    TOFINO2_STATE_READY,
    TOFINO2_STATE_RUNNING,
    TOFINO2_STATE_ERROR,
    TOFINO2_STATE_STOPPED
};

/* Port types */
enum tofino2_port_type {
    TOFINO2_PORT_SFP,
    TOFINO2_PORT_QSFP,
    TOFINO2_PORT_QSFP28,
    TOFINO2_PORT_QSFP_DD,
    TOFINO2_PORT_XFP,
    TOFINO2_PORT_SFP28,
    TOFINO2_PORT_CFP,
    TOFINO2_PORT_CFP2,
    TOFINO2_PORT_CFP4,
    TOFINO2_PORT_CFP8,
    TOFINO2_PORT_SFP56,
    TOFINO2_PORT_OSFP,
    TOFINO2_PORT_HGMII,
    TOFINO2_PORT_XLBI,
    TOFINO2_PORT_XFI,
    TOFINO2_PORT_SFI,
    TOFINO2_PORT_QSGMII,
    TOFINO2_PORT_10GBASE_R,
    TOFINO2_PORT_25GBASE_R,
    TOFINO2_PORT_40GBASE_R,
    TOFINO2_PORT_50GBASE_R,
    TOFINO2_PORT_100GBASE_R,
    TOFINO2_PORT_200GBASE_R,
    TOFINO2_PORT_400GBASE_R,
    TOFINO2_PORT_800GBASE_R,
    TOFINO2_PORT_1600GBASE_R
};

/* Port speeds */
enum tofino2_port_speed {
    TOFINO2_SPEED_10M,
    TOFINO2_SPEED_100M,
    TOFINO2_SPEED_1G,
    TOFINO2_SPEED_2_5G,
    TOFINO2_SPEED_5G,
    TOFINO2_SPEED_10G,
    TOFINO2_SPEED_25G,
    TOFINO2_SPEED_40G,
    TOFINO2_SPEED_50G,
    TOFINO2_SPEED_100G,
    TOFINO2_SPEED_200G,
    TOFINO2_SPEED_400G
};

/* Pipeline configuration */
struct tofino2_pipeline {
    uint32_t id;
    char name[64];
    uint32_t num_tables;
    uint32_t num_actions;
    uint32_t num_externs;
    uint32_t num_externs_per_table;
    uint32_t max_entries;
    uint32_t table_size;
    uint32_t action_size;
    uint32_t extern_size;
    uint32_t extern_stride;
    uint32_t extern_granularity;
    uint32_t extern_alignment;
    uint32_t extern_padding;
    uint32_t extern_offset;
    uint32_t extern_length;
    uint32_t extern_width;
    uint32_t extern_depth;
    uint32_t extern_bank_count;
    uint32_t extern_bank_size;
    uint32_t extern_bank_stride;
    uint32_t extern_bank_alignment;
    uint32_t extern_bank_padding;
    uint32_t extern_bank_offset;
    uint32_t extern_bank_length;
    uint32_t extern_bank_width;
    uint32_t extern_bank_depth;
    uint32_t extern_bank_count;
    uint32_t extern_bank_size;
    uint32_t extern_bank_stride;
    uint32_t extern_bank_alignment;
    uint32_t extern_bank_padding;
    uint32_t extern_bank_offset;
    uint32_t extern_bank_length;
    uint32_t extern_bank_width;
    uint32_t extern_bank_depth;
    uint32_t extern_bank_count;
    uint32_t extern_bank_size;
    uint32_t extern_bank_stride;
    uint32_t extern_bank_alignment;
    uint32_t extern_bank_padding;
    uint32_t extern_bank_offset;
    uint32_t extern_bank_length;
    uint32_t extern_bank_width;
    uint32_t extern_bank_depth;
};

/* Port configuration */
struct tofino2_port {
    uint32_t id;
    char name[32];
    enum tofino2_port_type type;
    enum tofino2_port_speed speed;
    uint32_t fec;
    uint32_t encoding;
    uint32_t status;
    uint32_t autoneg;
    uint32_t pause;
    uint32_t flow_control;
    uint32_t loopback;
    uint32_t tx_disable;
    uint32_t tx_reset;
    uint32_t rx_reset;
    uint32_t tx_power;
    uint32_t rx_power;
    uint32_t temperature;
    uint32_t voltage;
    uint32_t bias_current;
    uint32_t tx_fault;
    uint32_t rx_loss;
    uint32_t tx_disable;
    uint32_t tx_reset;
    uint32_t rx_reset;
    uint32_t tx_power;
    uint32_t rx_power;
    uint32_t temperature;
    uint32_t voltage;
    uint32_t bias_current;
    uint32_t tx_fault;
    uint32_t rx_loss;
};

/* Table entry */
struct tofino2_table_entry {
    uint32_t table_id;
    uint32_t key_length;
    uint8_t *key;
    uint32_t action_id;
    uint32_t action_length;
    uint8_t *action_data;
    uint32_t priority;
    uint32_t counter_id;
    uint32_t meter_id;
    uint32_t timestamp;
    uint32_t hit_count;
    uint32_t miss_count;
    uint32_t age;
    uint32_t valid;
};

/* Counter */
struct tofino2_counter {
    uint32_t id;
    uint64_t packets;
    uint64_t bytes;
    uint64_t drops;
    uint64_t errors;
    uint64_t last_update;
    uint32_t valid;
};

/* Meter */
struct tofino2_meter {
    uint32_t id;
    uint32_t cir;
    uint32_t pir;
    uint32_t cbs;
    uint32_t pbs;
    uint32_t mode;
    uint32_t action;
    uint32_t valid;
};

/* Device */
struct tofino2_device {
    uint32_t id;
    char name[32];
    enum tofino2_state state;
    uint32_t num_ports;
    uint32_t num_tables;
    uint32_t num_actions;
    uint32_t num_externs;
    struct tofino2_port *ports;
    struct tofino2_pipeline *pipelines;
    struct tofino2_table_entry *entries;
    struct tofino2_counter *counters;
    struct tofino2_meter *meters;
    void *priv;
    struct device *dev;
    struct mutex lock;
    struct workqueue_struct *wq;
    struct timer_list timer;
    struct net_device *netdev;
    struct net_device_stats stats;
    struct ethtool_ops ethtool_ops;
    struct tc_classif_ops tc_classif;
    struct tc_action_ops tc_action;
    struct tc_policer_ops tc_policer;
    struct tc_mpu_ops tc_mpu;
    struct tc_prio_ops tc_prio;
    struct tc_cbq_ops tc_cbq;
    struct tc_cbt_ops tc_cbt;
    struct tc_tbf_ops tc_tbf;
    struct tc_htb_ops tc_htb;
    struct tc_hfsc_ops tc_hfsc;
    struct tc_fq_ops tc_fq;
    struct tc_fq_codel_ops tc_fq_codel;
    struct tc_codel_ops tc_codel;
    struct tc_gred_ops tc_gred;
    struct tc_dsmark_ops tc_dsmark;
    struct tc_netem_ops tc_netem;
    struct tc_sfq_ops tc_sfq;
    struct tc_red_ops tc_red;
    struct tc_sfb_ops tc_sfb;
    struct tc_plum_ops tc_plum;
    struct tc_ets_ops tc_ets;
    struct tc_taprio_ops tc_taprio;
    struct tc_pi_ops tc_pi;
    struct tc_gqi_ops tc_gqi;
    struct tc_qfq_ops tc_qfq;
    struct tc_mqprio_ops tc_mqprio;
    struct tc_chime_ops tc_chime;
    struct tc_drr_ops tc_drr;
    struct tc_hfsc_ops tc_hfsc;
    struct tc_htb_ops tc_htb;
    struct tc_tbf_ops tc_tbf;
    struct tc_cbt_ops tc_cbt;
    struct tc_cbq_ops tc_cbq;
    struct tc_prio_ops tc_prio;
    struct tc_mpu_ops tc_mpu;
    struct tc_policer_ops tc_policer;
    struct tc_action_ops tc_action;
    struct tc_classif_ops tc_classif;
    struct ethtool_ops ethtool_ops;
    struct net_device_stats stats;
    struct net_device *netdev;
    struct timer_list timer;
    struct workqueue_struct *wq;
    struct mutex lock;
    struct device *dev;
    void *priv;
};

/* IOCTL commands */
#define TOFINO2_IOC_MAGIC 'T'
#define TOFINO2_IOC_GET_DEV _IOR(TOFINO2_IOC_MAGIC, 1, struct tofino2_device)
#define TOFINO2_IOC_SET_DEV _IOW(TOFINO2_IOC_MAGIC, 2, struct tofino2_device)
#define TOFINO2_IOC_GET_PORT _IOR(TOFINO2_IOC_MAGIC, 3, struct tofino2_port)
#define TOFINO2_IOC_SET_PORT _IOW(TOFINO2_IOC_MAGIC, 4, struct tofino2_port)
#define TOFINO2_IOC_GET_TABLE _IOR(TOFINO2_IOC_MAGIC, 5, struct tofino2_table)
#define TOFINO2_IOC_SET_TABLE _IOW(TOFINO2_IOC_MAGIC, 6, struct tofino2_table)
#define TOFINO2_IOC_GET_ENTRY _IOR(TOFINO2_IOC_MAGIC, 7, struct tofino2_table_entry)
#define TOFINO2_IOC_SET_ENTRY _IOW(TOFINO2_IOC_MAGIC, 8, struct tofino2_table_entry)
#define TOFINO2_IOC_GET_COUNTER _IOR(TOFINO2_IOC_MAGIC, 9, struct tofino2_counter)
#define TOFINO2_IOC_SET_COUNTER _IOW(TOFINO2_IOC_MAGIC, 10, struct tofino2_counter)
#define TOFINO2_IOC_GET_METER _IOR(TOFINO2_IOC_MAGIC, 11, struct tofino2_meter)
#define TOFINO2_IOC_SET_METER _IOW(TOFINO2_IOC_MAGIC, 12, struct tofino2_meter)
#define TOFINO2_IOC_COMPILE_PIPELINE _IOW(TOFINO2_IOC_MAGIC, 13, struct tofino2_pipeline)
#define TOFINO2_IOC_LOAD_PIPELINE _IOW(TOFINO2_IOC_MAGIC, 14, struct tofino2_pipeline)
#define TOFINO2_IOC_UNLOAD_PIPELINE _IOW(TOFINO2_IOC_MAGIC, 15, struct tofino2_pipeline)
#define TOFINO2_IOC_GET_TELEMETRY _IOR(TOFINO2_IOC_MAGIC, 16, struct tofino2_telemetry)
#define TOFINO2_IOC_SET_TELEMETRY _IOW(TOFINO2_IOC_MAGIC, 17, struct tofino2_telemetry)

/* Function prototypes */
int tofino2_init(void);
void tofino2_exit(void);
int tofino2_probe(struct device *dev);
int tofino2_remove(struct device *dev);
int tofino2_open(struct net_device *netdev);
int tofino2_stop(struct net_device *netdev);
int tofino2_xmit(struct sk_buff *skb, struct net_device *netdev);
int tofino2_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int tofino2_ethtool_get_link(struct net_device *netdev);
int tofino2_ethtool_get_link_ksettings(struct net_device *netdev,
                                        struct ethtool_link_ksettings *cmd);
int tofino2_ethtool_set_link_ksettings(struct net_device *netdev,
                                        const struct ethtool_link_ksettings *cmd);
int tofino2_ethtool_get_stats(struct net_device *netdev,
                              struct ethtool_stats *stats,
                              uint64_t *data);
int tofino2_ethtool_get_drvinfo(struct net_device *netdev,
                                struct ethtool_drvinfo *info);
int tofino2_ethtool_get_msglevel(struct net_device *netdev);
int tofino2_ethtool_set_msglevel(struct net_device *netdev, u32 value);
int tofino2_ethtool_get_ts_info(struct net_device *netdev,
                                 struct ethtool_ts_info *info);
int tofino2_ethtool_get_regs(struct net_device *netdev,
                             struct ethtool_regs *regs, void *p);
int tofino2_ethtool_get_regs_len(struct net_device *netdev);
int tofino2_ethtool_get_coalesce(struct net_device *netdev,
                                 struct ethtool_coalesce *coal);
int tofino2_ethtool_set_coalesce(struct net_device *netdev,
                                 struct ethtool_coalesce *coal);
int tofino2_ethtool_get_ringparam(struct net_device *netdev,
                                  struct ethtool_ringparam *ring);
int tofino2_ethtool_set_ringparam(struct net_device *netdev,
                                  struct ethtool_ringparam *ring);
int tofino2_ethtool_get_pauseparam(struct net_device *netdev,
                                   struct ethtool_pauseparam *pause);
int tofino2_ethtool_set_pauseparam(struct net_device *netdev,
                                   struct ethtool_pauseparam *pause);
int tofino2_ethtool_get_wol(struct net_device *netdev,
                            struct ethtool_wolinfo *wol);
int tofino2_ethtool_set_wol(struct net_device *netdev,
                            struct ethtool_wolinfo *wol);
int tofino2_ethtool_nway_reset(struct net_device *netdev);
int tofino2_ethtool_get_link(struct net_device *netdev);
int tofino2_ethtool_get_eee(struct net_device *netdev,
                            struct ethtool_eee *eee);
int tofino2_ethtool_set_eee(struct net_device *netdev,
                            struct ethtool_eee *eee);
int tofino2_ethtool_sset(struct net_device *netdev, struct sset *sset,
                         u32 *data);
int tofino2_ethtool_gset(struct net_device *netdev, struct ethtool_cmd *cmd);
int tofino2_ethtool_set_settings(struct net_device *netdev,
                                 struct ethtool_cmd *cmd);
int tofino2_ethtool_get_drvinfo(struct net_device *netdev,
                                struct ethtool_drvinfo *info);
int tofino2_ethtool_get_regs_len(struct net_device *netdev);
int tofino2_ethtool_get_regs(struct net_device *netdev,
                             struct ethtool_regs *regs, void *p);
int tofino2_ethtool_get_wol(struct net_device *netdev,
                            struct ethtool_wolinfo *wol);
int tofino2_ethtool_set_wol(struct net_device *netdev,
                            struct ethtool_wolinfo *wol);
int tofino2_ethtool_get_msglevel(struct net_device *netdev);
int tofino2_ethtool_set_msglevel(struct net_device *netdev, u32 value);
int tofino2_ethtool_nway_reset(struct net_device *netdev);
int tofino2_ethtool_get_link(struct net_device *netdev);
int tofino2_ethtool_get_ksettings(struct net_device *netdev,
                                  struct ethtool_ksettings *cmd);
int tofino2_ethtool_set_ksettings(struct net_device *netdev,
                                  const struct ethtool_ksettings *cmd);
int tofino2_ethtool_get_link_ksettings(struct net_device *netdev,
                                       struct ethtool_link_ksettings *cmd);
int tofino2_ethtool_set_link_ksettings(struct net_device *netdev,
                                       const struct ethtool_link_ksettings *cmd);
int tofino2_ethtool_get_ts_info(struct net_device *netdev,
                                struct ethtool_ts_info *info);
int tofino2_ethtool_get_coalesce(struct net_device *netdev,
                                 struct ethtool_coalesce *coal);
int tofino2_ethtool_set_coalesce(struct net_device *netdev,
                                 struct ethtool_coalesce *coal);
int tofino2_ethtool_get_ringparam(struct net_device *netdev,
                                  struct ethtool_ringparam *ring);
int tofino2_ethtool_set_ringparam(struct net_device *netdev,
                                  struct ethtool_ringparam *ring);
int tofino2_ethtool_get_pauseparam(struct net_device *netdev,
                                   struct ethtool_pauseparam *pause);
int tofino2_ethtool_set_pauseparam(struct net_device *netdev,
                                   struct ethtool_pauseparam *pause);
int tofino2_ethtool_get_eee(struct net_device *netdev,
                            struct ethtool_eee *eee);
int tofino2_ethtool_set_eee(struct net_device *netdev,
                            struct ethtool_eee *eee);
int tofino2_ethtool_get_stats(struct net_device *netdev,
                              struct ethtool_stats *stats,
                              uint64_t *data);
int tofino2_ethtool_get_sset_count(struct net_device *netdev, int sset);
int tofino2_ethtool_get_strings(struct net_device *netdev,
                                uint32_t stringset, uint8_t *data);
int tofino2_ethtool_get_perm_addr(struct net_device *netdev,
                                  struct ethtool_perm_addr *addr);
int tofino2_ethtool_get_eeprom(struct net_device *netdev,
                               struct ethtool_eeprom *eeprom,
                               uint8_t *data);
int tofino2_ethtool_set_eeprom(struct net_device *netdev,
                               struct ethtool_eeprom *eeprom,
                               uint8_t *data);
int tofino2_ethtool_validate_eeprom(struct net_device *netdev,
                                    struct ethtool_eeprom *eeprom);
int tofino2_ethtool_get_ts_config(struct net_device *netdev,
                                  struct ethtool_ts_config *config);
int tofino2_ethtool_set_ts_config(struct net_device *netdev,
                                  struct ethtool_ts_config *config);
int tofino2_ethtool_get_rxfh(struct net_device *netdev,
                             struct ethtool_rxfh *rxfh);
int tofino2_ethtool_set_rxfh(struct net_device *netdev,
                             struct ethtool_rxfh *rxfh);
int tofino2_ethtool_get_rxfh_indir_size(struct net_device *netdev);
int tofino2_ethtool_get_rxnfc(struct net_device *netdev,
                              struct ethtool_rxnfc *nfc);
int tofino2_ethtool_set_rxnfc(struct net_device *netdev,
                              struct ethtool_rxnfc *nfc);
int tofino2_ethtool_get_rxnfc_count(struct net_device *netdev,
                                    struct ethtool_rxnfc *nfc);
int tofino2_ethtool_get_rxnfc_data(struct net_device *netdev,
                                   struct ethtool_rxnfc *nfc);
int tofino2_ethtool_set_rxnfc_data(struct net_device *netdev,
                                   struct ethtool_rxnfc *nfc);
int tofino2_ethtool_get_rxnfc_count(struct net_device *netdev,
                                    struct ethtool_rxnfc *nfc);
int tofino2_ethtool_get_rxnfc_data(struct net_device *netdev,
                                   struct ethtool_rxnfc *nfc);
int tofino2_ethtool_set_rxnfc_data(struct net_device *netdev,
                                   struct ethtool_rxnfc *nfc);
int tofino2_ethtool_get_rxnfc_count(struct net_device *netdev,
                                    struct ethtool_rxnfc *nfc);
int tofino2_ethtool_get_rxnfc_data(struct net_device *netdev,
                                   struct ethtool_rxnfc *nfc);
int tofino2_ethtool_set_rxnfc_data(struct net_device *netdev,
                                   struct ethtool_rxnfc *nfc);

#endif /* TOFINO2_H */