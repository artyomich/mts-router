/**
 * MTS-MC-5000 Mobile Core — SMF HAL
 * Session Management Function hardware abstraction
 * 
 * Responsibilities:
 * - Monitor SMF session management state
 * - Read DNS configuration from kernel
 * - Track PGW connectivity
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <cstdint>

namespace mts::mc5000::hal {

struct SmfConfig {
    std::string smf_id;
    std::string status;     // "active", "inactive", "error"
    uint32_t max_sessions;
    uint32_t current_sessions;
    std::string dns_primary;
    std::string dns_secondary;
    std::string pgw_ip;
    int64_t last_updated;  // timestamp in nanoseconds
};

struct InterfaceStats {
    std::string name;
    uint64_t rx_bytes;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t tx_packets;
};

class ISmfHal {
public:
    virtual ~ISmfHal() = default;
    virtual SmfConfig getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class SmfHal : public ISmfHal {
public:
    SmfHal();
    ~SmfHal() override = default;

    SmfConfig getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool updateDnsConfig(const std::string& primary_dns,
                         const std::string& secondary_dns);
    bool updatePgwAddress(const std::string& pgw_ip);

    void setMockMode(bool enabled);

private:
    bool readFromProcNet();
    void applyMockData();

    SmfConfig smf_config_;
    std::vector<InterfaceStats> interface_stats_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::mc5000::hal
