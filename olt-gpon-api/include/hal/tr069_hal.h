/**
 * MTS-OLT-2000 OLT GPON — TR-069 HAL
 * CWMP (CPE WAN Management Protocol) agent monitoring
 * 
 * Responsibilities:
 * - Monitor TR-069 ACS connection state
 * - Read TCP connections to ACS from /proc/net/tcp
 * - Parse cwmpd logs for event tracking
 * - Update ACS URL configuration
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <cstdint>

namespace mts::olt2000::hal {

struct Tr069Config {
    std::string device_id;
    std::string url;
    std::string username;
    bool enabled;
    uint32_t polling_interval;
    int64_t last_poll;
    int64_t next_poll;
};

class ITr069Hal {
public:
    virtual ~ITr069Hal() = default;
    virtual Tr069Config getConfig() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class Tr069Hal : public ITr069Hal {
public:
    Tr069Hal();
    ~Tr069Hal() override = default;

    Tr069Config getConfig() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool setUrl(const std::string& url);

    void setMockMode(bool enabled);

private:
    bool monitorCwmpd();
    bool updateFromCwmpd();
    bool updateTcpConnections();
    void applyMockData();

    std::string acs_url_;
    Tr069Config tr069_config_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::olt2000::hal
