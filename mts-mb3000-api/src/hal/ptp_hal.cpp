/**
 * MTS-MB-3000 PTP HAL Implementation
 * Implements Precision Time Protocol (1588v2) management
 */

#include "hal/ptp_hal.h"
#include <iostream>
#include <cstring>
#include <mutex>
#include <chrono>
#include <ctime>

// Internal state
static std::mutex ptp_mutex;
static bool ptp_initialized = false;

// PTP state
static struct {
    char device_name[64];
    mts_mb_ptp_mode_t mode;
    int64_t current_time_ns;
    int64_t offset_from_master_ns;
    int64_t mean_path_delay_ns;
    double frequency_offset_ppm;
    mts_mb_ptp_status_t status;
} g_ptp_state = {
    .device_name = "ptp4l",
    .mode = MTS_MB_PTP_GRANDMASTER,
    .current_time_ns = 0,
    .offset_from_master_ns = 0,
    .mean_path_delay_ns = 0,
    .frequency_offset_ppm = 0.0,
    .status = MTS_MB_PTP_ACTIVE
};

extern "C" {

int mts_mb_ptp_init(void) {
    std::lock_guard<std::mutex> lock(ptp_mutex);
    
    if (ptp_initialized) {
        return 0;
    }
    
    // Initialize PTP state
    strncpy(g_ptp_state.device_name, "ptp4l", sizeof(g_ptp_state.device_name) - 1);
    g_ptp_state.mode = MTS_MB_PTP_GRANDMASTER;
    g_ptp_state.status = MTS_MB_PTP_INACTIVE;
    
    ptp_initialized = true;
    std::cout << "[PTP HAL] Initialized (linuxptp)" << std::endl;
    return 0;
}

void mts_mb_ptp_cleanup(void) {
    std::lock_guard<std::mutex> lock(ptp_mutex);
    
    ptp_initialized = false;
    g_ptp_state.status = MTS_MB_PTP_INACTIVE;
    std::cout << "[PTP HAL] Cleanup completed" << std::endl;
}

int mts_mb_ptp_get_status(mts_mb_ptp_status_t *status) {
    if (!status) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(ptp_mutex);
    
    strncpy(status->device_name, g_ptp_state.device_name, sizeof(status->device_name) - 1);
    status->mode = g_ptp_state.mode;
    status->current_time_ns = g_ptp_state.current_time_ns;
    status->offset_from_master_ns = g_ptp_state.offset_from_master_ns;
    status->mean_path_delay_ns = g_ptp_state.mean_path_delay_ns;
    status->frequency_offset_ppm = g_ptp_state.frequency_offset_ppm;
    status->status = g_ptp_state.status;
    
    return 0;
}

int mts_mb_ptp_set_mode(mts_mb_ptp_mode_t mode) {
    if (mode != MTS_MB_PTP_GRANDMASTER && 
        mode != MTS_MB_PTP_BOUNDARY && 
        mode != MTS_MB_PTP_ORDINARY) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(ptp_mutex);
    
    g_ptp_state.mode = mode;
    g_ptp_state.status = MTS_MB_PTP_ACTIVE;
    
    std::cout << "[PTP HAL] Mode set to: " 
              << (mode == MTS_MB_PTP_GRANDMASTER ? "GRANDMASTER" : 
                  mode == MTS_MB_PTP_BOUNDARY ? "BOUNDARY" : "ORDINARY") << std::endl;
    
    return 0;
}

int mts_mb_ptp_set_grandmaster(const char *device_id) {
    if (!device_id) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(ptp_mutex);
    
    g_ptp_state.mode = MTS_MB_PTP_GRANDMASTER;
    g_ptp_state.status = MTS_MB_PTP_ACTIVE;
    
    std::cout << "[PTP HAL] Set grandmaster: " << device_id << std::endl;
    
    return 0;
}

int mts_mb_ptp_get_offset(int64_t *offset_ns) {
    if (!offset_ns) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(ptp_mutex);
    *offset_ns = g_ptp_state.offset_from_master_ns;
    
    return 0;
}

int mts_mb_ptp_get_frequency_offset(double *offset_ppm) {
    if (!offset_ppm) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(ptp_mutex);
    *offset_ppm = g_ptp_state.frequency_offset_ppm;
    
    return 0;
}

} // extern "C"
