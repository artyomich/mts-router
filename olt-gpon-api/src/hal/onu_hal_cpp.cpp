/**
 * MTS-OLT-2000 ONU HAL — C++ Wrapper Implementation
 */

#include "hal/onu_hal_cpp.h"
#include <iostream>

namespace mts::olt2000::hal {

OnuHal::OnuHal() : available_(true), mock_mode_(false) {
    std::cout << "[OnuHal] Constructed" << std::endl;
}

OnuHal::~OnuHal() {
    std::cout << "[OnuHal] Destructed" << std::endl;
}

bool OnuHal::discover(uint32_t pon_port, std::vector<mts_olt2000_onu_t>& onus) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* ops = mts_olt2000_onu_get_ops();
    if (!ops || !ops->discover) return false;
    return ops->discover(pon_port, nullptr, nullptr) >= 0;
}

bool OnuHal::activate(uint32_t pon_port, const std::string& serial, mts_olt2000_onu_t& onu) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* ops = mts_olt2000_onu_get_ops();
    if (!ops || !ops->activate) return false;
    return ops->activate(pon_port, serial.c_str(), &onu) >= 0;
}

bool OnuHal::deactivate(const std::string& onu_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* ops = mts_olt2000_onu_get_ops();
    if (!ops || !ops->deactivate) return false;
    return ops->deactivate(onu_id.c_str()) >= 0;
}

bool OnuHal::getTlv(const std::string& onu_id, uint16_t type, void* value, uint32_t& len) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* ops = mts_olt2000_onu_get_ops();
    if (!ops || !ops->get_tlv) return false;
    return ops->get_tlv(onu_id.c_str(), &type, value, &len) >= 0;
}

bool OnuHal::setTlv(const std::string& onu_id, uint16_t type, const void* value, uint32_t len) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* ops = mts_olt2000_onu_get_ops();
    if (!ops || !ops->set_tlv) return false;
    return ops->set_tlv(onu_id.c_str(), type, value, len) >= 0;
}

bool OnuHal::isAvailable() {
    return available_;
}

void OnuHal::setMockMode(bool enabled) {
    mock_mode_.store(enabled);
}

} // namespace mts::olt2000::hal
