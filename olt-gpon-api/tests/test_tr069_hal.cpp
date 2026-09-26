/**
 * MTS-OLT-2000 — TR-069 HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/tr069_hal.h"

using namespace mts::olt2000::hal;

TEST(Tr069HalTest, DefaultConstructor) {
    Tr069Hal hal;
    auto config = hal.getConfig();
    
    EXPECT_EQ(config.device_id, "MTS-OLT-2000-001");
    EXPECT_EQ(config.url, "acs.mts.ru:7547");
    EXPECT_TRUE(config.enabled);
    EXPECT_EQ(config.polling_interval, 300);
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-OLT-2000-TR069");
}

TEST(Tr069HalTest, SetUrl) {
    Tr069Hal hal;
    
    hal.setUrl("new-acs.mts.ru:7547");
    auto config = hal.getConfig();
    EXPECT_EQ(config.url, "new-acs.mts.ru:7547");
}

TEST(Tr069HalTest, MockMode) {
    Tr069Hal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
