/**
 * MTS-ER-1000 Enterprise Router — MPLS HAL
 * Hardware Abstraction Layer for MPLS LSP management
 * 
 * Responsibilities:
 * - Monitor MPLS LSP state from /proc/net/mpls
 * - Manage MPLS labels via iproute2
 * - Monitor FRRouting (zebra/bgpd) daemon
 * - Track LSP counters and forwarding state
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

struct MplsLspStatus {
    uint32_t lsp_id;
    std::string name;
    std::string ingress_label;
    std::string egress_label;
    std::string next_hop;
    std::string interface;
    std::string status;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

class IMplsHal {
public:
    virtual ~IMplsHal() = default;
    virtual std::vector<MplsLspStatus> getLspStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class MplsHal : public IMplsHal {
public:
    MplsHal();
    ~MplsHal() override = default;

    std::vector<MplsLspStatus> getLspStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool createLsp(const MplsLspStatus& lsp);
    bool deleteLsp(uint32_t lsp_id);

    void setMockMode(bool enabled);

private:
    bool checkFrrRunning();
    uint32_t readLspCount();
    bool readLspFromProc();
    bool updateLspCounters();
    std::vector<MplsLspStatus> getLspStatusInternal();
    std::vector<MplsLspStatus> applyMockLspStatus();

    std::map<uint32_t, MplsLspStatus> lsps_;
    uint32_t lsp_count_;
    bool frr_available_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::er1000::hal
