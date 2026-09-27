/**
 * MTS-OLT-2000 TR-069 HAL — C++ Wrapper Implementation
 */

#include "hal/tr069_hal_cpp.h"
#include <iostream>
#include <ctime>
#include <cstring>

namespace mts::olt2000::hal {

Tr069Hal::Tr069Hal() : available_(true), mock_mode_(false) {
    std::cout << "[Tr069Hal] Constructed" << std::endl;
}

Tr069Hal::~Tr069Hal() {
    std::cout << "[Tr069Hal] Destructed" << std::endl;
}

Tr069Config Tr069Hal::getConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (mock_mode_.load()) {
        Tr069Config config;
        config.enabled = true;
        config.acs_url = "http://acs.example.com:7547";
        config.polling_enabled = true;
        config.polling_interval = 300;
        config.username = "admin";
        config.status = "active";
        config.last_session_id = 1;
        config.last_bootstrap = std::time(nullptr);
        config.next_bootstrap = config.last_bootstrap + 300;
        return config;
    }
    
    mts_olt2000_tr069_config_t c_config;
    if (mts_olt2000_tr069_get_config(&c_config) != 0) {
        Tr069Config config;
        config.status = "error";
        return config;
    }
    
    Tr069Config config;
    config.enabled = c_config.enabled;
    config.acs_url = std::string(c_config.acs_url);
    config.polling_enabled = c_config.polling_enabled;
    config.polling_interval = c_config.polling_interval;
    config.username = std::string(c_config.username);
    config.status = (c_config.status == MTS_OLT2000_TR069_ACTIVE) ? "active" : "inactive";
    config.last_session_id = c_config.last_session_id;
    config.last_bootstrap = c_config.last_bootstrap;
    config.next_bootstrap = c_config.next_bootstrap;
    return config;
}

bool Tr069Hal::setConfig(const Tr069Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    mts_olt2000_tr069_config_t c_config = {};
    c_config.enabled = config.enabled;
    strncpy(c_config.acs_url, config.acs_url.c_str(), sizeof(c_config.acs_url) - 1);
    c_config.acs_url[sizeof(c_config.acs_url) - 1] = '\0';
    c_config.polling_enabled = config.polling_enabled;
    c_config.polling_interval = config.polling_interval;
    strncpy(c_config.username, config.username.c_str(), sizeof(c_config.username) - 1);
    c_config.username[sizeof(c_config.username) - 1] = '\0';
    return mts_olt2000_tr069_set_config(&c_config) >= 0;
}

bool Tr069Hal::enable() {
    std::lock_guard<std::mutex> lock(mutex_);
    return mts_olt2000_tr069_enable() >= 0;
}

bool Tr069Hal::disable() {
    std::lock_guard<std::mutex> lock(mutex_);
    return mts_olt2000_tr069_disable() >= 0;
}

bool Tr069Hal::triggerInform() {
    std::lock_guard<std::mutex> lock(mutex_);
    return mts_olt2000_tr069_trigger_inform() >= 0;
}

bool Tr069Hal::isAvailable() {
    return available_;
}

void Tr069Hal::setMockMode(bool enabled) {
    mock_mode_.store(enabled);
}

} // namespace mts::olt2000::hal
