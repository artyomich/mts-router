/**
 * MTS-CR-9000 Core Router — Port HAL
 * Hardware Abstraction Layer for port monitoring
 */

#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace mts::cr9000::hal {

struct PortStats {
    std::string name;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
};

class IPortHal {
public:
    virtual ~IPortHal() = default;
    virtual std::vector<PortStats> getStats() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class PortHal : public IPortHal {
public:
    PortHal();
    ~PortHal() override = default;

    std::vector<PortStats> getStats() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    void setMockMode(bool enabled);
    void setMockStats(const std::vector<PortStats>& stats);

private:
    std::vector<PortStats> readFromSysfs();
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    std::vector<PortStats> mock_stats_;
};

} // namespace mts::cr9000::hal
