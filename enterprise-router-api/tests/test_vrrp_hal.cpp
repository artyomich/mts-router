/**
 * MTS-ER-1000 — VRRP HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/vrrp_hal.h"

using namespace mts::er1000::hal;

TEST(VrrpHalTest, DefaultConstructor) {
    VrrpHal hal;
    auto status = hal.getStatus();
    EXPECT_EQ(status.size(), 1);
    EXPECT_EQ(status[0].interface, "eth0");
    EXPECT_EQ(status[0].virtual_router_id, 1);
    EXPECT_EQ(status[0].status, "master");
    EXPECT_EQ(status[0].priority, 100.0);
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-ER-1000-VRRP");
}

TEST(VrrpHalTest, MockMode) {
    VrrpHal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
