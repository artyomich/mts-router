/**
 * MTS-OLT-2000 ONU HAL — ONU management
 */

#ifndef MTS_OLT2000_ONU_HAL_H
#define MTS_OLT2000_ONU_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "gpon_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ONU management operations */
typedef struct {
    int (*init)(void);
    int (*cleanup)(void);
    int (*discover)(uint32_t pon_port, mts_olt2000_onu_t **onus, uint32_t *count);
    int (*activate)(uint32_t pon_port, const char *serial, mts_olt2000_onu_t *onu);
    int (*deactivate)(const char *onu_id);
    int (*get_tlv)(const char *onu_id, uint16_t *type, void *value, uint32_t *len);
    int (*set_tlv)(const char *onu_id, uint16_t type, const void *value, uint32_t len);
} mts_olt2000_onu_ops_t;

/**
 * Get ONU management operations
 * @return Pointer to ONU operations structure
 */
const mts_olt2000_onu_ops_t *mts_olt2000_onu_get_ops(void);

/**
 * Register ONU management operations
 * @param ops Pointer to operations structure
 * @return 0 on success, -1 on error
 */
int mts_olt2000_onu_register_ops(const mts_olt2000_onu_ops_t *ops);

#ifdef __cplusplus
}
#endif

#endif /* MTS_OLT2000_ONU_HAL_H */
