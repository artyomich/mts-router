/**
 * MTS-CR-9000 Core Router — P4 Runtime Manager
 * Handles P4 pipeline configuration via P4Runtime protocol
 * 
 * Responsibilities:
 * - Compile P4 programs for Tofino 2
 * - Manage P4 pipeline tables and entries
 * - Forward P4Runtime gRPC calls to ASIC
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>

namespace mts::cr9000::p4runtime {

struct P4PipelineStatus {
    std::string pipeline_id;
    std::string program_name;
    std::string version;
    std::string status;  // "compiled", "loaded", "running", "error"
    uint64_t compiled_at;
    uint64_t loaded_at;
    uint32_t num_tables;
    uint64_t num_entries;
};

struct CompileResult {
    bool success;
    std::string message;
    std::string pipeline_id;
};

class P4Manager {
public:
    P4Manager();
    ~P4Manager() = default;

    P4PipelineStatus getPipelineStatus();
    CompileResult compile(const std::string& program, const std::string& target);
    bool loadPipeline(const std::string& pipeline_id);
    bool deletePipeline(const std::string& pipeline_id);
    bool addTableEntry(const std::string& pipeline_id, const std::string& table_name,
                       const std::string& key, const std::string& action);
    bool deleteTableEntry(const std::string& pipeline_id, const std::string& table_name,
                          const std::string& key);
    std::vector<std::string> getPipelineList();

private:
    std::mutex mutex_;
    std::map<std::string, P4PipelineStatus> pipelines_;
    std::string current_pipeline_;
};

} // namespace mts::cr9000::p4runtime
