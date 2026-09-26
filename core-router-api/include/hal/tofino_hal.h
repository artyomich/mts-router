/**
 * MTS-CR-9000 Core Router — Tofino 2 HAL
 * Hardware Abstraction Layer for Intel Tofino 2 Barefoot TNA ASIC
 * 
 * Responsibilities:
 * - Monitor Tofino 2 pipeline state and utilization
 * - Read per-port statistics from /proc/net
 * - Manage P4 pipeline via bfrt_cli and P4 Runtime
 * - Track table entries and rule utilization
 * - Monitor Tofino 2 temperature
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>
#include <cstdint>
#include <cstring>

namespace mts::cr9000::hal {

struct PortStats {
    std::string name;
    uint64_t rx_bytes;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
};

struct TofinoStatus {
    std::string device_id;
    std::string status;     // "active", "inactive", "error"
    uint32_t total_ports;
    uint32_t active_ports;
    uint32_t total_rules;
    uint32_t active_rules;
    double temperature;
    uint32_t pipeline_depth;
    double table_utilization;
};

class ITofinoHal {
public:
    virtual ~ITofinoHal() = default;
    virtual TofinoStatus getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class TofinoHal : public ITofinoHal {
public:
    TofinoHal();
    ~TofinoHal() override = default;

    TofinoStatus getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool loadPipeline(const std::string& p4_program);
    bool updateTableEntry(const std::string& table_name,
                          const std::string& entry);
    bool deleteTableEntry(const std::string& table_name,
                          const std::string& key);

    void setMockMode(bool enabled);

private:
    bool readPortStatistics();
    bool readTableUtilization();
    bool readTemperature();
    bool readP4RuntimeEntries();
    std::vector<std::string> getPortListFromSysfs();
    bool checkP4RuntimeRunning();
    std::string findTemperatureSource();
    TofinoStatus applyMockStatus();

    TofinoStatus tofino_status_;
    std::map<std::string, PortStats> port_stats_;
    std::vector<std::string> port_list_;
    bool p4rt_available_;
    std::string temperature_source_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::cr9000::hal
