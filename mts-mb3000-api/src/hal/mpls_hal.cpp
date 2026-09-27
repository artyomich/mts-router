/**
 * MTS-MB-3000 MPLS-TP HAL Implementation
 * Implements MPLS-TP pseudowire forwarding
 */

#include "hal/mpls_hal.h"
#include <iostream>
#include <cstring>
#include <mutex>
#include <algorithm>

// Internal state
static std::mutex mpls_mutex;
static bool mpls_initialized = false;

// MPLS-TP pseudowire state
static struct {
    uint32_t pw_id;
    char ingress_port[32];
    char egress_port[32];
    mts_mb_mpls_encapsulation_t encapsulation;
    uint32_t qos_class;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    mts_mb_mpls_pw_status_code_t status;
} g_mpls_pws[MTS_MB_MPLS_MAX_PW] = {};

extern "C" {

int mts_mb_mpls_init(void) {
    std::lock_guard<std::mutex> lock(mpls_mutex);
    
    if (mpls_initialized) {
        return 0;
    }
    
    // Clear all pseudowires
    memset(g_mpls_pws, 0, sizeof(g_mpls_pws));
    
    mpls_initialized = true;
    std::cout << "[MPLS-TP HAL] Initialized" << std::endl;
    return 0;
}

void mts_mb_mpls_cleanup(void) {
    std::lock_guard<std::mutex> lock(mpls_mutex);
    
    mpls_initialized = false;
    memset(g_mpls_pws, 0, sizeof(g_mpls_pws));
    std::cout << "[MPLS-TP HAL] Cleanup completed" << std::endl;
}

int mts_mb_mpls_create_pw(uint32_t pw_id, const char *ingress_port, 
                          const char *egress_port, 
                          mts_mb_mpls_encapsulation_t encapsulation,
                          uint32_t qos_class) {
    if (!ingress_port || !egress_port) {
        return -1;
    }
    
    if (encapsulation != MTS_MB_MPLS_ETH && 
        encapsulation != MTS_MB_MPLS_HDLC && 
        encapsulation != MTS_MB_MPLS_UNSTRUCTURED) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(mpls_mutex);
    
    // Find empty slot
    for (int i = 0; i < MTS_MB_MPLS_MAX_PW; i++) {
        if (g_mpls_pws[i].pw_id == 0 || g_mpls_pws[i].status == MTS_MB_MPLS_PW_DOWN) {
            g_mpls_pws[i].pw_id = pw_id;
            strncpy(g_mpls_pws[i].ingress_port, ingress_port, sizeof(g_mpls_pws[i].ingress_port) - 1);
            strncpy(g_mpls_pws[i].egress_port, egress_port, sizeof(g_mpls_pws[i].egress_port) - 1);
            g_mpls_pws[i].encapsulation = encapsulation;
            g_mpls_pws[i].qos_class = qos_class;
            g_mpls_pws[i].status = MTS_MB_MPLS_PW_INITIALIZING;
            
            // Simulate activation
            g_mpls_pws[i].status = MTS_MB_MPLS_PW_UP;
            
            std::cout << "[MPLS-TP HAL] Created PW " << pw_id 
                      << " " << ingress_port << " -> " << egress_port << std::endl;
            return 0;
        }
    }
    
    return -1; // No empty slot
}

int mts_mb_mpls_delete_pw(uint32_t pw_id) {
    std::lock_guard<std::mutex> lock(mpls_mutex);
    
    for (int i = 0; i < MTS_MB_MPLS_MAX_PW; i++) {
        if (g_mpls_pws[i].pw_id == pw_id) {
            g_mpls_pws[i].status = MTS_MB_MPLS_PW_DOWN;
            std::cout << "[MPLS-TP HAL] Deleted PW " << pw_id << std::endl;
            return 0;
        }
    }
    
    return -1; // PW not found
}

int mts_mb_mpls_get_pw_status(uint32_t pw_id, mts_mb_mpls_pw_status_t *status) {
    if (!status) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(mpls_mutex);
    
    for (int i = 0; i < MTS_MB_MPLS_MAX_PW; i++) {
        if (g_mpls_pws[i].pw_id == pw_id) {
            status->pw_id = g_mpls_pws[i].pw_id;
            strncpy(status->ingress_port, g_mpls_pws[i].ingress_port, sizeof(status->ingress_port) - 1);
            strncpy(status->egress_port, g_mpls_pws[i].egress_port, sizeof(status->egress_port) - 1);
            status->encapsulation = g_mpls_pws[i].encapsulation;
            status->qos_class = g_mpls_pws[i].qos_class;
            status->rx_bytes = g_mpls_pws[i].rx_bytes;
            status->tx_bytes = g_mpls_pws[i].tx_bytes;
            status->rx_packets = g_mpls_pws[i].rx_packets;
            status->tx_packets = g_mpls_pws[i].tx_packets;
            status->rx_errors = g_mpls_pws[i].rx_errors;
            status->tx_errors = g_mpls_pws[i].tx_errors;
            status->status = g_mpls_pws[i].status;
            return 0;
        }
    }
    
    return -1; // PW not found
}

int mts_mb_mpls_get_all_pws(mts_mb_mpls_pw_status_t *pws, int max_pws) {
    if (!pws || max_pws <= 0) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(mpls_mutex);
    
    int count = 0;
    for (int i = 0; i < MTS_MB_MPLS_MAX_PW && count < max_pws; i++) {
        if (g_mpls_pws[i].pw_id != 0) {
            pws[count].pw_id = g_mpls_pws[i].pw_id;
            strncpy(pws[count].ingress_port, g_mpls_pws[i].ingress_port, sizeof(pws[count].ingress_port) - 1);
            strncpy(pws[count].egress_port, g_mpls_pws[i].egress_port, sizeof(pws[count].egress_port) - 1);
            pws[count].encapsulation = g_mpls_pws[i].encapsulation;
            pws[count].qos_class = g_mpls_pws[i].qos_class;
            pws[count].rx_bytes = g_mpls_pws[i].rx_bytes;
            pws[count].tx_bytes = g_mpls_pws[i].tx_bytes;
            pws[count].rx_packets = g_mpls_pws[i].rx_packets;
            pws[count].tx_packets = g_mpls_pws[i].tx_packets;
            pws[count].rx_errors = g_mpls_pws[i].rx_errors;
            pws[count].tx_errors = g_mpls_pws[i].tx_errors;
            pws[count].status = g_mpls_pws[i].status;
            count++;
        }
    }
    
    return count;
}

} // extern "C"

namespace mts::hal {

MplsTpHal::MplsTpHal() : available_(true) {
    mts_mb_mpls_init();
    std::cout << "[MplsTpHal] Constructed" << std::endl;
}

MplsTpHal::~MplsTpHal() {
    mts_mb_mpls_cleanup();
    std::cout << "[MplsTpHal] Destructed" << std::endl;
}

std::vector<MplsTpPwStatus> MplsTpHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MplsTpPwStatus> result;
    for (uint32_t id : pw_id_list_) {
        MplsTpPwStatus s;
        s.pw_id = id;
        s.ingress_port = "eth0";
        s.egress_port = "eth1";
        s.encapsulation = "eth";
        s.qos_class = 0;
        s.rx_bytes = 0;
        s.tx_bytes = 0;
        s.rx_packets = 0;
        s.tx_packets = 0;
        s.rx_errors = 0;
        s.tx_errors = 0;
        s.status = "up";
        result.push_back(s);
    }
    return result;
}

bool MplsTpHal::createPw(uint32_t pw_id, const std::string& ingress,
                         const std::string& egress, const std::string& encap,
                         uint32_t qos_class) {
    pw_id_list_.push_back(pw_id);
    return true;
}

bool MplsTpHal::deletePw(uint32_t pw_id) {
    auto it = std::find(pw_id_list_.begin(), pw_id_list_.end(), pw_id);
    if (it != pw_id_list_.end()) {
        pw_id_list_.erase(it);
        return true;
    }
    return false;
}

bool MplsTpHal::isAvailable() {
    return available_;
}

std::vector<uint32_t> MplsTpHal::getPwList() {
    std::lock_guard<std::mutex> lock(mutex_);
    return pw_id_list_;
}

} // namespace mts::hal
