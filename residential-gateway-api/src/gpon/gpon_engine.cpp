/**
 * MTS-RG-500 GPON Engine — GPON ONU monitoring implementation
 * Provides mock implementation for GPON ONU status
 */

#include "hal/gpon_hal.h"
#include <cstdio>
#include <cstring>

namespace mts::rg500::hal {

GponHal::GponHal() : mock_mode_(false) {
    memset(&gpon_status_, 0, sizeof(gpon_status_));
    gpon_status_.onu_id = "ONU-001";
    gpon_status_.status = "online";
    gpon_status_.power_level = -20;
    gpon_status_.distance = 1000;
    gpon_status_.pon_port = "PON-0";
    gpon_status_.vlan = 100;
    gpon_status_.rx_bytes = 0;
    gpon_status_.tx_bytes = 0;
}

GponOnuStatus GponHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockStatus();
    }
    return gpon_status_;
}

bool GponHal::isAvailable() {
    return gpon_status_.status == "online";
}

std::string GponHal::getDeviceName() {
    return "MTS-RG-500-GPON";
}

void GponHal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

GponOnuStatus GponHal::applyMockStatus() {
    GponOnuStatus mock;
    mock.onu_id = "MOCK-ONU-001";
    mock.status = "online";
    mock.power_level = -18;
    mock.distance = 800;
    mock.pon_port = "PON-0";
    mock.vlan = 100;
    mock.rx_bytes = 1024000;
    mock.tx_bytes = 512000;
    return mock;
}

} // namespace mts::rg500::hal
