/**
 * MTS-ER-1000 Enterprise Router — VRRP HAL
 * Hardware Abstraction Layer for VRRP monitoring
 * 
 * Responsibilities:
 * - Monitor VRRP instance state from keepalived
 * - Track master/backup instances per interface
 * - Read virtual IP state via ip link
 * - Monitor keepalived daemon health
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

namespace mts::er1000::hal {

struct VrrpStatus {
    std::string interface;
    uint32_t virtual_router_id;
    std::string status;     // "master", "backup", "init"
    double priority;
    std::string master_ip;
    double preempt_delay;
};

struct VrrpGlobalStatus {
    std::string device_id;
    std::string status;
    uint32_t total_instances;
    uint32_t master_instances;
    uint32_t backup_instances;
};

class IVrrpHal {
public:
    virtual ~IVrrpHal() = default;
    virtual std::vector<VrrpStatus> getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class VrrpHal : public IVrrpHal {
public:
    VrrpHal();
    ~VrrpHal() override = default;

    std::vector<VrrpStatus> getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    void setMockMode(bool enabled);

private:
    bool readVrrpStateFromKeepalived();
    bool updateVrrpStatistics();
    std::vector<std::string> getVrrpInstances();
    bool checkKeepalivedRunning();
    std::vector<VrrpStatus> getVrrpStatusInternal();
    std::vector<VrrpStatus> applyMockStatus();

    VrrpGlobalStatus vrrp_status_;
    std::vector<std::string> vrrp_instances_;
    bool keepalived_available_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::er1000::hal
