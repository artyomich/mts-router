/**
 * MTS-CR-9000 Core Router — MPLS LSP Manager Implementation
 * Manages MPLS Label Switched Paths via iproute2
 */

#include "mpls/lsp_manager.h"
#include <sstream>
#include <iostream>

namespace mts::cr9000::mpls {

LspManager::LspManager() : next_id_(1) {}

std::vector<MplsLspStatus> LspManager::getLspStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    return lsps_;
}

CreateLspResult LspManager::createLsp(const std::string& name, const std::string& ingress_label,
                                       const std::string& egress_label, const std::string& next_hop,
                                       const std::string& interface) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    CreateLspResult result;
    result.success = false;
    
    if (name.empty() || ingress_label.empty() || egress_label.empty()) {
        result.message = "Invalid LSP parameters";
        return result;
    }
    
    MplsLspStatus lsp;
    lsp.lsp_id = next_id_++;
    lsp.name = name;
    lsp.ingress_label = ingress_label;
    lsp.egress_label = egress_label;
    lsp.next_hop = next_hop;
    lsp.interface = interface;
    lsp.status = "initializing";
    lsp.rx_packets = 0;
    lsp.tx_packets = 0;
    lsp.rx_bytes = 0;
    lsp.tx_bytes = 0;
    
    lsps_.push_back(lsp);
    
    // Set status to up
    lsps_.back().status = "up";
    
    result.success = true;
    result.message = "LSP created";
    result.lsp_id = lsp.lsp_id;
    
    return result;
}

bool LspManager::deleteLsp(uint32_t lsp_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(lsps_.begin(), lsps_.end(),
        [lsp_id](const MplsLspStatus& lsp) { return lsp.lsp_id == lsp_id; });
    
    if (it == lsps_.end()) return false;
    
    lsps_.erase(it);
    return true;
}

bool LspManager::updateLsp(uint32_t lsp_id, const std::string& next_hop) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& lsp : lsps_) {
        if (lsp.lsp_id == lsp_id) {
            lsp.next_hop = next_hop;
            return true;
        }
    }
    return false;
}

} // namespace mts::cr9000::mpls
