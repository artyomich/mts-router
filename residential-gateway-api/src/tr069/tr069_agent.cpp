/**
 * MTS-RG-500 TR-069 Agent — TR-069 (CWMP) agent monitoring
 * Provides mock implementation for TR-069 ACS connection state
 */

#include "hal/tr069_hal.h"
#include <cstdio>
#include <cstring>
#include <chrono>

namespace mts::rg500::hal {

Tr069Hal::Tr069Hal() : mock_mode_(false), available_(true), cwmpd_running_(true) {
    memset(&tr069_status_, 0, sizeof(tr069_status_));
    tr069_status_.device_id = "MTS-RG-500-TR069";
    tr069_status_.url = "acs.mts-router.com:3695";
    tr069_status_.enabled = true;
    tr069_status_.polling_interval = 600;
    tr069_status_.status = "active";
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    tr069_status_.last_poll = now;
    tr069_status_.next_poll = now + 600;
}

Tr069Status Tr069Hal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockStatus();
    }
    return tr069_status_;
}

bool Tr069Hal::isAvailable() {
    return cwmpd_running_ && available_;
}

std::string Tr069Hal::getDeviceName() {
    return "MTS-RG-500-TR069-CWMP";
}

void Tr069Hal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

bool Tr069Hal::monitorCwmpd() {
    return cwmpd_running_;
}

bool Tr069Hal::updateTcpConnections() {
    return true;
}

bool Tr069Hal::updateFromCwmpd() {
    return true;
}

Tr069Status Tr069Hal::applyMockStatus() {
    Tr069Status mock;
    mock.device_id = "MOCK-TR069-001";
    mock.url = "acs-mock.mts-router.com:3695";
    mock.enabled = true;
    mock.polling_interval = 600;
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    mock.last_poll = now;
    mock.next_poll = now + 600;
    mock.status = "active";
    return mock;
}

} // namespace mts::rg500::hal
