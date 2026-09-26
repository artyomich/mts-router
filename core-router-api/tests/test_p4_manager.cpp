/**
 * MTS-CR-9000 — P4 Runtime Manager Unit Tests
 */

#include <gtest/gtest.h>
#include "p4runtime/p4_manager.h"

using namespace mts::cr9000::p4runtime;

TEST(P4ManagerTest, DefaultConstructor) {
    P4Manager mgr;
    auto status = mgr.getPipelineStatus();
    EXPECT_TRUE(status.pipeline_id.empty());
    EXPECT_EQ(status.status, "running");
}

TEST(P4ManagerTest, CompileValidProgram) {
    P4Manager mgr;
    
    std::string program = R"(
        table ipv4_lpm {
            key = {
                header.ipv4.dstAddr: lpm;
            }
            actions = {
                NoAction;
                ipv4_forward;
            }
        }
        
        action ipv4_forward {
            next_hop;
        }
    )";
    
    auto result = mgr.compile(program, "tofino2");
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.pipeline_id.empty());
}

TEST(P4ManagerTest, CompileEmptyProgram) {
    P4Manager mgr;
    auto result = mgr.compile("", "tofino2");
    EXPECT_FALSE(result.success);
}

TEST(P4ManagerTest, CompileInvalidProgram) {
    P4Manager mgr;
    auto result = mgr.compile("invalid p4 code", "tofino2");
    EXPECT_FALSE(result.success);
}

TEST(P4ManagerTest, LoadPipeline) {
    P4Manager mgr;
    
    std::string program = R"(
        table ipv4_lpm {
            key = { header.ipv4.dstAddr: lpm; }
            actions = { NoAction; ipv4_forward; }
        }
        action ipv4_forward { next_hop; }
    )";
    
    auto compile_result = mgr.compile(program, "tofino2");
    EXPECT_TRUE(compile_result.success);
    
    auto loaded = mgr.loadPipeline(compile_result.pipeline_id);
    EXPECT_TRUE(loaded);
    
    auto status = mgr.getPipelineStatus();
    EXPECT_EQ(status.pipeline_id, compile_result.pipeline_id);
}

TEST(P4ManagerTest, DeletePipeline) {
    P4Manager mgr;
    
    std::string program = R"(
        table ipv4_lpm {
            key = { header.ipv4.dstAddr: lpm; }
            actions = { NoAction; ipv4_forward; }
        }
    )";
    
    auto compile_result = mgr.compile(program, "tofino2");
    EXPECT_TRUE(compile_result.success);
    
    auto deleted = mgr.deletePipeline(compile_result.pipeline_id);
    EXPECT_TRUE(deleted);
    
    auto pipelines = mgr.getPipelineList();
    EXPECT_TRUE(pipelines.empty());
}

TEST(P4ManagerTest, AddTableEntry) {
    P4Manager mgr;
    
    std::string program = R"(
        table ipv4_lpm {
            key = { header.ipv4.dstAddr: lpm; }
            actions = { NoAction; ipv4_forward; }
        }
    )";
    
    mgr.compile(program, "tofino2");
    
    auto added = mgr.addTableEntry("test-pipeline", "ipv4_lpm", "10.0.0.0/8", "forward");
    EXPECT_TRUE(added);
}

TEST(P4ManagerTest, DeleteTableEntry) {
    P4Manager mgr;
    
    std::string program = R"(
        table ipv4_lpm {
            key = { header.ipv4.dstAddr: lpm; }
            actions = { NoAction; ipv4_forward; }
        }
    )";
    
    mgr.compile(program, "tofino2");
    
    auto deleted = mgr.deleteTableEntry("test-pipeline", "ipv4_lpm", "10.0.0.0/8");
    EXPECT_TRUE(deleted);
}

TEST(P4ManagerTest, GetPipelineList) {
    P4Manager mgr;
    
    std::string program1 = R"(
        table ipv4_lpm {
            key = { header.ipv4.dstAddr: lpm; }
            actions = { NoAction; ipv4_forward; }
        }
    )";
    
    std::string program2 = R"(
        table ipv6_lpm {
            key = { header.ipv6.dstAddr: lpm; }
            actions = { NoAction; ipv6_forward; }
        }
    )";
    
    mgr.compile(program1, "tofino2");
    mgr.compile(program2, "tofino2");
    
    auto pipelines = mgr.getPipelineList();
    EXPECT_EQ(pipelines.size(), 2);
}
