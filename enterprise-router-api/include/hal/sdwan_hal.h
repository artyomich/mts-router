/**
 * MTS-ER-1000 Enterprise Router — SD-WAN HAL
 * Hardware Abstraction Layer for SD-WAN path management
 * 
 * Responsibilities:
 * - Monitor WAN path health (latency, packet loss)
 * - Manage multipath routing via iproute2
 * - BFD session monitoring for fast failover
 * - keepalived HA state monitoring
 * - Policy-based routing for SD-WAN path selection
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

namespace mts::er1000::hal {

struct WanPath {
    std::string path_id;
    std::string wan_interface;
    std::string type;       // "mpls", "internet", "lte", "5g"
    std::string status;     // "active", "standby", "down"
    uint32_t priority;
    std::string qos_profile;
    std::string failover_interface;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    double latency_ms;
    double packet_loss_pct;
};

struct SdwanStatus {
    std::string controller_id;
    std::string status;     // "active", "inactive", "error"
    std::vector<WanPath> paths;
    uint32_t active_paths;
    uint32_t max_paths;
};

class ISdwanHal {
public:
    virtual ~ISdwanHal() = default;
    virtual SdwanStatus getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class SdwanHal : public ISdwanHal {
public:
    SdwanHal();
    ~SdwanHal() override = default;

    SdwanStatus getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool addPath(const WanPath& path);
    bool deletePath(const std::string& path_id);
    bool updatePath(const std::string& path_id, const std::string& config);

    void setMockMode(bool enabled);

private:
    bool updatePathStatistics();
    bool updateBfdSessions();
    bool updateKeepalivedState();
    std::vector<std::string> getWanInterfaces();
    bool checkBfdRunning();
    bool checkKeepalivedRunning();
    SdwanStatus applyMockStatus();

    SdwanStatus sdwan_status_;
    std::map<std::string, WanPath> paths_;
    std::vector<std::string> wan_interfaces_;
    bool bfd_available_;
    bool keepalived_available_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::er1000::hal
