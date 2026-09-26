/**
 * MTS-CR-9000 — Line Card HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/line_card_hal.h"

using namespace mts::cr9000::hal;

TEST(LineCardHalTest, GetStatus) {
    LineCardHal hal;
    auto status = hal.getStatus(1);
    
    EXPECT_EQ(status.card_id, 1);
    EXPECT_EQ(status.asic_type, "tofino2");
    EXPECT_EQ(status.status, "up");
    EXPECT_FALSE(status.ports.empty());
}

TEST(LineCardHalTest, GetAllCardStatus) {
    LineCardHal hal;
    auto cards = hal.getAllCardStatus();
    
    EXPECT_EQ(cards.size(), 8);
    for (size_t i = 0; i < cards.size(); i++) {
        EXPECT_EQ(cards[i].card_id, static_cast<uint32_t>(i + 1));
    }
}

TEST(LineCardHalTest, MockMode) {
    LineCardHal hal;
    hal.setMockMode(true);
    
    PortStatus port;
    port.name = "eth0";
    hal.setMockPorts({port});
    
    auto status = hal.getStatus(1);
    EXPECT_FALSE(status.ports.empty());
}

TEST(LineCardHalTest, IsAvailable) {
    LineCardHal hal;
    EXPECT_TRUE(hal.isAvailable(1));
    EXPECT_FALSE(hal.isAvailable(9)); // Beyond max slots
}

TEST(LineCardHalTest, GetDeviceName) {
    LineCardHal hal;
    EXPECT_EQ(hal.getDeviceName(1), "MTS-CR-9000-SLOT-1");
    EXPECT_EQ(hal.getDeviceName(8), "MTS-CR-9000-SLOT-8");
}
