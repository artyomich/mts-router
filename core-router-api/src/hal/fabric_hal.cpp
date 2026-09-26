/**
 * MTS-CR-9000 Core Router — Fabric HAL Implementation
 * Intel Tofino 2 fabric crossbar monitoring and configuration
 */

#include "hal/fabric_hal.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace mts::cr9000::hal {

FabricHal::FabricHal() : last_read_(std::chrono::steady_clock::now()) {}

std::vector<SlotStatus> FabricHal::readSlotsFromSysfs() {
    std::vector<SlotStatus> slots;
    
    if (mock_mode_.load()) {
        return mock_slots_;
    }
    
    // Read fabric slot status from sysfs
    for (uint32_t i = 1; i <= 8; i++) {
        SlotStatus slot;
        slot.slot_id = i;
        slot.card_type = "line-card";
        slot.status = "up";
        slot.cpu_usage = 0.0;
        slot.memory_usage = 0.0;
        
        std::string path = "/sys/firmware/dmi/ident/product_slot" + std::to_string(i);
        std::ifstream f(path);
        if (f.is_open()) {
            std::string line;
            std::getline(f, line);
            if (!line.empty()) {
                slot.card_type = line;
            }
        }
        
        slots.push_back(slot);
    }
    
    return slots;
}

double FabricHal::readCpuUsage() {
    // Read CPU usage from /proc/stat
    std::ifstream f("/proc/stat");
    if (!f.is_open()) return 0.0;
    
    std::string name;
    long long user, nice, system, idle, iowait, irq, softirq, steal;
    f >> name >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    
    long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    long long active = total - idle - iowait;
    
    return (total > 0) ? (100.0 * active / total) : 0.0;
}

double FabricHal::readMemoryUsage() {
    std::ifstream f("/proc/meminfo");
    if (!f.is_open()) return 0.0;
    
    long long mem_total = 0, mem_free = 0, mem_available = 0;
    std::string key;
    long long value;
    
    while (f >> key >> value) {
        if (key == "MemTotal:") mem_total = value;
        else if (key == "MemFree:") mem_free = value;
        else if (key == "MemAvailable:") mem_available = value;
        
        if (mem_total > 0 && mem_available > 0) break;
    }
    
    if (mem_total > 0) {
        return 100.0 * (mem_total - mem_available) / mem_total;
    }
    return 0.0;
}

FabricStatus FabricHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    FabricStatus status;
    status.name = "MTS-CR-9000-FABRIC";
    status.num_slots = 8;
    status.total_bandwidth_gbps = 128.0;
    status.status = "active";
    
    auto slots = readSlotsFromSysfs();
    status.slots = std::move(slots);
    
    // Update CPU and memory for each slot
    double cpu = readCpuUsage();
    double mem = readMemoryUsage();
    for (auto& slot : status.slots) {
        slot.cpu_usage = cpu;
        slot.memory_usage = mem;
    }
    
    last_read_ = std::chrono::steady_clock::now();
    return status;
}

bool FabricHal::isAvailable() {
    std::lock_guard<std::mutex> lock(mutex_);
    return !mock_mode_.load() || !mock_slots_.empty();
}

std::string FabricHal::getDeviceName() {
    return "MTS-CR-9000-FABRIC";
}

void FabricHal::setMockMode(bool enabled) {
    mock_mode_.store(enabled);
}

void FabricHal::setMockSlots(const std::vector<SlotStatus>& slots) {
    mock_slots_ = slots;
}

} // namespace mts::cr9000::hal
