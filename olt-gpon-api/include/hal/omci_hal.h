/**
 * MTS-OLT-2000 OMCI HAL — OMCI management
 */

#ifndef MTS_OLT2000_OMCI_HAL_H
#define MTS_OLT2000_OMCI_HAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MTS_OLT2000_OMCI_MAX_ENTITIES 256

/* OMCI entity types */
typedef enum {
    MTS_OLT2000_OMCI_ENTITY_ONU = 1,
    MTS_OLT2000_OMCI_ENTITY_ETH_PORT = 2,
    MTS_OLT2000_OMCI_ENTITY_POTS_PORT = 3,
    MTS_OLT2000_OMCI_ENTITY_GPON_TN = 4,
    MTS_OLT2000_OMCI_ENTITY_VLAN_TNP = 5,
    MTS_OLT2000_OMCI_ENTITY_MULTICAST_SET = 6,
    MTS_OLT2000_OMCI_ENTITY_VLAN_FILTER = 7,
    MTS_OLT2000_OMCI_ENTITY_QOS_TRAFFIC_CTRL = 8,
    MTS_OLT2000_OMCI_ENTITY_LINK = 9
} mts_olt2000_omci_entity_type_t;

/* OMCI entity status */
typedef enum {
    MTS_OLT2000_OMCI_ENTITY_ACTIVE = 0,
    MTS_OLT2000_OMCI_ENTITY_INACTIVE,
    MTS_OLT2000_OMCI_ENTITY_TESTING,
    MTS_OLT2000_OMCI_ENTITY_CREATING,
    MTS_OLT2000_OMCI_ENTITY_DELETING
} mts_olt2000_omci_entity_status_t;

/* OMCI entity info */
typedef struct {
    uint32_t entity_id;
    mts_olt2000_omci_entity_type_t type;
    mts_olt2000_omci_entity_status_t status;
    char name[64];
    uint32_t associated_entities[8];
    uint32_t num_associated;
} mts_olt2000_omci_entity_t;

/* OMCI status */
typedef struct {
    mts_olt2000_omci_entity_status_t status;
    uint32_t total_entities;
    uint32_t active_entities;
    uint32_t inactive_entities;
    uint32_t error_entities;
} mts_olt2000_omci_status_t;

/* ==================== API ==================== */

/**
 * Initialize OMCI HAL
 * @return 0 on success, -1 on error
 */
int mts_olt2000_omci_init(void);

/**
 * Cleanup OMCI HAL
 */
void mts_olt2000_omci_cleanup(void);

/**
 * Get OMCI status
 * @param status Output buffer for OMCI status
 * @return 0 on success, -1 on error
 */
int mts_olt2000_omci_get_status(mts_olt2000_omci_status_t *status);

/**
 * Get OMCI entity
 * @param entity_id Entity ID
 * @param entity Output buffer for entity info
 * @return 0 on success, -1 on error
 */
int mts_olt2000_omci_get_entity(uint32_t entity_id, mts_olt2000_omci_entity_t *entity);

/**
 * Get all OMCI entities
 * @param entities Output array of entities
 * @param max_entities Maximum number of entities
 * @return Number of entities returned, -1 on error
 */
int mts_olt2000_omci_get_all_entities(mts_olt2000_omci_entity_t *entities, int max_entities);

/**
 * Create OMCI entity
 * @param type Entity type
 * @param entity Output buffer for created entity
 * @return 0 on success, -1 on error
 */
int mts_olt2000_omci_create_entity(mts_olt2000_omci_entity_type_t type, mts_olt2000_omci_entity_t *entity);

/**
 * Delete OMCI entity
 * @param entity_id Entity ID
 * @return 0 on success, -1 on error
 */
int mts_olt2000_omci_delete_entity(uint32_t entity_id);

#ifdef __cplusplus
}
#endif

#endif /* MTS_OLT2000_OMCI_HAL_H */
