/**
 * MTS-ER-1000 IPsec Tunnel Manager — IPsec tunnel monitoring
 * Provides mock implementation for IPsec SA and tunnel state tracking
 */

#include "hal/ipsec_hal.h"
#include <cstdio>
#include <cstring>
#include <chrono>

namespace mts::er1000::hal {

IpsecHal::IpsecHal() : mock_mode_(false), ipsec_daemon_available_(true), available_(true) {
    memset(&sa_count_, 0, sizeof(sa_count_));
    memset(&policy_count_, 0, sizeof(policy_count_));
}

std::vector<IpsecTunnelStatus> IpsecHal::getTunnelStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockTunnels();
    }
    return getTunnelStatusInternal();
}

bool IpsecHal::isAvailable() {
    return ipsec_daemon_available_ && available_;
}

std::string IpsecHal::getDeviceName() {
    return "MTS-ER-1000-IPsec";
}

bool IpsecHal::createTunnel(const IpsecTunnelStatus& tunnel) {
    std::lock_guard<std::mutex> lock(mutex_);
    tunnels_[tunnel.tunnel_id] = tunnel;
    sa_count_++;
    return true;
}

bool IpsecHal::deleteTunnel(const std::string& tunnel_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tunnels_.find(tunnel_id);
    if (it == tunnels_.end()) return false;
    tunnels_.erase(it);
    sa_count_--;
    return true;
}

void IpsecHal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

bool IpsecHal::checkIpsecDaemonRunning() {
    return ipsec_daemon_available_;
}

uint32_t IpsecHal::readSaCount() {
    return sa_count_;
}

uint32_t IpsecHal::readPolicyCount() {
    return policy_count_;
}

bool IpsecHal::readSaFromProc() {
    return true;
}

bool IpsecHal::updateSaCounters() {
    return true;
}

std::vector<IpsecTunnelStatus> IpsecHal::getTunnelStatusInternal() {
    std::vector<IpsecTunnelStatus> tunnels;
    for (const auto& pair : tunnels_) {
        tunnels.push_back(pair.second);
    }
    return tunnels;
}

std::vector<IpsecTunnelStatus> IpsecHal::applyMockTunnels() {
    std::vector<IpsecTunnelStatus> tunnels;

    IpsecTunnelStatus tunnel1, tunnel2;
    tunnel1.tunnel_id = "tunnel-001";
    tunnel1.name = "IPsec-001";
    tunnel1.peer_ip = "203.0.113.1";
    tunnel1.local_subnet = "10.0.0.0/24";
    tunnel1.remote_subnet = "192.168.1.0/24";
    tunnel1.mode = "tunnel";
    tunnel1.status = "up";
    tunnel1.phase = 2;
    tunnel1.rx_bytes = 50000000;
    tunnel1.tx_bytes = 45000000;
    tunnel1.rx_packets = 500000;
    tunnel1.tx_packets = 450000;
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    tunnel1.established_at = now - 3600;

    tunnel2.tunnel_id = "tunnel-002";
    tunnel2.name = "IPsec-002";
    tunnel2.peer_ip = "198.51.100.1";
    tunnel2.local_subnet = "10.0.1.0/24";
    tunnel2.remote_subnet = "172.16.0.0/24";
    tunnel2.mode = "tunnel";
    tunnel2.status = "negotiating";
    tunnel2.phase = 1;
    tunnel2.rx_bytes = 0;
    tunnel2.tx_bytes = 0;
    tunnel2.rx_packets = 0;
    tunnel2.tx_packets = 0;
    tunnel2.established_at = 0;

    tunnels.push_back(tunnel1);
    tunnels.push_back(tunnel2);
    return tunnels;
}

} // namespace mts::er1000::hal
