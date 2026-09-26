/**
 * MTS-OLT-2000 OLT GPON — GPON HAL
 * Hardware Abstraction Layer for GPON PON ports
 * 
 * Responsibilities:
 * - Monitor 16 PON port status (optical power, utilization)
 * - Read temperature and voltage from sysfs
 * - Manage ONU registration/deregistration
 * - Track ONU statistics per PON port
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>
#include <chrono>
#include <cstdint>
#include <cstring>

namespace mts::olt2000::hal {

struct PonPortInfo {
    std::string pon_id;
    std::string name;
    std::string status;     // "up", "down", "error"
    uint32_t num_onu;
    uint32_t max_onu;
    double downstream_rate;
    double upstream_rate;
    double downstream_util;
    double upstream_util;
    double optical_power;
};

struct OnuInfo {
    std::string onu_id;
    std::string serial;
    std::string mac;
    std::string pon_port;
    std::string status;     // "online", "offline", "error"
    int32_t power_level;    // dBm
    int32_t distance;       // meters
    uint32_t vlan;
    std::string qos_profile;
    uint32_t bandwidth_up;     // kbps
    uint32_t bandwidth_down;   // kbps
    int64_t last_seen;
    int64_t created;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

struct OltStatus {
    std::string device_id;
    std::string status;     // "active", "degraded", "offline"
    uint32_t total_onu;
    uint32_t online_onu;
    uint32_t offline_onu;
    uint32_t error_onu;
    double temperature;
    double voltage;
    uint64_t uptime_seconds;
};

class IGponHal {
public:
    virtual ~IGponHal() = default;
    virtual OltStatus getStatus() = 0;
    virtual std::vector<PonPortInfo> getPonPorts() = 0;
    virtual std::vector<OnuInfo> getOnuList() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class GponHal : public IGponHal {
public:
    GponHal();
    ~GponHal() override = default;

    OltStatus getStatus() override;
    std::vector<PonPortInfo> getPonPorts() override;
    std::vector<OnuInfo> getOnuList() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool registerOnu(const OnuInfo& onu);
    bool deregisterOnu(const std::string& onu_id);
    bool updateOnuConfig(const std::string& onu_id, const std::string& config);

    void setMockMode(bool enabled);
    void setMockOnuList(const std::vector<OnuInfo>& onus);

private:
    bool readTemperature();
    bool readVoltage();
    bool readUptime();
    bool updateOnuCount();
    bool readPonPortStatus();
    bool updateOnuPerPort();
    bool readOnuListFromSysfs();
    bool updateOnuStatistics();
    std::vector<std::string> getPonPortList();
    std::vector<std::string> getOnuListFromSysfs();
    std::vector<OnuInfo> getOnuListInternal();
    void applyMockData();

    OltStatus olt_status_;
    std::map<std::string, OnuInfo> onus_;
    std::vector<std::string> pon_port_list_;
    std::vector<std::string> onu_list_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
    std::vector<OnuInfo> mock_onu_list_;
    std::vector<PonPortInfo> mock_pon_ports_;
};

} // namespace mts::olt2000::hal
