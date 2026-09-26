/**
 * MTS-OLT-2000 OLT GPON — ONU HAL
 * Hardware Abstraction Layer for ONU management
 * 
 * Responsibilities:
 * - Monitor ONU configuration (VLAN, QoS, bandwidth)
 * - Read ONU config from sysfs
 * - Apply bandwidth/QoS via rtl_gpon CLI and SNMP
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>

namespace mts::olt2000::hal {

struct OnuConfig {
    std::string onu_id;
    std::string pon_port;
    uint32_t vlan;
    std::string qos_profile;
    uint32_t bandwidth_up;
    uint32_t bandwidth_down;
};

class IOnuHal {
public:
    virtual ~IOnuHal() = default;
    virtual std::vector<OnuConfig> getConfigs() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class OnuHal : public IOnuHal {
public:
    OnuHal();
    ~OnuHal() override = default;

    std::vector<OnuConfig> getConfigs() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool setConfig(const OnuConfig& config);
    bool deleteConfig(const std::string& onu_id);

    void setMockMode(bool enabled);

private:
    bool loadConfigsFromSysfs();
    std::vector<OnuConfig> readConfigsFromSysfs();
    std::vector<OnuConfig> applyMockConfigs();

    std::map<std::string, OnuConfig> configs_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::olt2000::hal
