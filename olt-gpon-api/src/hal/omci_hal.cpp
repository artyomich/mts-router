/**
 * MTS-OLT-2000 OMCI HAL — OMCI management implementation
 * Provides OMCI entity management
 */

#include "hal/omci_hal.h"
#include <cstdio>
#include <cstring>

static mts_olt2000_omci_entity_t g_entities[MTS_OLT2000_OMCI_MAX_ENTITIES];
static uint32_t g_num_entities = 0;
static mts_olt2000_omci_status_t g_omci_status;

int mts_olt2000_omci_init(void) {
    memset(&g_omci_status, 0, sizeof(g_omci_status));
    memset(g_entities, 0, sizeof(g_entities));
    g_omci_status.status = MTS_OLT2000_OMCI_ENTITY_ACTIVE;
    return 0;
}

void mts_olt2000_omci_cleanup(void) {
    g_num_entities = 0;
    memset(&g_omci_status, 0, sizeof(g_omci_status));
}

int mts_olt2000_omci_get_status(mts_olt2000_omci_status_t *status) {
    if (!status) return -1;
    *status = g_omci_status;
    return 0;
}

int mts_olt2000_omci_get_entity(uint32_t entity_id, mts_olt2000_omci_entity_t *entity) {
    if (!entity || entity_id >= g_num_entities) return -1;
    *entity = g_entities[entity_id];
    return 0;
}

int mts_olt2000_omci_get_all_entities(mts_olt2000_omci_entity_t *entities, int max_entities) {
    if (!entities || max_entities <= 0) return -1;
    int count = (max_entities < (int)g_num_entities) ? max_entities : (int)g_num_entities;
    for (int i = 0; i < count; i++) {
        entities[i] = g_entities[i];
    }
    return count;
}

int mts_olt2000_omci_create_entity(mts_olt2000_omci_entity_type_t type, mts_olt2000_omci_entity_t *entity) {
    if (!entity || g_num_entities >= MTS_OLT2000_OMCI_MAX_ENTITIES) return -1;
    uint32_t id = g_num_entities;
    memset(entity, 0, sizeof(*entity));
    entity->entity_id = id;
    entity->type = type;
    entity->status = MTS_OLT2000_OMCI_ENTITY_ACTIVE;
    snprintf(entity->name, sizeof(entity->name), "entity-%u", id);
    g_entities[id] = *entity;
    g_num_entities++;
    g_omci_status.total_entities = g_num_entities;
    g_omci_status.active_entities++;
    return 0;
}

int mts_olt2000_omci_delete_entity(uint32_t entity_id) {
    if (entity_id >= g_num_entities) return -1;
    for (uint32_t i = entity_id; i < g_num_entities - 1; i++) {
        g_entities[i] = g_entities[i + 1];
    }
    g_num_entities--;
    g_omci_status.total_entities = g_num_entities;
    g_omci_status.active_entities--;
    return 0;
}
