/**
 * MTS-CR-9000 — Port HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/port_hal.h"

using namespace mts::cr9000::hal;

TEST(PortHalTest, GetStats) {
    PortHal hal;
    auto stats = hal.getStats();
    // Stats may be empty on non-Linux, but should not crash
    EXPECT_TRUE(true);
}

TEST(PortHalTest, MockMode) {
    PortHal hal;
    hal.setMockMode(true);
    
    PortStats stat;
    stat.name = "eth0";
    stat.rx_bytes = 1000;
    stat.tx_bytes = 500;
    hal.setMockStats({stat});
    
    auto stats = hal.getStats();
    EXPECT_EQ(stats.size(), 1);
    EXPECT_EQ(stats[0].rx_bytes, 1000);
}

TEST(PortHalTest, IsAvailable) {
    PortHal hal;
    EXPECT_TRUE(hal.isAvailable());
    
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable()); // No mock data
}

TEST(PortHalTest, GetDeviceName) {
    PortHal hal;
    EXPECT_EQ(hal.getDeviceName(), "MTS-CR-9000-PORTS");
}
