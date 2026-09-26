/**
 * MTS-ER-1000 Enterprise Router — IPsec HAL
 * Hardware Abstraction Layer for IPsec tunnel management
 * 
 * Responsibilities:
 * - Monitor IPsec SA state from /proc/net/xfrm_state
 * - Read IPsec policy from /proc/net/xfrm_policy
 * - Manage IPsec tunnels via ipsecctl
 * - Monitor strongSwan/libreswan daemon
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>
#include <cstdint>
#include <cstring>

namespace mts::er1000::hal {

struct IpsecTunnelStatus {
    std::string tunnel_id;
    std::string name;
    std::string peer_ip;
    std::string local_subnet;
    std::string remote_subnet;
    std::string mode;     // "tunnel", "transport"
    std::string status;   // "up", "down", "negotiating"
    uint32_t phase;       // 1, 2
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    int64_t established_at;
};

class IIpsecHal {
public:
    virtual ~IIpsecHal() = default;
    virtual std::vector<IpsecTunnelStatus> getTunnelStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class IpsecHal : public IIpsecHal {
public:
    IpsecHal();
    ~IpsecHal() override = default;

    std::vector<IpsecTunnelStatus> getTunnelStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool createTunnel(const IpsecTunnelStatus& tunnel);
    bool deleteTunnel(const std::string& tunnel_id);

    void setMockMode(bool enabled);

private:
    bool checkIpsecDaemonRunning();
    uint32_t readSaCount();
    uint32_t readPolicyCount();
    bool readSaFromProc();
    bool updateSaCounters();
    std::vector<IpsecTunnelStatus> getTunnelStatusInternal();
    std::vector<IpsecTunnelStatus> applyMockTunnels();

    std::map<std::string, IpsecTunnelStatus> tunnels_;
    uint32_t sa_count_;
    uint32_t policy_count_;
    bool ipsec_daemon_available_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::er1000::hal
