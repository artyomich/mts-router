/**
 * MTS-CR-9000 Core Router — MPLS LSP Manager
 * Manages MPLS Label Switched Paths
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>

namespace mts::cr9000::mpls {

struct MplsLspStatus {
    uint32_t lsp_id;
    std::string name;
    std::string ingress_label;
    std::string egress_label;
    std::string next_hop;
    std::string interface;
    std::string status;  // "up", "down", "initializing"
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

struct CreateLspResult {
    bool success;
    std::string message;
    uint32_t lsp_id;
};

class LspManager {
public:
    LspManager();
    ~LspManager() = default;

    std::vector<MplsLspStatus> getLspStatus();
    CreateLspResult createLsp(const std::string& name, const std::string& ingress_label,
                               const std::string& egress_label, const std::string& next_hop,
                               const std::string& interface);
    bool deleteLsp(uint32_t lsp_id);
    bool updateLsp(uint32_t lsp_id, const std::string& next_hop);

private:
    std::mutex mutex_;
    std::vector<MplsLspStatus> lsps_;
    uint32_t next_id_;
};

} // namespace mts::cr9000::mpls
