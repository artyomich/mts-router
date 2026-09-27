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
    mts_mb_ptp_status_code_t status;
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

namespace mts::hal {

PtpHal::PtpHal() : mock_mode_(false) {
    mts_mb_ptp_init();
    std::cout << "[PtpHal] Constructed" << std::endl;
}

PtpHal::~PtpHal() {
    mts_mb_ptp_cleanup();
    std::cout << "[PtpHal] Destructed" << std::endl;
}

PtpStatus PtpHal::getStatus() {
    std::lock_guard<std::mutex> lock(ptp_mutex);
    PtpStatus s;
    mts_mb_ptp_status_t c;
    mts_mb_ptp_get_status(&c);
    s.device_name = c.device_name;
    s.mode = (c.mode == MTS_MB_PTP_GRANDMASTER) ? "grandmaster" :
             (c.mode == MTS_MB_PTP_BOUNDARY) ? "boundary" : "ordinary";
    s.current_time = c.current_time_ns;
    s.offset_from_master = c.offset_from_master_ns;
    s.mean_path_delay = c.mean_path_delay_ns;
    s.frequency_offset = c.frequency_offset_ppm;
    s.status = (c.status == MTS_MB_PTP_ACTIVE) ? "active" :
               (c.status == MTS_MB_PTP_FAULT) ? "fault" : "inactive";
    s.phase_offset = 0.0;
    return s;
}

bool PtpHal::setGrandmasterMode(bool enable) {
    if (enable) {
        return mts_mb_ptp_set_grandmaster("gm-001") == 0;
    }
    return mts_mb_ptp_set_mode(MTS_MB_PTP_BOUNDARY) == 0;
}

bool PtpHal::isAvailable() {
    return true;
}

std::string PtpHal::getDeviceName() {
    return "ptp4l";
}

void PtpHal::setMockMode(bool enable) {
    mock_mode_ = enable;
}

void PtpHal::applyMockData() {
    // No-op for mock mode
}

} // namespace mts::hal
