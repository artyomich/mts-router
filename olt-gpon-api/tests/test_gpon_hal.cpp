/**
 * MTS-OLT-2000 — GPON HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/gpon_hal.h"

using namespace mts::olt2000::hal;

TEST(GponHalTest, DefaultConstructor) {
    GponHal hal;
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-OLT-2000");
}

TEST(GponHalTest, GetStatus) {
    GponHal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.device_id, "MTS-OLT-2000-001");
    EXPECT_EQ(status.status, "active");
    EXPECT_EQ(status.total_onu, 0);
    EXPECT_EQ(status.online_onu, 0);
    EXPECT_EQ(status.offline_onu, 0);
    EXPECT_EQ(status.error_onu, 0);
}

TEST(GponHalTest, GetPonPorts) {
    GponHal hal;
    auto ports = hal.getPonPorts();
    
    EXPECT_EQ(ports.size(), 16);
    for (const auto& port : ports) {
        EXPECT_FALSE(port.pon_id.empty());
        EXPECT_FALSE(port.name.empty());
        EXPECT_EQ(port.max_onu, 128);
        EXPECT_EQ(port.downstream_rate, 2488.0);
        EXPECT_EQ(port.upstream_rate, 1244.0);
    }
}

TEST(GponHalTest, RegisterOnu) {
    GponHal hal;
    
    OnuInfo onu;
    onu.onu_id = "onu-001";
    onu.serial = "MTS20240001";
    onu.mac = "aa:bb:cc:dd:ee:01";
    onu.pon_port = "pon0";
    onu.status = "online";
    onu.power_level = -25;
    onu.distance = 10000;
    onu.vlan = 100;
    onu.qos_profile = "gold";
    onu.bandwidth_up = 100;
    onu.bandwidth_down = 300;
    onu.last_seen = 0;
    onu.created = 0;
    onu.rx_bytes = 0;
    onu.tx_bytes = 0;
    
    auto registered = hal.registerOnu(onu);
    EXPECT_TRUE(registered);
    
    auto onus = hal.getOnuList();
    EXPECT_EQ(onus.size(), 1);
    EXPECT_EQ(onus[0].onu_id, "onu-001");
}

TEST(GponHalTest, DeregisterOnu) {
    GponHal hal;
    
    OnuInfo onu;
    onu.onu_id = "onu-001";
    onu.serial = "MTS20240001";
    onu.mac = "aa:bb:cc:dd:ee:01";
    onu.pon_port = "pon0";
    onu.status = "online";
    onu.power_level = -25;
    onu.distance = 10000;
    onu.vlan = 100;
    onu.qos_profile = "gold";
    onu.bandwidth_up = 100;
    onu.bandwidth_down = 300;
    onu.last_seen = 0;
    onu.created = 0;
    onu.rx_bytes = 0;
    onu.tx_bytes = 0;
    
    hal.registerOnu(onu);
    auto deregistered = hal.deregisterOnu("onu-001");
    EXPECT_TRUE(deregistered);
    
    auto onus = hal.getOnuList();
    EXPECT_TRUE(onus.empty());
}

TEST(GponHalTest, UpdateOnuConfig) {
    GponHal hal;
    
    OnuInfo onu;
    onu.onu_id = "onu-001";
    onu.serial = "MTS20240001";
    onu.mac = "aa:bb:cc:dd:ee:01";
    onu.pon_port = "pon0";
    onu.status = "online";
    onu.power_level = -25;
    onu.distance = 10000;
    onu.vlan = 100;
    onu.qos_profile = "gold";
    onu.bandwidth_up = 100;
    onu.bandwidth_down = 300;
    onu.last_seen = 0;
    onu.created = 0;
    onu.rx_bytes = 0;
    onu.tx_bytes = 0;
    
    hal.registerOnu(onu);
    auto updated = hal.updateOnuConfig("onu-001", "update");
    EXPECT_TRUE(updated);
    
    auto updated2 = hal.updateOnuConfig("nonexistent", "update");
    EXPECT_FALSE(updated2);
}

TEST(GponHalTest, MockMode) {
    GponHal hal;
    hal.setMockMode(true);
    
    OnuInfo onu;
    onu.onu_id = "mock-001";
    onu.status = "online";
    hal.setMockOnuList({onu});
    
    auto onus = hal.getOnuList();
    EXPECT_EQ(onus.size(), 1);
    EXPECT_EQ(onus[0].onu_id, "mock-001");
}
