/*
 * rtl960x_omci.h — Драйвер OMCI (ONT Management and Control Interface)
 *
 * MTS-OLT-2000 GPON OLT — Управление OMCI entities
 */

#ifndef RTL960X_OMCI_H
#define RTL960X_OMCI_H

#include <linux/types.h>

#define RTL960X_OMCI_MAX_ENTITIES 256
#define RTL960X_OMCI_MAX_POTS 64
#define RTL960X_OMCI_MAX_ETH_PORTS 32

/* OMCI entity types */
enum omci_entity_type {
    OMCI_ENTITY_GPTON = 1,
    OMCI_ENTITY_NPT = 2,
    OMNI_ENTITY_ETH_TBI = 3,
    OMCI_ENTITY_ETH_PORT = 4,
    OMCI_ENTITY_POTS_PORT = 5,
    OMCI_ENTITY_POTS_LINE = 6,
    OMCI_ENTITY_VLAN_PORT = 7,
    OMCI_ENTITY_IP_DEVICE = 8,
    OMCI_ENTITY_ETH_SWITCH_PORT = 9,
    OMCI_ENTITY_WLAN_DEVICE = 10,
    OMCI_ENTITY_WLAN_PORT = 11,
    OMCI_ENTITY_QOS_UNI_PORT = 12,
    OMCI_ENTITY_MULTICAST_GROUP = 13,
    OMCI_ENTITY_BDI = 14,
    OMCI_ENTITY_FDI = 15,
    OMCI_ENTITY_HDB_POTS = 16
};

/* OMCI message types */
enum omci_msg_type {
    OMCI_MSG_CREATE_ENTITY = 1,
    OMCI_MSG_DELETE_ENTITY = 2,
    OMCI_MSG_GET_ATTRIBUTES = 3,
    OMCI_MSG_SET_ATTRIBUTES = 4,
    OMCI_MSG_GET_ATTRIBUTES_EX = 5,
    OMCI_MSG_TEST = 6,
    OMCI_MSG_TEST_RESPONSE = 7,
    OMCI_MSG_NOTIFY = 8,
    OMCI_MSG_CREATE_ENTITY_ACK = 129,
    OMCI_MSG_DELETE_ENTITY_ACK = 130,
    OMCI_MSG_GET_ATTRIBUTES_ACK = 131,
    OMCI_MSG_SET_ATTRIBUTES_ACK = 132,
    OMCI_MSG_ERROR = 133,
    OMCI_MSG_EVENT_ALERT = 134
};

/* OMCI entity */
struct omci_entity {
    uint16_t entity_inst;
    enum omci_entity_type type;
    uint16_t managed_class;
    uint32_t status;       /* ACTIVE/INACTIVE/FAULT */
    uint32_t operating_state;
    uint32_t init_state;
    uint64_t created;
    uint64_t last_seen;
    uint8_t  attributes[256];
    uint32_t attr_len;
};

/* OMCI device */
struct omci_device {
    uint32_t id;
    char name[32];
    struct omci_entity entities[RTL960X_OMCI_MAX_ENTITIES];
    uint32_t num_entities;
    void *priv;
    struct device *dev;
    struct mutex lock;
    uint32_t onu_mac[6];
    uint32_t vendor_id;
    uint32_t serial_num;
    uint32_t sw_version[4];
};

/* OMCI IOCTL commands */
#define OMCI_IOC_MAGIC 'O'
#define OMCI_IOC_ADD_ENTITY     _IOW(OMCI_IOC_MAGIC, 1, struct omci_entity)
#define OMCI_IOC_DEL_ENTITY     _IO(OMCI_IOC_MAGIC, 2)
#define OMCI_IOC_GET_ENTITY     _IOR(OMCI_IOC_MAGIC, 3, struct omci_entity)
#define OMCI_IOC_SET_ATTR       _IOW(OMCI_IOC_MAGIC, 4, struct omci_entity)
#define OMCI_IOC_GET_ATTR       _IOR(OMCI_IOC_MAGIC, 5, struct omci_entity)
#define OMCI_IOC_GET_ENTITIES   _IOR(OMCI_IOC_MAGIC, 6, struct omci_entity[])
#define OMCI_IOC_NOTIFY         _IOW(OMCI_IOC_MAGIC, 7, struct omci_entity)
#define OMCI_IOC_ALERT          _IOW(OMCI_IOC_MAGIC, 8, uint16_t)

/* Function prototypes */
int omci_init(struct omci_device *odev);
void omci_exit(struct omci_device *odev);
int omci_probe(struct device *dev);
int omci_remove(struct device *dev);
int omci_create_entity(struct omci_device *odev, struct omci_entity *entity);
int omci_delete_entity(struct omci_device *odev, uint16_t entity_inst);
int omci_get_entity(struct omci_device *odev, uint16_t entity_inst, struct omci_entity *entity);
int omci_set_attributes(struct omci_device *odev, struct omci_entity *entity);
int omci_get_attributes(struct omci_device *odev, struct omci_entity *entity);
int omci_get_all_entities(struct omci_device *odev, struct omci_entity *entities, uint32_t max);
int omci_notify(struct omci_device *odev, struct omci_entity *entity);
int omci_alert(struct omci_device *odev, uint16_t alert_id);
int omci_send_message(struct omci_device *odev, uint8_t msg_type, uint8_t *data, uint32_t len);

#endif /* RTL960X_OMCI_H */
