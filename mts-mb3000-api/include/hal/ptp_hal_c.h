#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MTS_MB_PTP_GRANDMASTER = 0,
    MTS_MB_PTP_BOUNDARY,
    MTS_MB_PTP_ORDINARY
} mts_mb_ptp_mode_t;

typedef enum {
    MTS_MB_PTP_ACTIVE = 0,
    MTS_MB_PTP_INACTIVE,
    MTS_MB_PTP_FAULT
} mts_mb_ptp_status_t;

typedef struct {
    char device_name[64];
    mts_mb_ptp_mode_t mode;
    int64_t current_time_ns;
    int64_t offset_from_master_ns;
    int64_t mean_path_delay_ns;
    double frequency_offset_ppm;
    mts_mb_ptp_status_t status;
} mts_mb_ptp_state_t;

int mts_mb_ptp_init(void);
void mts_mb_ptp_cleanup(void);
int mts_mb_ptp_get_status(mts_mb_ptp_status_t *status);
int mts_mb_ptp_set_mode(mts_mb_ptp_mode_t mode);
int mts_mb_ptp_set_grandmaster(const char *device_id);
int mts_mb_ptp_get_offset(int64_t *offset_ns);
int mts_mb_ptp_get_frequency_offset(double *offset_ppm);

#ifdef __cplusplus
}
#endif
