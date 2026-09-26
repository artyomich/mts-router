/**
 * MTS-RG-500 — TR-069 HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/tr069_hal.h"

using namespace mts::rg500::hal;

TEST(Tr069HalTest, DefaultConstructor) {
    Tr069Hal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.device_id, "MTS-RG-500-001");
    EXPECT_EQ(status.url, "acs.mts.ru:7547");
    EXPECT_TRUE(status.enabled);
    EXPECT_EQ(status.polling_interval, 600);
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-RG-500-TR069");
}

TEST(Tr069HalTest, MockMode) {
    Tr069Hal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
