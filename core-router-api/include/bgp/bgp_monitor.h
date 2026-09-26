/**
 * MTS-CR-9000 Core Router — BGP Monitor
 * Monitors BGP neighbor status and routing table
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace mts::cr9000::bgp {

struct BgpNeighbor {
    std::string peer;
    uint32_t peer_as;
    std::string state;  // "idle", "connect", "active", "opensent", "openconfirm", "established"
    uint32_t prefixes_received;
    uint32_t prefixes_sent;
};

struct BgpStatus {
    std::string device_id;
    uint32_t local_as;
    std::string router_id;
    std::string state;
    std::vector<BgpNeighbor> neighbors;
    uint32_t prefixes_received;
    uint32_t prefixes_sent;
};

class BgpMonitor {
public:
    BgpMonitor();
    ~BgpMonitor() = default;

    BgpStatus getStatus();
    bool addNeighbor(const std::string& peer, uint32_t peer_as);
    bool deleteNeighbor(const std::string& peer);
    bool updateNeighbor(const std::string& peer, const std::string& config);
    std::vector<BgpNeighbor> getNeighbors();

private:
    std::mutex mutex_;
    std::vector<BgpNeighbor> neighbors_;
};

} // namespace mts::cr9000::bgp
