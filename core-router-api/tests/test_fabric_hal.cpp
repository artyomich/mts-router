/**
 * MTS-CR-9000 — Fabric HAL Unit Tests
 * Tests for fabric monitoring and configuration
 */

#include <gtest/gtest.h>
#include "hal/fabric_hal.h"

using namespace mts::cr9000::hal;

// Test: Default constructor creates valid state
TEST(FabricHalTest, DefaultConstructor) {
    FabricHal hal;
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-CR-9000-FABRIC");
}

// Test: getStatus returns valid FabricStatus
TEST(FabricHalTest, GetStatus) {
    FabricHal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.name, "MTS-CR-9000-FABRIC");
    EXPECT_EQ(status.num_slots, 8);
    EXPECT_EQ(status.total_bandwidth_gbps, 128.0);
    EXPECT_EQ(status.status, "active");
    EXPECT_EQ(status.slots.size(), 8);
    
    // Verify each slot has valid data
    for (const auto& slot : status.slots) {
        EXPECT_GT(slot.slot_id, 0);
        EXPECT_FALSE(slot.card_type.empty());
        EXPECT_FALSE(slot.status.empty());
        EXPECT_GE(slot.cpu_usage, 0.0);
        EXPECT_GE(slot.memory_usage, 0.0);
    }
}

// Test: Mock mode enables mock data
TEST(FabricHalTest, MockMode) {
    FabricHal hal;
    
    // Set mock mode
    hal.setMockMode(true);
    
    // Create mock slots
    std::vector<SlotStatus> mock_slots;
    for (int i = 1; i <= 8; i++) {
        SlotStatus slot;
        slot.slot_id = i;
        slot.card_type = "line-card";
        slot.status = "up";
        slot.cpu_usage = 50.0;
        slot.memory_usage = 60.0;
        mock_slots.push_back(slot);
    }
    hal.setMockSlots(mock_slots);
    
    auto status = hal.getStatus();
    EXPECT_EQ(status.slots.size(), 8);
    EXPECT_EQ(status.slots[0].cpu_usage, 50.0);
    EXPECT_EQ(status.slots[0].memory_usage, 60.0);
}

// Test: isAvailable returns false in mock mode with no data
TEST(FabricHalTest, IsAvailableMockEmpty) {
    FabricHal hal;
    hal.setMockMode(true);
    // No mock slots set
    EXPECT_FALSE(hal.isAvailable());
}

// Test: isAvailable returns true in mock mode with data
TEST(FabricHalTest, IsAvailableMockWithData) {
    FabricHal hal;
    hal.setMockMode(true);
    
    SlotStatus slot;
    slot.slot_id = 1;
    hal.setMockSlots({slot});
    
    EXPECT_TRUE(hal.isAvailable());
}

// Test: getDeviceName returns correct identifier
TEST(FabricHalTest, GetDeviceName) {
    FabricHal hal;
    EXPECT_EQ(hal.getDeviceName(), "MTS-CR-9000-FABRIC");
}

// Test: Multiple getStatus calls are thread-safe (basic check)
TEST(FabricHalTest, ThreadSafety) {
    FabricHal hal;
    std::thread t1([&hal]() { hal.getStatus(); });
    std::thread t2([&hal]() { hal.getStatus(); });
    std::thread t3([&hal]() { hal.getStatus(); });
    
    t1.join();
    t2.join();
    t3.join();
    
    // If we get here without crash, basic thread safety is OK
    EXPECT_TRUE(true);
}
