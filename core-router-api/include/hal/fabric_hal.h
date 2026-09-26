#pragma once
/**
 * MTS-CR-9000 Core Router — Fabric HAL
 * Hardware Abstraction Layer for Intel Tofino 2 fabric crossbar
 * 
 * Responsibilities:
 * - Monitor fabric slot status (8 slots)
 * - Read ASIC telemetry via sysfs/procfs
 * - Configure crossbar via P4Runtime
 * - Thread-safe with mock mode for testing
 */

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>
#include <chrono>

namespace mts::cr9000::hal {

struct SlotStatus {
    uint32_t slot_id;
    std::string card_type;  // "line-card", "control-card", "fabric-card"
    std::string status;     // "up", "down", "initializing"
    double cpu_usage;
    double memory_usage;
};

struct FabricStatus {
    std::string name;
    uint32_t num_slots;
    std::vector<SlotStatus> slots;
    double total_bandwidth_gbps;
    std::string status;  // "active", "degraded", "offline"
};

class IFabricHal {
public:
    virtual ~IFabricHal() = default;
    virtual FabricStatus getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class FabricHal : public IFabricHal {
public:
    FabricHal();
    ~FabricHal() override = default;

    FabricStatus getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    // Set mock mode for testing
    void setMockMode(bool enabled);
    void setMockSlots(const std::vector<SlotStatus>& slots);

private:
    std::vector<SlotStatus> readSlotsFromSysfs();
    double readCpuUsage();
    double readMemoryUsage();

    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_{false};
    std::vector<SlotStatus> mock_slots_;
    std::chrono::steady_clock::time_point last_read_;
};

} // namespace mts::cr9000::hal
