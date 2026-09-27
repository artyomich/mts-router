/**
 * MTS-OLT-2000 ONU HAL — C++ Wrapper
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include "hal/onu_hal.h"

namespace mts::olt2000::hal {

class OnuHal {
public:
    OnuHal();
    ~OnuHal();

    bool discover(uint32_t pon_port, std::vector<mts_olt2000_onu_t>& onus);
    bool activate(uint32_t pon_port, const std::string& serial, mts_olt2000_onu_t& onu);
    bool deactivate(const std::string& onu_id);
    bool getTlv(const std::string& onu_id, uint16_t type, void* value, uint32_t& len);
    bool setTlv(const std::string& onu_id, uint16_t type, const void* value, uint32_t len);
    bool isAvailable();

    void setMockMode(bool enabled);

private:
    bool available_;
    std::atomic<bool> mock_mode_;
    mutable std::mutex mutex_;
};

} // namespace mts::olt2000::hal
