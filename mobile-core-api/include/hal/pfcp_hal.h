/**
 * MTS-MC-5000 Mobile Core — PFCP HAL
 * Packet Forwarding Control Protocol hardware abstraction
 * 
 * Responsibilities:
 * - Monitor PFCP session state and statistics
 * - Manage PFCP steering rules (FAR, QER)
 * - Read session counters from /proc/net/pfcp
 * - Update conntrack for PFCP traffic
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

namespace mts::mc5000::hal {

struct PfcpRule {
    uint32_t rule_id;
    std::string description;
    std::string action;  // "forward", "drop", "buffer"
    uint32_t qos_index;
};

struct PfcpSession {
    std::string session_id;
    std::string f_seid;
    std::string peer_ip;
    std::string type;    // "upf", "smf"
    std::string status;  // "established", "inactive"
    std::vector<PfcpRule> rules;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

struct PfcpStatus {
    std::string device_name;
    std::string status;
    uint32_t total_sessions;
    uint32_t active_sessions;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
};

class IPfcpHal {
public:
    virtual ~IPfcpHal() = default;
    virtual std::vector<PfcpSession> getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class PfcpHal : public IPfcpHal {
public:
    PfcpHal();
    ~PfcpHal() override = default;

    std::vector<PfcpSession> getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool addSession(const PfcpSession& session);
    bool deleteSession(const std::string& session_id);
    bool createSteeringRule(const std::string& session_id, const PfcpRule& rule);
    bool deleteSteeringRule(const std::string& session_id, uint32_t rule_id);
    std::vector<PfcpRule> getSteeringRules(const std::string& session_id);

    void setMockMode(bool enabled);

private:
    bool readFromPfcpProc();
    bool updateConntrackSessions();
    bool loadSteeringRules();
    std::vector<PfcpSession> getSessionListInternal();
    std::vector<std::string> getSessionList();
    void applyMockData();

    PfcpStatus pfcp_status_;
    std::map<std::string, PfcpSession> sessions_;
    std::map<std::string, std::vector<PfcpRule>> steering_rules_;
    std::vector<std::string> session_list_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
    std::vector<PfcpSession> mock_sessions_;
};

} // namespace mts::mc5000::hal
