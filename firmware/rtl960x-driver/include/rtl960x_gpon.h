/*
 * rtl960x_gpon.h — Драйвер GPON line interface для RTL960x
 *
 * MTS-OLT-2000 GPON OLT — Управление GPON портами
 */

#ifndef RTL960X_GPON_H
#define RTL960X_GPON_H

#include <linux/types.h>
#include <linux/netdevice.h>

#define RTL960X_GPON_MAX_PORTS 8
#define RTL960X_GPON_MAX_PON  4
#define RTL960X_GPON_ONU_PER_PON 128

/* GPON encapsulation types */
enum gpon_encap {
    GPON_ENCAP_ATM,
    GPON_ENAP_GEM,
    GPON_ENCAP_MIXED
};

/* GPON status */
enum gpon_port_status {
    GPON_PORT_DOWN,
    GPON_PORT_UP,
    GPON_PORT_MAINTENANCE,
    GPON_PORT_FAULT
};

/* GPON port */
struct gpon_port {
    uint32_t id;
    char name[32];
    enum gpon_port_status status;
    uint32_t mode;            /* ONU/OLT */
    uint32_t tx_power;        /* dBm * 100 */
    uint32_t rx_power;        /* dBm * 100 */
    uint32_t bias_current;    /* mA * 100 */
    uint32_t temperature;     /* Celsius * 100 */
    uint32_t pon_id;
    uint32_t num_onu;
    uint32_t max_onu;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t tx_errors;
    uint64_t rx_errors;
    uint64_t crc_errors;
    uint64_t lip_errors;
};

/* GPON device */
struct gpon_device {
    uint32_t id;
    char name[32];
    struct gpon_port ports[RTL960X_GPON_MAX_PORTS];
    void *priv;
    struct device *dev;
    struct mutex lock;
    enum gpon_encap encap_type;
};

/* GPON IOCTL commands */
#define GPON_IOC_MAGIC 'G'
#define GPON_IOC_GET_PORT     _IOR(GPON_IOC_MAGIC, 1, struct gpon_port)
#define GPON_IOC_SET_PORT     _IOW(GPON_IOC_MAGIC, 2, struct gpon_port)
#define GPON_IOC_GET_PORTS    _IOR(GPON_IOC_MAGIC, 3, struct gpon_port[])
#define GPON_IOC_SET_MODE     _IOW(GPON_IOC_MAGIC, 4, uint32_t)
#define GPON_IOC_SET_TX_POWER _IOW(GPON_IOC_MAGIC, 5, uint32_t)
#define GPON_IOC_GET_TEMP     _IOR(GPON_IOC_MAGIC, 6, uint32_t)
#define GPON_IOC_GET_BIAS     _IOR(GPON_IOC_MAGIC, 7, uint32_t)
#define GPON_IOC_RESET_PORT   _IOW(GPON_IOC_MAGIC, 8, uint32_t)
#define GPON_IOC_GET_STATS    _IOR(GPON_IOC_MAGIC, 9, struct gpon_port)

/* Function prototypes */
int gpon_init(struct gpon_device *gdev);
void gpon_exit(struct gpon_device *gdev);
int gpon_probe(struct device *dev);
int gpon_remove(struct device *dev);
int gpon_open(struct net_device *netdev);
int gpon_stop(struct net_device *netdev);
int gpon_xmit(struct sk_buff *skb, struct net_device *netdev);
int gpon_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int gpon_set_encap_type(struct gpon_device *gdev, enum gpon_encap type);
enum gpon_encap gpon_get_encap_type(struct gpon_device *gdev);
int gpon_set_port_mode(struct gpon_device *gdev, uint32_t port_id, uint32_t mode);
int gpon_get_port_status(struct gpon_device *gdev, uint32_t port_id);
int gpon_set_tx_power(struct gpon_device *gdev, uint32_t port_id, uint32_t power);
int gpon_get_rx_power(struct gpon_device *gdev, uint32_t port_id);
int gpon_get_temperature(struct gpon_device *gdev, uint32_t *temp);
int gpon_get_bias_current(struct gpon_device *gdev, uint32_t *bias);
int gpon_reset_port(struct gpon_device *gdev, uint32_t port_id);

#endif /* RTL960X_GPON_H */
