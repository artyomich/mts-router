/**
 * MTS-ER-1000 SD-WAN Path Manager — SD-WAN path monitoring
 * Provides mock implementation for SD-WAN WAN path management
 */

#include "hal/sdwan_hal.h"
#include <cstdio>
#include <cstring>

namespace mts::er1000::hal {

SdwanHal::SdwanHal() : mock_mode_(false), bfd_available_(true), keepalived_available_(true), available_(true) {
    memset(&sdwan_status_, 0, sizeof(sdwan_status_));
    sdwan_status_.controller_id = "MTS-ER-1000-SD-WAN";
    sdwan_status_.status = "active";
    sdwan_status_.active_paths = 0;
    sdwan_status_.max_paths = 16;
}

SdwanStatus SdwanHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockStatus();
    }
    return sdwan_status_;
}

bool SdwanHal::isAvailable() {
    return available_;
}

std::string SdwanHal::getDeviceName() {
    return "MTS-ER-1000-SD-WAN";
}

bool SdwanHal::addPath(const WanPath& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (paths_.size() >= sdwan_status_.max_paths) return false;
    paths_[path.path_id] = path;
    sdwan_status_.active_paths++;
    return true;
}

bool SdwanHal::deletePath(const std::string& path_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = paths_.find(path_id);
    if (it == paths_.end()) return false;
    paths_.erase(it);
    sdwan_status_.active_paths--;
    return true;
}

bool SdwanHal::updatePath(const std::string& path_id, const std::string& config) {
    (void)path_id;
    (void)config;
    return true;
}

void SdwanHal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

bool SdwanHal::updatePathStatistics() { return true; }
bool SdwanHal::updateBfdSessions() { return bfd_available_; }
bool SdwanHal::updateKeepalivedState() { return keepalived_available_; }
std::vector<std::string> SdwanHal::getWanInterfaces() { return {"eth0", "eth1", "eth2"}; }
bool SdwanHal::checkBfdRunning() { return bfd_available_; }
bool SdwanHal::checkKeepalivedRunning() { return keepalived_available_; }

SdwanStatus SdwanHal::applyMockStatus() {
    SdwanStatus mock;
    mock.controller_id = "MOCK-SDWAN-001";
    mock.status = "active";
    mock.active_paths = 3;
    mock.max_paths = 16;

    WanPath path1, path2, path3;
    path1.path_id = "path-001";
    path1.wan_interface = "eth0";
    path1.type = "mpls";
    path1.status = "active";
    path1.priority = 100;
    path1.qos_profile = "high";
    path1.latency_ms = 5.0;
    path1.packet_loss_pct = 0.0;

    path2.path_id = "path-002";
    path2.wan_interface = "eth1";
    path2.type = "internet";
    path2.status = "standby";
    path2.priority = 50;
    path2.qos_profile = "medium";
    path2.latency_ms = 20.0;
    path2.packet_loss_pct = 0.1;

    path3.path_id = "path-003";
    path3.wan_interface = "eth2";
    path3.type = "lte";
    path3.status = "down";
    path3.priority = 10;
    path3.qos_profile = "low";
    path3.latency_ms = 50.0;
    path3.packet_loss_pct = 1.0;

    mock.paths.push_back(path1);
    mock.paths.push_back(path2);
    mock.paths.push_back(path3);
    return mock;
}

} // namespace mts::er1000::hal
