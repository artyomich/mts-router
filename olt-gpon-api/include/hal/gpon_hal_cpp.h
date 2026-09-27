/**
 * MTS-OLT-2000 GPON HAL — C++ Wrapper
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include "hal/gpon_hal.h"

namespace mts::olt2000::hal {

struct PonPortInfo {
    uint32_t port_id = 0;
    std::string status;
    double rx_power_dbm = 0.0;
    double tx_power_dbm = 0.0;
    double temperature_c = 0.0;
    uint32_t active_onu = 0;
    uint32_t max_onu = 0;
};

struct OnuInfo {
    std::string onu_id;
    uint32_t pon_port = 0;
    std::string status;
    std::string serial_number;
    std::string mac_address;
    std::string firmware_version;
    int32_t power_level_dbm = 0;
    uint32_t distance_m = 0;
    uint32_t vlan = 0;
    std::string qos_profile;
    uint32_t bandwidth_up_mbps = 0;
    uint32_t bandwidth_down_mbps = 0;
    uint64_t rx_bytes = 0;
    uint64_t tx_bytes = 0;
};

struct OltStatus {
    std::string olt_id;
    std::string status;
    std::string firmware_version;
    uint32_t total_onu = 0;
    uint32_t online_onu = 0;
    uint32_t pon_ports = 0;
    uint64_t rx_bytes = 0;
    uint64_t tx_bytes = 0;
};

class GponHal {
public:
    GponHal();
    ~GponHal();

    OltStatus getStatus();
    std::vector<PonPortInfo> getPonPorts();
    std::vector<OnuInfo> getOnuList();
    bool configureOnu(const std::string& onu_id, uint32_t pon_port, uint32_t vlan,
                      const std::string& qos_profile, uint32_t bw_up, uint32_t bw_down);
    bool resetOnu(const std::string& onu_id);
    bool isAvailable();

    void setMockMode(bool enabled);

private:
    bool checkInitialized();
    OltStatus applyMockStatus();
    std::vector<PonPortInfo> applyMockPonPorts();
    std::vector<OnuInfo> applyMockOnuList();

    bool available_;
    std::atomic<bool> mock_mode_;
    mutable std::mutex mutex_;
};

} // namespace mts::olt2000::hal
