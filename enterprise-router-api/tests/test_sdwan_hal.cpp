/**
 * MTS-ER-1000 — SD-WAN HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/sdwan_hal.h"

using namespace mts::er1000::hal;

TEST(SdwanHalTest, DefaultConstructor) {
    SdwanHal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.controller_id, "MTS-SDWAN-CTRL-001");
    EXPECT_EQ(status.status, "active");
    EXPECT_EQ(status.max_paths, 256);
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-ER-1000-SDWAN");
}

TEST(SdwanHalTest, AddPath) {
    SdwanHal hal;
    
    WanPath path;
    path.path_id = "path-001";
    path.wan_interface = "eth1";
    path.type = "mpls";
    path.status = "active";
    path.priority = 10;
    path.qos_profile = "gold";
    path.failover_interface = "eth2";
    path.rx_bytes = 0;
    path.tx_bytes = 0;
    path.latency_ms = 0.0;
    path.packet_loss_pct = 0.0;
    
    auto added = hal.addPath(path);
    EXPECT_TRUE(added);
    
    auto status = hal.getStatus();
    EXPECT_EQ(status.active_paths, 1);
    EXPECT_EQ(status.paths.size(), 1);
}

TEST(SdwanHalTest, DeletePath) {
    SdwanHal hal;
    
    WanPath path;
    path.path_id = "path-001";
    path.wan_interface = "eth1";
    path.type = "mpls";
    path.status = "active";
    path.priority = 10;
    path.qos_profile = "gold";
    path.failover_interface = "eth2";
    path.rx_bytes = 0;
    path.tx_bytes = 0;
    path.latency_ms = 0.0;
    path.packet_loss_pct = 0.0;
    
    hal.addPath(path);
    auto deleted = hal.deletePath("path-001");
    EXPECT_TRUE(deleted);
    
    auto status = hal.getStatus();
    EXPECT_EQ(status.active_paths, 0);
}

TEST(SdwanHalTest, UpdatePath) {
    SdwanHal hal;
    
    WanPath path;
    path.path_id = "path-001";
    path.wan_interface = "eth1";
    path.type = "mpls";
    path.status = "active";
    path.priority = 10;
    path.qos_profile = "gold";
    path.failover_interface = "eth2";
    path.rx_bytes = 0;
    path.tx_bytes = 0;
    path.latency_ms = 0.0;
    path.packet_loss_pct = 0.0;
    
    hal.addPath(path);
    auto updated = hal.updatePath("path-001", "update");
    EXPECT_TRUE(updated);
    
    auto updated2 = hal.updatePath("nonexistent", "update");
    EXPECT_FALSE(updated2);
}
