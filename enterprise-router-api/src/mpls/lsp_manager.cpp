/**
 * MTS-ER-1000 MPLS LSP Manager — MPLS LSP monitoring
 * Provides mock implementation for MPLS LSP state tracking
 */

#include "hal/mpls_hal.h"
#include <cstdio>
#include <cstring>

namespace mts::er1000::hal {

MplsHal::MplsHal() : mock_mode_(false), frr_available_(true), available_(true) {
    memset(&lsp_count_, 0, sizeof(lsp_count_));
}

std::vector<MplsLspStatus> MplsHal::getLspStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockLspStatus();
    }
    return getLspStatusInternal();
}

bool MplsHal::isAvailable() {
    return frr_available_ && available_;
}

std::string MplsHal::getDeviceName() {
    return "MTS-ER-1000-MPLS";
}

bool MplsHal::createLsp(const MplsLspStatus& lsp) {
    std::lock_guard<std::mutex> lock(mutex_);
    lsps_[lsp.lsp_id] = lsp;
    lsp_count_++;
    return true;
}

bool MplsHal::deleteLsp(uint32_t lsp_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = lsps_.find(lsp_id);
    if (it == lsps_.end()) return false;
    lsps_.erase(it);
    lsp_count_--;
    return true;
}

void MplsHal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

bool MplsHal::checkFrrRunning() {
    return frr_available_;
}

uint32_t MplsHal::readLspCount() {
    return lsp_count_;
}

bool MplsHal::readLspFromProc() {
    return true;
}

bool MplsHal::updateLspCounters() {
    return true;
}

std::vector<MplsLspStatus> MplsHal::getLspStatusInternal() {
    std::vector<MplsLspStatus> lsps;
    for (const auto& pair : lsps_) {
        lsps.push_back(pair.second);
    }
    return lsps;
}

std::vector<MplsLspStatus> MplsHal::applyMockLspStatus() {
    std::vector<MplsLspStatus> lsps;

    MplsLspStatus lsp1, lsp2;
    lsp1.lsp_id = 1;
    lsp1.name = "LSP-001";
    lsp1.ingress_label = "16";
    lsp1.egress_label = "24";
    lsp1.next_hop = "10.0.0.2";
    lsp1.interface = "eth0";
    lsp1.status = "up";
    lsp1.rx_packets = 1000000;
    lsp1.tx_packets = 950000;
    lsp1.rx_bytes = 150000000;
    lsp1.tx_bytes = 142500000;

    lsp2.lsp_id = 2;
    lsp2.name = "LSP-002";
    lsp2.ingress_label = "17";
    lsp2.egress_label = "25";
    lsp2.next_hop = "10.0.0.3";
    lsp2.interface = "eth1";
    lsp2.status = "up";
    lsp2.rx_packets = 500000;
    lsp2.tx_packets = 475000;
    lsp2.rx_bytes = 75000000;
    lsp2.tx_bytes = 71250000;

    lsps.push_back(lsp1);
    lsps.push_back(lsp2);
    return lsps;
}

} // namespace mts::er1000::hal
