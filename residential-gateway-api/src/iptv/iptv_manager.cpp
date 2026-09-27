/**
 * MTS-RG-500 IPTV Manager — IPTV multicast monitoring
 * Provides mock implementation for IPTV channel tracking
 */

#include "hal/iptv_hal.h"
#include <cstdio>
#include <cstring>

namespace mts::rg500::hal {

IptvHal::IptvHal() : mock_mode_(false), igmp_available_(true), available_(true) {
    memset(&iptv_status_, 0, sizeof(iptv_status_));
    iptv_status_.device_id = "MTS-RG-500-IPTV";
    iptv_status_.status = "active";
    iptv_status_.active_channels = 0;
    iptv_status_.total_channels = 50;
    iptv_status_.bandwidth_mbps = 0.0;
}

IptvStatus IptvHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockStatus();
    }
    return iptv_status_;
}

bool IptvHal::isAvailable() {
    return igmp_available_ && available_;
}

std::string IptvHal::getDeviceName() {
    return "MTS-RG-500-IPTV";
}

bool IptvHal::subscribeChannel(const std::string& multicast_ip, uint32_t multicast_port) {
    (void)multicast_ip;
    (void)multicast_port;
    return true;
}

bool IptvHal::unsubscribeChannel(const std::string& multicast_ip) {
    (void)multicast_ip;
    return true;
}

void IptvHal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

bool IptvHal::checkIgmpProxyRunning() {
    return igmp_available_;
}

std::vector<std::string> IptvHal::getMulticastGroupsFromProc() {
    return {"239.1.1.1", "239.1.1.2", "239.1.1.3"};
}

bool IptvHal::readMulticastGroups() { return true; }
bool IptvHal::readMulticastStats() { return true; }
double IptvHal::calculateBandwidth() { return 15.0; }

IptvStatus IptvHal::applyMockStatus() {
    IptvStatus mock;
    mock.device_id = "MOCK-IPTV-001";
    mock.status = "active";
    mock.active_channels = 3;
    mock.total_channels = 50;
    mock.bandwidth_mbps = 15.0;

    IptvChannel ch1, ch2, ch3;
    ch1.channel_id = 1;
    ch1.name = "Channel-001";
    ch1.multicast_ip = "239.1.1.1";
    ch1.multicast_port = 5004;
    ch1.status = "active";
    ch1.viewers = 2;

    ch2.channel_id = 2;
    ch2.name = "Channel-002";
    ch2.multicast_ip = "239.1.1.2";
    ch2.multicast_port = 5006;
    ch2.status = "active";
    ch2.viewers = 1;

    ch3.channel_id = 3;
    ch3.name = "Channel-003";
    ch3.multicast_ip = "239.1.1.3";
    ch3.multicast_port = 5008;
    ch3.status = "active";
    ch3.viewers = 3;

    mock.channels.push_back(ch1);
    mock.channels.push_back(ch2);
    mock.channels.push_back(ch3);
    return mock;
}

} // namespace mts::rg500::hal
