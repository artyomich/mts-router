#pragma once
/**
 * MTS-CR-9000 Core Router — Line Card HAL
 * Hardware Abstraction Layer for Intel Tofino 2 line cards
 *
 * Responsibilities:
 * - Monitor line card status (8 slots)
 * - Read port statistics from sysfs/procfs
 * - Configure ASIC via P4Runtime
 * - Thread-safe with mock mode for testing
 */

#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace mts::cr9000::hal {

struct PortStatus {
    std::string name;
    std::string type;     // "qsfp28", "sfp+", etc.
    std::string status;   // "up", "down", "error"
    uint32_t speed_mbps;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
};

struct LineCardStatus {
    uint32_t card_id;
    std::string asic_type;
    std::string status;   // "up", "down", "initializing"
    std::vector<PortStatus> ports;
    uint32_t active_sessions;
    uint64_t packets_forwarded;
    uint64_t bytes_forwarded;
    uint64_t errors;
};

class ILineCardHal {
public:
    virtual ~ILineCardHal() = default;
    virtual LineCardStatus getStatus(uint32_t card_id) = 0;
    virtual std::vector<LineCardStatus> getAllCardStatus() = 0;
    virtual bool isAvailable(uint32_t card_id) = 0;
    virtual std::string getDeviceName(uint32_t card_id) = 0;
};

class LineCardHal : public ILineCardHal {
public:
    LineCardHal();
    ~LineCardHal() override = default;

    LineCardStatus getStatus(uint32_t card_id) override;
    std::vector<LineCardStatus> getAllCardStatus() override;
    bool isAvailable(uint32_t card_id) override;
    std::string getDeviceName(uint32_t card_id) override;

    // Set mock mode for testing
    void setMockMode(bool enabled);
    void setMockPorts(const std::vector<PortStatus>& ports);

private:
    std::vector<PortStatus> readPortStatsFromSysfs();
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_{false};
    std::vector<PortStatus> mock_ports_;
};

} // namespace mts::cr9000::hal
