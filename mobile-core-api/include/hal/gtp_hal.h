/**
 * MTS-MC-5000 Mobile Core — GTP HAL
 * GTP-U/GTP-C tunnel management
 * 
 * Responsibilities:
 * - Monitor GTP tunnel state and statistics
 * - Create/delete GTP tunnels via netlink
 * - Read tunnel counters from /proc/net/gtp
 * - Configure QoS per tunnel (QFI/5QI)
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

namespace mts::mc5000::hal {

struct GtpTunnel {
    std::string tunnel_id;
    std::string local_ip;
    std::string remote_ip;
    uint32_t local_teid;
    uint32_t remote_teid;
    std::string type;    // "gtp-u", "gtp-c"
    std::string status;  // "active", "inactive"
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

struct GtpStatus {
    std::string device_name;
    std::string status;
    uint32_t total_tunnels;
    uint32_t active_tunnels;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
};

class IGtpHal {
public:
    virtual ~IGtpHal() = default;
    virtual std::vector<GtpTunnel> getTunnels() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class GtpHal : public IGtpHal {
public:
    GtpHal();
    ~GtpHal() override = default;

    std::vector<GtpTunnel> getTunnels() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool createTunnel(const GtpTunnel& tunnel);
    bool deleteTunnel(const std::string& tunnel_id);
    bool setTunnelQos(const std::string& tunnel_id, uint32_t qfi,
                      uint32_t five_qi);

    void setMockMode(bool enabled);

private:
    bool readFromGtpProc();
    std::vector<std::string> getTunnelList();
    std::vector<GtpTunnel> getTunnelListInternal();
    void applyMockData();

    GtpStatus gtp_status_;
    std::map<std::string, GtpTunnel> tunnels_;
    std::vector<std::string> tunnel_list_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
    std::vector<GtpTunnel> mock_tunnels_;
};

} // namespace mts::mc5000::hal
