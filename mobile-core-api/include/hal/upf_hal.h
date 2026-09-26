/**
 * MTS-MC-5000 Mobile Core — UPF HAL
 * User Plane Function hardware abstraction
 * 
 * Responsibilities:
 * - Monitor UPF session count and throughput
 * - Read GTP tunnel statistics from kernel
 * - Manage PFCP sessions
 * - Track conntrack entries for GTP tunnels
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>
#include <chrono>
#include <cstdint>

namespace mts::mc5000::hal {

struct UpfStatus {
    std::string upf_id;
    std::string status;     // "active", "inactive", "error"
    uint32_t active_sessions;
    uint32_t max_sessions;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    double cpu_usage;
    double memory_usage;
    int64_t last_updated;   // timestamp in nanoseconds
};

struct PduSession {
    std::string session_id;
    std::string ue_ip;
    std::string upf_ip;
    uint32_t teid;
    uint32_t qfi;
    uint32_t five_qi;
    std::string pnni;
    std::string status;     // "active", "inactive", "releasing"
    int64_t created_at;
    int64_t last_active;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

struct PfcpSession {
    std::string session_id;
    std::string f_seid;
    std::string peer_ip;
    std::string type;       // "upf", "smf"
    std::string status;     // "established", "inactive"
};

struct GtpTunnel {
    std::string tunnel_id;
    std::string local_ip;
    std::string remote_ip;
    uint32_t local_teid;
    uint32_t remote_teid;
    std::string type;       // "gtp-u", "gtp-c"
    std::string status;     // "active", "inactive"
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

struct ConntrackEntry {
    std::string src_ip;
    std::string dst_ip;
    uint32_t proto_src;
    uint32_t proto_dst;
    uint32_t mark;
};

struct InterfaceStats {
    std::string name;
    uint64_t rx_bytes;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t tx_packets;
};

class IUpfHal {
public:
    virtual ~IUpfHal() = default;
    virtual UpfStatus getStatus() = 0;
    virtual std::vector<PduSession> getPduSessions() = 0;
    virtual bool createPduSession(const PduSession& session) = 0;
    virtual bool deletePduSession(const std::string& session_id) = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class UpfHal : public IUpfHal {
public:
    UpfHal();
    ~UpfHal() override = default;

    UpfStatus getStatus() override;
    std::vector<PduSession> getPduSessions() override;
    bool createPduSession(const PduSession& session) override;
    bool deletePduSession(const std::string& session_id) override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    std::vector<std::string> getInterfaceList();

    void setMockMode(bool enabled);
    void setMockSessions(const std::vector<PduSession>& sessions);

private:
    bool readCpuUsage();
    bool readMemoryUsage();
    bool readInterfaceStats();
    std::vector<ConntrackEntry> readConntrackTable();
    void applyMockData();

    UpfStatus upf_status_;
    std::map<std::string, PduSession> sessions_;
    std::vector<ConntrackEntry> conntrack_entries_;
    std::vector<std::string> interface_list_;
    std::vector<InterfaceStats> interface_stats_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
    std::vector<PduSession> mock_sessions_;
};

} // namespace mts::mc5000::hal
