/**
 * MTS-CR-9000 — SRv6 Manager Unit Tests
 */

#include <gtest/gtest.h>
#include "srv6/srv6_manager.h"

using namespace mts::cr9000::srv6;

TEST(Srv6ManagerTest, DefaultConstructor) {
    Srv6Manager mgr;
    auto entries = mgr.getEntries();
    EXPECT_TRUE(entries.empty());
    EXPECT_EQ(mgr.getBaseSid(), "2001:db8::");
}

TEST(Srv6ManagerTest, AddEntry) {
    Srv6Manager mgr;
    
    auto added = mgr.addEntry("2001:db8::1", 80, "end");
    EXPECT_TRUE(added);
    
    auto entries = mgr.getEntries();
    EXPECT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].sid, "2001:db8::1");
    EXPECT_EQ(entries[0].sid_length, 80);
    EXPECT_EQ(entries[0].encap_mode, "end");
    EXPECT_EQ(entries[0].status, "active");
}

TEST(Srv6ManagerTest, AddDuplicateEntry) {
    Srv6Manager mgr;
    
    mgr.addEntry("2001:db8::1", 80, "end");
    auto added = mgr.addEntry("2001:db8::1", 80, "end.x");
    EXPECT_FALSE(added);
    
    auto entries = mgr.getEntries();
    EXPECT_EQ(entries.size(), 1);
}

TEST(Srv6ManagerTest, AddMultipleEntries) {
    Srv6Manager mgr;
    
    mgr.addEntry("2001:db8::1", 80, "end");
    mgr.addEntry("2001:db8::2", 64, "end.x");
    mgr.addEntry("2001:db8::3", 96, "end.dx");
    
    auto entries = mgr.getEntries();
    EXPECT_EQ(entries.size(), 3);
}

TEST(Srv6ManagerTest, DeleteEntry) {
    Srv6Manager mgr;
    
    mgr.addEntry("2001:db8::1", 80, "end");
    auto deleted = mgr.deleteEntry("2001:db8::1");
    EXPECT_TRUE(deleted);
    
    auto entries = mgr.getEntries();
    EXPECT_TRUE(entries.empty());
}

TEST(Srv6ManagerTest, DeleteNonExistentEntry) {
    Srv6Manager mgr;
    auto deleted = mgr.deleteEntry("2001:db8::999");
    EXPECT_FALSE(deleted);
}

TEST(Srv6ManagerTest, UpdateEntry) {
    Srv6Manager mgr;
    
    mgr.addEntry("2001:db8::1", 80, "end");
    auto updated = mgr.updateEntry("2001:db8::1", "end.x");
    EXPECT_TRUE(updated);
    
    auto entries = mgr.getEntries();
    EXPECT_EQ(entries[0].encap_mode, "end.x");
}

TEST(Srv6ManagerTest, UpdateNonExistentEntry) {
    Srv6Manager mgr;
    auto updated = mgr.updateEntry("2001:db8::999", "end.x");
    EXPECT_FALSE(updated);
}

TEST(Srv6ManagerTest, GetBaseSid) {
    Srv6Manager mgr;
    EXPECT_EQ(mgr.getBaseSid(), "2001:db8::");
}
