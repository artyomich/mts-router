/**
 * MTS-CR-9000 — MPLS LSP Manager Unit Tests
 */

#include <gtest/gtest.h>
#include "mpls/lsp_manager.h"

using namespace mts::cr9000::mpls;

TEST(LspManagerTest, DefaultConstructor) {
    LspManager mgr;
    auto lsps = mgr.getLspStatus();
    EXPECT_TRUE(lsps.empty());
}

TEST(LspManagerTest, CreateLsp) {
    LspManager mgr;
    
    auto result = mgr.createLsp("test-lsp", "16", "24", "192.168.1.1", "eth0");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.lsp_id, 1);
    EXPECT_FALSE(result.message.empty());
}

TEST(LspManagerTest, CreateMultipleLsp) {
    LspManager mgr;
    
    auto r1 = mgr.createLsp("lsp-1", "16", "24", "192.168.1.1", "eth0");
    auto r2 = mgr.createLsp("lsp-2", "17", "25", "192.168.1.2", "eth1");
    
    EXPECT_TRUE(r1.success);
    EXPECT_TRUE(r2.success);
    EXPECT_NE(r1.lsp_id, r2.lsp_id);
    
    auto lsps = mgr.getLspStatus();
    EXPECT_EQ(lsps.size(), 2);
}

TEST(LspManagerTest, CreateLspInvalidParams) {
    LspManager mgr;
    
    auto result = mgr.createLsp("", "", "", "", "");
    EXPECT_FALSE(result.success);
}

TEST(LspManagerTest, DeleteLsp) {
    LspManager mgr;
    
    auto create_result = mgr.createLsp("lsp-1", "16", "24", "192.168.1.1", "eth0");
    EXPECT_TRUE(create_result.success);
    
    auto deleted = mgr.deleteLsp(create_result.lsp_id);
    EXPECT_TRUE(deleted);
    
    auto lsps = mgr.getLspStatus();
    EXPECT_TRUE(lsps.empty());
}

TEST(LspManagerTest, DeleteNonExistentLsp) {
    LspManager mgr;
    auto deleted = mgr.deleteLsp(999);
    EXPECT_FALSE(deleted);
}

TEST(LspManagerTest, UpdateLsp) {
    LspManager mgr;
    
    auto create_result = mgr.createLsp("lsp-1", "16", "24", "192.168.1.1", "eth0");
    EXPECT_TRUE(create_result.success);
    
    auto updated = mgr.updateLsp(create_result.lsp_id, "192.168.1.2");
    EXPECT_TRUE(updated);
    
    auto lsps = mgr.getLspStatus();
    EXPECT_EQ(lsps[0].next_hop, "192.168.1.2");
}

TEST(LspManagerTest, UpdateNonExistentLsp) {
    LspManager mgr;
    auto updated = mgr.updateLsp(999, "192.168.1.2");
    EXPECT_FALSE(updated);
}

TEST(LspManagerTest, GetLspStatus) {
    LspManager mgr;
    
    mgr.createLsp("lsp-1", "16", "24", "192.168.1.1", "eth0");
    
    auto lsps = mgr.getLspStatus();
    EXPECT_EQ(lsps.size(), 1);
    EXPECT_EQ(lsps[0].name, "lsp-1");
    EXPECT_EQ(lsps[0].ingress_label, "16");
    EXPECT_EQ(lsps[0].egress_label, "24");
    EXPECT_EQ(lsps[0].next_hop, "192.168.1.1");
    EXPECT_EQ(lsps[0].interface, "eth0");
    EXPECT_EQ(lsps[0].status, "up");
}
