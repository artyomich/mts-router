/**
 * MTS-CR-9000 Core Router — P4 Runtime Manager Implementation
 * Handles P4 pipeline configuration for Intel Tofino 2
 */

#include "p4runtime/p4_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <algorithm>

namespace mts::cr9000::p4runtime {

P4Manager::P4Manager() : current_pipeline_("") {}

P4PipelineStatus P4Manager::getPipelineStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    P4PipelineStatus status;
    status.pipeline_id = current_pipeline_;
    status.program_name = "default";
    status.version = "1.0.0";
    status.status = "running";
    status.compiled_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    status.loaded_at = status.compiled_at;
    status.num_tables = 0;
    status.num_entries = 0;
    
    if (!current_pipeline_.empty() && pipelines_.count(current_pipeline_)) {
        status = pipelines_[current_pipeline_];
    }
    
    return status;
}

CompileResult P4Manager::compile(const std::string& program, const std::string& target) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    CompileResult result;
    result.success = false;
    
    if (program.empty()) {
        result.message = "Empty P4 program";
        return result;
    }
    
    // Validate P4 program syntax (basic check)
    if (program.find("table") == std::string::npos ||
        program.find("action") == std::string::npos) {
        result.message = "Invalid P4 program: missing table or action definitions";
        return result;
    }
    
    // Generate pipeline ID from program hash (simplified)
    std::string pipeline_id = "p4-" + std::to_hash(program);
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    // Create pipeline status
    P4PipelineStatus pipeline;
    pipeline.pipeline_id = pipeline_id;
    pipeline.program_name = "p4_program";
    pipeline.version = "1.0.0";
    pipeline.status = "compiled";
    pipeline.compiled_at = now;
    pipeline.loaded_at = 0;
    pipeline.num_tables = 0;
    pipeline.num_entries = 0;
    
    pipelines_[pipeline_id] = pipeline;
    
    result.success = true;
    result.message = "P4 program compiled successfully";
    result.pipeline_id = pipeline_id;
    
    return result;
}

bool P4Manager::loadPipeline(const std::string& pipeline_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!pipelines_.count(pipeline_id)) {
        return false;
    }
    
    pipelines_[pipeline_id].status = "loaded";
    pipelines_[pipeline_id].loaded_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    current_pipeline_ = pipeline_id;
    return true;
}

bool P4Manager::deletePipeline(const std::string& pipeline_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!pipelines_.count(pipeline_id)) {
        return false;
    }
    
    if (current_pipeline_ == pipeline_id) {
        current_pipeline_ = "";
    }
    
    pipelines_.erase(pipeline_id);
    return true;
}

bool P4Manager::addTableEntry(const std::string& pipeline_id, const std::string& table_name,
                               const std::string& key, const std::string& action) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!pipelines_.count(pipeline_id)) {
        return false;
    }
    
    pipelines_[pipeline_id].num_entries++;
    return true;
}

bool P4Manager::deleteTableEntry(const std::string& pipeline_id, const std::string& table_name,
                                  const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!pipelines_.count(pipeline_id)) {
        return false;
    }
    
    if (pipelines_[pipeline_id].num_entries > 0) {
        pipelines_[pipeline_id].num_entries--;
    }
    return true;
}

std::vector<std::string> P4Manager::getPipelineList() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> ids;
    for (const auto& pair : pipelines_) {
        ids.push_back(pair.first);
    }
    return ids;
}

} // namespace mts::cr9000::p4runtime
