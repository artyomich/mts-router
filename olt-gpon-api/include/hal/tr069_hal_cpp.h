/**
 * MTS-OLT-2000 TR-069 HAL — C++ Wrapper
 */

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include "hal/tr069_hal.h"

namespace mts::olt2000::hal {

struct Tr069Config {
    bool enabled = false;
    std::string acs_url;
    bool polling_enabled = false;
    uint32_t polling_interval = 300;
    std::string username;
    std::string password;
    std::string status;
    uint32_t last_session_id = 0;
    int64_t last_bootstrap = 0;
    int64_t next_bootstrap = 0;
};

class Tr069Hal {
public:
    Tr069Hal();
    ~Tr069Hal();

    Tr069Config getConfig();
    bool setConfig(const Tr069Config& config);
    bool enable();
    bool disable();
    bool triggerInform();
    bool isAvailable();

    void setMockMode(bool enabled);

private:
    bool available_;
    std::atomic<bool> mock_mode_;
    mutable std::mutex mutex_;
};

} // namespace mts::olt2000::hal
