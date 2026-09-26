/**
 * MTS-OLT-2000 — OMCI HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/omci_hal.h"

using namespace mts::olt2000::hal;

TEST(OmciHalTest, DefaultConstructor) {
    OmciHal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.device_id, "MTS-OLT-2000-001");
    EXPECT_EQ(status.status, "active");
    EXPECT_EQ(status.total_sessions, 2048);
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-OLT-2000-OMCI");
}

TEST(OmciHalTest, MockMode) {
    OmciHal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
