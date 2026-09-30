/*
 * tomtom_phy.h — PHY layer for Broadcom TomTom
 *
 * MTS-ER-1000 Enterprise Router — Управление PHY портами
 */

#ifndef TOMTOM_PHY_H
#define TOMTOM_PHY_H

#include <linux/types.h>

#define TOMTOM_PHY_MAX_PORTS 64
#define TOMTOM_PHY_MAX_SPEED 400000 /* 400 Gbps */

/* PHY port status */
enum tomtom_phy_status {
    TOMTOM_PHY_DOWN = 0,
    TOMTOM_PHY_UP = 1,
    TOMTOM_PHY_MAINTENANCE = 2,
    TOMTOM_PHY_FAULT = 3
};

/* PHY port */
struct tomtom_phy_port {
    uint32_t id;
    char name[32];
    uint32_t status;        /* TOMTOM_PHY_* */
    uint32_t speed;         /* Mbps */
    uint32_t duplex;        /* half/full */
    uint32_t auto_neg;
    uint32_t fec_enabled;
    uint32_t tx_power;      /* dBm * 100 */
    uint32_t rx_power;      /* dBm * 100 */
    uint32_t temperature;   /* Celsius * 100 */
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t tx_errors;
    uint64_t rx_errors;
    uint64_t tx_drops;
    uint64_t rx_drops;
};

/* PHY device */
struct tomtom_phy_device {
    uint32_t id;
    char name[32];
    struct tomtom_phy_port ports[TOMTOM_PHY_MAX_PORTS];
    uint32_t num_ports;
    void *priv;
    struct device *dev;
    struct mutex lock;
};

/* Function prototypes */
int tomtom_phy_init(struct tomtom_phy_device *pdev);
void tomtom_phy_exit(struct tomtom_phy_device *pdev);
int tomtom_phy_probe(struct device *dev);
int tomtom_phy_remove(struct device *dev);
int tomtom_phy_set_port_status(struct tomtom_phy_device *pdev, uint32_t port_id, uint32_t status);
int tomtom_phy_get_port_status(struct tomtom_phy_device *pdev, uint32_t port_id);
int tomtom_phy_set_speed(struct tomtom_phy_device *pdev, uint32_t port_id, uint32_t speed);
int tomtom_phy_get_speed(struct tomtom_phy_device *pdev, uint32_t port_id);
int tomtom_phy_set_tx_power(struct tomtom_phy_device *pdev, uint32_t port_id, uint32_t power);
int tomtom_phy_get_rx_power(struct tomtom_phy_device *pdev, uint32_t port_id);
int tomtom_phy_get_temperature(struct tomtom_phy_device *pdev, uint32_t port_id);
int tomtom_phy_reset_port(struct tomtom_phy_device *pdev, uint32_t port_id);

#endif /* TOMTOM_PHY_H */
