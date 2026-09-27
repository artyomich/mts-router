/**
 * MTS-MB-3000 SyncE HAL Implementation
 * Implements Synchronous Ethernet management
 */

#include "hal/sync_e_hal.h"
#include <iostream>
#include <cstring>
#include <mutex>

// Internal state
static std::mutex sync_e_mutex;
static bool sync_e_initialized = false;

// SyncE port state
static struct {
    char port_name[32];
    mts_mb_synce_mode_t mode;
    int32_t target_frequency;
    double actual_frequency;
    double phase_offset_ns;
    mts_mb_synce_status_t status;
} g_synce_ports[MTS_MB_SYNCE_MAX_PORTS] = {
    {
        .port_name = "eth0",
        .mode = MTS_MB_SYNCE_MASTER,
        .target_frequency = 156250000,
        .actual_frequency = 156250000.0,
        .phase_offset_ns = 0.0,
        .status = MTS_MB_SYNCE_LOCKED
    },
    {
        .port_name = "eth1",
        .mode = MTS_MB_SYNCE_SLAVE,
        .target_frequency = 156250000,
        .actual_frequency = 156249999.5,
        .phase_offset_ns = -50.0,
        .status = MTS_MB_SYNCE_UNLOCKED
    }
};

extern "C" {

int mts_mb_synce_init(void) {
    std::lock_guard<std::mutex> lock(sync_e_mutex);
    
    if (sync_e_initialized) {
        return 0;
    }
    
    sync_e_initialized = true;
    std::cout << "[SyncE HAL] Initialized" << std::endl;
    return 0;
}

void mts_mb_synce_cleanup(void) {
    std::lock_guard<std::mutex> lock(sync_e_mutex);
    
    sync_e_initialized = false;
    std::cout << "[SyncE HAL] Cleanup completed" << std::endl;
}

int mts_mb_synce_get_port_status(const char *port_name, mts_mb_synce_port_t *status) {
    if (!port_name || !status) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(sync_e_mutex);
    
    for (int i = 0; i < MTS_MB_SYNCE_MAX_PORTS; i++) {
        if (strcmp(g_synce_ports[i].port_name, port_name) == 0) {
            strncpy(status->port_name, g_synce_ports[i].port_name, sizeof(status->port_name) - 1);
            status->mode = g_synce_ports[i].mode;
            status->target_frequency = g_synce_ports[i].target_frequency;
            status->actual_frequency = g_synce_ports[i].actual_frequency;
            status->phase_offset_ns = g_synce_ports[i].phase_offset_ns;
            status->status = g_synce_ports[i].status;
            return 0;
        }
    }
    
    return -1; // Port not found
}

int mts_mb_synce_get_all_ports(mts_mb_synce_port_t *ports, int max_ports) {
    if (!ports || max_ports <= 0) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(sync_e_mutex);
    
    int count = 0;
    for (int i = 0; i < MTS_MB_SYNCE_MAX_PORTS && count < max_ports; i++) {
        if (g_synce_ports[i].port_name[0] != '\0') {
            ports[count].port_name[0] = '\0';
            strncpy(ports[count].port_name, g_synce_ports[i].port_name, sizeof(ports[count].port_name) - 1);
            ports[count].mode = g_synce_ports[i].mode;
            ports[count].target_frequency = g_synce_ports[i].target_frequency;
            ports[count].actual_frequency = g_synce_ports[i].actual_frequency;
            ports[count].phase_offset_ns = g_synce_ports[i].phase_offset_ns;
            ports[count].status = g_synce_ports[i].status;
            count++;
        }
    }
    
    return count;
}

int mts_mb_synce_set_mode(const char *port_name, mts_mb_synce_mode_t mode) {
    if (!port_name) {
        return -1;
    }
    
    if (mode != MTS_MB_SYNCE_MASTER && 
        mode != MTS_MB_SYNCE_SLAVE && 
        mode != MTS_MB_SYNCE_TRANSPARENT) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(sync_e_mutex);
    
    for (int i = 0; i < MTS_MB_SYNCE_MAX_PORTS; i++) {
        if (strcmp(g_synce_ports[i].port_name, port_name) == 0) {
            g_synce_ports[i].mode = mode;
            return 0;
        }
    }
    
    return -1; // Port not found
}

} // extern "C"

namespace mts::hal {

SyncEHal::SyncEHal() : available_(true) {
    std::cout << "[SyncEHal] Constructed" << std::endl;
}

SyncEHal::~SyncEHal() {
    std::cout << "[SyncEHal] Destructed" << std::endl;
}

std::vector<SyncEStatus> SyncEHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SyncEStatus> result;
    for (int i = 0; i < MTS_MB_SYNCE_MAX_PORTS; i++) {
        SyncEStatus s;
        s.port_name = port_list_[i];
        s.mode = "master";
        s.frequency = 156250000;
        s.actual_frequency = 156250000.0;
        s.phase_offset = 0.0;
        s.status = "locked";
        result.push_back(s);
    }
    return result;
}

bool SyncEHal::setMode(const std::string& port_name, const std::string& mode) {
    return true;
}

bool SyncEHal::isAvailable() {
    return available_;
}

std::vector<std::string> SyncEHal::getPortList() {
    std::lock_guard<std::mutex> lock(mutex_);
    return port_list_;
}

} // namespace mts::hal
