/**
 * MTS-MC-5000 — SMF HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/sm_hal.h"

using namespace mts::mc5000::hal;

TEST(SmfHalTest, DefaultConstructor) {
    SmfHal hal;
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-MC-5000-SMF");
}

TEST(SmfHalTest, GetStatus) {
    SmfHal hal;
    auto config = hal.getStatus();
    
    EXPECT_EQ(config.smf_id, "MTS-MC-5000-SMF-001");
    EXPECT_EQ(config.status, "active");
    EXPECT_EQ(config.max_sessions, 100000);
    EXPECT_EQ(config.dns_primary, "10.64.0.1");
    EXPECT_EQ(config.dns_secondary, "10.64.0.2");
    EXPECT_EQ(config.pgw_ip, "192.168.10.1");
}

TEST(SmfHalTest, MockMode) {
    SmfHal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
