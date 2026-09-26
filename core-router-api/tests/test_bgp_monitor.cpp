/**
 * MTS-CR-9000 — BGP Monitor Unit Tests
 */

#include <gtest/gtest.h>
#include "bgp/bgp_monitor.h"

using namespace mts::cr9000::bgp;

TEST(BgpMonitorTest, DefaultConstructor) {
    BgpMonitor monitor;
    auto status = monitor.getStatus();
    
    EXPECT_EQ(status.device_id, "MTS-CR-9000-001");
    EXPECT_EQ(status.local_as, 65001);
    EXPECT_EQ(status.router_id, "10.0.0.1");
    EXPECT_EQ(status.state, "established");
    EXPECT_TRUE(status.neighbors.empty());
}

TEST(BgpMonitorTest, AddNeighbor) {
    BgpMonitor monitor;
    
    auto added = monitor.addNeighbor("192.168.1.2", 65002);
    EXPECT_TRUE(added);
    
    auto neighbors = monitor.getNeighbors();
    EXPECT_EQ(neighbors.size(), 1);
    EXPECT_EQ(neighbors[0].peer, "192.168.1.2");
    EXPECT_EQ(neighbors[0].peer_as, 65002);
    EXPECT_EQ(neighbors[0].state, "idle");
}

TEST(BgpMonitorTest, AddDuplicateNeighbor) {
    BgpMonitor monitor;
    
    monitor.addNeighbor("192.168.1.2", 65002);
    auto added = monitor.addNeighbor("192.168.1.2", 65003);
    EXPECT_FALSE(added);
}

TEST(BgpMonitorTest, AddMultipleNeighbors) {
    BgpMonitor monitor;
    
    monitor.addNeighbor("192.168.1.2", 65002);
    monitor.addNeighbor("192.168.1.3", 65003);
    monitor.addNeighbor("192.168.1.4", 65004);
    
    auto neighbors = monitor.getNeighbors();
    EXPECT_EQ(neighbors.size(), 3);
}

TEST(BgpMonitorTest, DeleteNeighbor) {
    BgpMonitor monitor;
    
    monitor.addNeighbor("192.168.1.2", 65002);
    auto deleted = monitor.deleteNeighbor("192.168.1.2");
    EXPECT_TRUE(deleted);
    
    auto neighbors = monitor.getNeighbors();
    EXPECT_TRUE(neighbors.empty());
}

TEST(BgpMonitorTest, DeleteNonExistentNeighbor) {
    BgpMonitor monitor;
    auto deleted = monitor.deleteNeighbor("192.168.1.999");
    EXPECT_FALSE(deleted);
}

TEST(BgpMonitorTest, UpdateNeighbor) {
    BgpMonitor monitor;
    
    monitor.addNeighbor("192.168.1.2", 65002);
    auto updated = monitor.updateNeighbor("192.168.1.2", "update");
    EXPECT_TRUE(updated);
}

TEST(BgpMonitorTest, UpdateNonExistentNeighbor) {
    BgpMonitor monitor;
    auto updated = monitor.updateNeighbor("192.168.1.999", "update");
    EXPECT_FALSE(updated);
}

TEST(BgpMonitorTest, GetNeighbors) {
    BgpMonitor monitor;
    
    monitor.addNeighbor("192.168.1.2", 65002);
    monitor.addNeighbor("192.168.1.3", 65003);
    
    auto neighbors = monitor.getNeighbors();
    EXPECT_EQ(neighbors.size(), 2);
}
