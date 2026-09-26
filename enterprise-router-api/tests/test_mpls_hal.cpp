/**
 * MTS-ER-1000 — MPLS HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/mpls_hal.h"

using namespace mts::er1000::hal;

TEST(MplsHalTest, DefaultConstructor) {
    MplsHal hal;
    auto lsps = hal.getLspStatus();
    EXPECT_TRUE(lsps.empty());
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-ER-1000-MPLS");
}

TEST(MplsHalTest, CreateLsp) {
    MplsHal hal;
    
    MplsLspStatus lsp;
    lsp.lsp_id = 1;
    lsp.name = "lsp-001";
    lsp.ingress_label = "16";
    lsp.egress_label = "24";
    lsp.next_hop = "192.168.1.1";
    lsp.interface = "eth0";
    lsp.status = "up";
    lsp.rx_packets = 0;
    lsp.tx_packets = 0;
    lsp.rx_bytes = 0;
    lsp.tx_bytes = 0;
    
    auto created = hal.createLsp(lsp);
    EXPECT_TRUE(created);
    
    auto lsps = hal.getLspStatus();
    EXPECT_EQ(lsps.size(), 1);
    EXPECT_EQ(lsps[0].name, "lsp-001");
}

TEST(MplsHalTest, DeleteLsp) {
    MplsHal hal;
    
    MplsLspStatus lsp;
    lsp.lsp_id = 1;
    lsp.name = "lsp-001";
    lsp.ingress_label = "16";
    lsp.egress_label = "24";
    lsp.next_hop = "192.168.1.1";
    lsp.interface = "eth0";
    lsp.status = "up";
    lsp.rx_packets = 0;
    lsp.tx_packets = 0;
    lsp.rx_bytes = 0;
    lsp.tx_bytes = 0;
    
    hal.createLsp(lsp);
    auto deleted = hal.deleteLsp(1);
    EXPECT_TRUE(deleted);
    
    auto lsps = hal.getLspStatus();
    EXPECT_TRUE(lsps.empty());
}
