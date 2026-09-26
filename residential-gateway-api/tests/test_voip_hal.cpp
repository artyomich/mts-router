/**
 * MTS-RG-500 — VoIP HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/voip_hal.h"

using namespace mts::rg500::hal;

TEST(VoipHalTest, DefaultConstructor) {
    VoipHal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.status, "active");
    EXPECT_EQ(status.lines.size(), 1);
    EXPECT_EQ(status.lines[0].line_id, 1);
    EXPECT_EQ(status.lines[0].status, "idle");
    EXPECT_EQ(status.lines[0].codec, "g711");
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-RG-500-VOIP");
}

TEST(VoipHalTest, MockMode) {
    VoipHal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
