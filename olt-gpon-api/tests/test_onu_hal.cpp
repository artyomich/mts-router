/**
 * MTS-OLT-2000 — ONU HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/onu_hal.h"

using namespace mts::olt2000::hal;

TEST(OnuHalTest, DefaultConstructor) {
    OnuHal hal;
    auto configs = hal.getConfigs();
    EXPECT_TRUE(configs.empty());
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-OLT-2000-ONU");
}

TEST(OnuHalTest, SetConfig) {
    OnuHal hal;
    
    OnuConfig config;
    config.onu_id = "onu-001";
    config.pon_port = "pon0";
    config.vlan = 100;
    config.qos_profile = "gold";
    config.bandwidth_up = 100;
    config.bandwidth_down = 300;
    
    auto set = hal.setConfig(config);
    EXPECT_TRUE(set);
    
    auto configs = hal.getConfigs();
    EXPECT_EQ(configs.size(), 1);
    EXPECT_EQ(configs[0].vlan, 100);
}

TEST(OnuHalTest, DeleteConfig) {
    OnuHal hal;
    
    OnuConfig config;
    config.onu_id = "onu-001";
    config.pon_port = "pon0";
    config.vlan = 100;
    config.qos_profile = "gold";
    config.bandwidth_up = 100;
    config.bandwidth_down = 300;
    
    hal.setConfig(config);
    auto deleted = hal.deleteConfig("onu-001");
    EXPECT_TRUE(deleted);
    
    auto configs = hal.getConfigs();
    EXPECT_TRUE(configs.empty());
}
