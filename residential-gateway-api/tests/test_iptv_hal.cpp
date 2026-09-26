/**
 * MTS-RG-500 — IPTV HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/iptv_hal.h"

using namespace mts::rg500::hal;

TEST(IptvHalTest, DefaultConstructor) {
    IptvHal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.status, "active");
    EXPECT_EQ(status.total_channels, 50);
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-RG-500-IPTV");
}

TEST(IptvHalTest, MockMode) {
    IptvHal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
