/**
 * MTS-OLT-2000 TR-069 HAL — TR-069 (CWMP) management
 */

#ifndef MTS_OLT2000_TR069_HAL_H
#define MTS_OLT2000_TR069_HAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MTS_OLT2000_TR069_MAX_URL 256
#define MTS_OLT2000_TR069_MAX_USERNAME 64
#define MTS_OLT2000_TR069_MAX_PASSWORD 64

/* TR-069 status */
typedef enum {
    MTS_OLT2000_TR069_ACTIVE = 0,
    MTS_OLT2000_TR069_INACTIVE,
    MTS_OLT2000_TR069_ERROR
} mts_olt2000_tr069_status_t;

/* TR-069 configuration */
typedef struct {
    bool enabled;
    char acs_url[MTS_OLT2000_TR069_MAX_URL];
    bool polling_enabled;
    uint32_t polling_interval; /* seconds */
    char username[MTS_OLT2000_TR069_MAX_USERNAME];
    char password[MTS_OLT2000_TR069_MAX_PASSWORD];
    mts_olt2000_tr069_status_t status;
    uint32_t last_session_id;
    int64_t last_bootstrap; /* timestamp */
    int64_t next_bootstrap; /* timestamp */
} mts_olt2000_tr069_config_t;

/* ==================== API ==================== */

/**
 * Initialize TR-069 HAL
 * @return 0 on success, -1 on error
 */
int mts_olt2000_tr069_init(void);

/**
 * Cleanup TR-069 HAL
 */
void mts_olt2000_tr069_cleanup(void);

/**
 * Get TR-069 configuration
 * @param config Output buffer for TR-069 config
 * @return 0 on success, -1 on error
 */
int mts_olt2000_tr069_get_config(mts_olt2000_tr069_config_t *config);

/**
 * Set TR-069 configuration
 * @param config TR-069 config to set
 * @return 0 on success, -1 on error
 */
int mts_olt2000_tr069_set_config(const mts_olt2000_tr069_config_t *config);

/**
 * Enable TR-069
 * @return 0 on success, -1 on error
 */
int mts_olt2000_tr069_enable(void);

/**
 * Disable TR-069
 * @return 0 on success, -1 on error
 */
int mts_olt2000_tr069_disable(void);

/**
 * Trigger ACS inform
 * @return 0 on success, -1 on error
 */
int mts_olt2000_tr069_trigger_inform(void);

#ifdef __cplusplus
}
#endif

#endif /* MTS_OLT2000_TR069_HAL_H */
