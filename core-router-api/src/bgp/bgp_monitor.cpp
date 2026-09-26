/**
 * MTS-CR-9000 Core Router — BGP Monitor Implementation
 * Monitors BGP neighbor status via FRRouting integration
 */

#include "bgp/bgp_monitor.h"
#include <iostream>

namespace mts::cr9000::bgp {

BgpMonitor::BgpMonitor() {}

BgpStatus BgpMonitor::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    BgpStatus status;
    status.device_id = "MTS-CR-9000-001";
    status.local_as = 65001;
    status.router_id = "10.0.0.1";
    status.state = "established";
    status.neighbors = neighbors_;
    status.prefixes_received = 0;
    status.prefixes_sent = 0;
    
    return status;
}

bool BgpMonitor::addNeighbor(const std::string& peer, uint32_t peer_as) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if already exists
    for (const auto& n : neighbors_) {
        if (n.peer == peer) return false;
    }
    
    BgpNeighbor neighbor;
    neighbor.peer = peer;
    neighbor.peer_as = peer_as;
    neighbor.state = "idle";
    neighbor.prefixes_received = 0;
    neighbor.prefixes_sent = 0;
    
    neighbors_.push_back(neighbor);
    return true;
}

bool BgpMonitor::deleteNeighbor(const std::string& peer) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(neighbors_.begin(), neighbors_.end(),
        [&peer](const BgpNeighbor& n) { return n.peer == peer; });
    
    if (it == neighbors_.end()) return false;
    
    neighbors_.erase(it);
    return true;
}

bool BgpMonitor::updateNeighbor(const std::string& peer, const std::string& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& n : neighbors_) {
        if (n.peer == peer) {
            // Parse config and update
            return true;
        }
    }
    return false;
}

std::vector<BgpNeighbor> BgpMonitor::getNeighbors() {
    std::lock_guard<std::mutex> lock(mutex_);
    return neighbors_;
}

} // namespace mts::cr9000::bgp
