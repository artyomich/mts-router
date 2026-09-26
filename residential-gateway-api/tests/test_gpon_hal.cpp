/**
 * MTS-RG-500 — GPON HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/gpon_hal.h"

using namespace mts::rg500::hal;

TEST(GponHalTest, DefaultConstructor) {
    GponHal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.onu_id, "MTS-RG-500-ONU-001");
    EXPECT_EQ(status.status, "online");
    EXPECT_EQ(status.power_level, -25);
    EXPECT_EQ(status.distance, 10000);
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-RG-500-GPON");
}

TEST(GponHalTest, MockMode) {
    GponHal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
