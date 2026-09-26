/**
 * MTS-RG-500 — WiFi HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/wifi_hal.h"

using namespace mts::rg500::hal;

TEST(WifiHalTest, DefaultConstructor) {
    WifiHal hal;
    auto bss = hal.getBssInfo();
    EXPECT_EQ(bss.size(), 2);
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-RG-500-WIFI");
}

TEST(WifiHalTest, GetBssInfo) {
    WifiHal hal;
    auto bss = hal.getBssInfo();
    
    // Find 2.4 GHz BSS
    bool has_24 = false, has_5 = false;
    for (const auto& b : bss) {
        if (b.band == "2.4ghz") {
            has_24 = true;
            EXPECT_EQ(b.ssid, "MTS_Home_2G");
            EXPECT_EQ(b.channel, 6);
            EXPECT_EQ(b.bandwidth, 40);
            EXPECT_EQ(b.security, "wpa2");
            EXPECT_EQ(b.mode, "ap");
            EXPECT_EQ(b.status, "up");
        }
        if (b.band == "5ghz") {
            has_5 = true;
            EXPECT_EQ(b.ssid, "MTS_Home_5G");
            EXPECT_EQ(b.channel, 36);
            EXPECT_EQ(b.bandwidth, 80);
            EXPECT_EQ(b.security, "wpa3");
            EXPECT_EQ(b.mode, "ap");
            EXPECT_EQ(b.status, "up");
        }
    }
    EXPECT_TRUE(has_24);
    EXPECT_TRUE(has_5);
}

TEST(WifiHalTest, GetClientInfo) {
    WifiHal hal;
    auto clients = hal.getClientInfo();
    // Should not crash
    EXPECT_TRUE(true);
}

TEST(WifiHalTest, MockMode) {
    WifiHal hal;
    hal.setMockMode(true);
    EXPECT_FALSE(hal.isAvailable());
}
