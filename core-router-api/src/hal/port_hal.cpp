/**
 * MTS-CR-9000 Core Router — Port HAL Implementation
 * Port statistics monitoring via sysfs/procfs
 */

#include "hal/port_hal.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace mts::cr9000::hal {

PortHal::PortHal() : mock_mode_(false) {}

std::vector<PortStats> PortHal::readFromSysfs() {
    std::vector<PortStats> stats;
    
    if (mock_mode_) {
        return mock_stats_;
    }
    
    // Read from /sys/class/net/<iface>/statistics/
    std::ifstream f("/proc/net/dev");
    if (!f.is_open()) return stats;
    
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(":") == std::string::npos) continue;
        
        std::istringstream iss(line);
        std::string iface;
        iss >> iface;
        iface.erase(iface.find(':'));
        
        // Skip loopback
        if (iface == "lo") continue;
        
        PortStats stat;
        stat.name = iface;
        iss >> stat.rx_bytes >> stat.rx_packets >> stat.rx_errors >> stat.rx_drops;
        iss >> stat.tx_bytes >> stat.tx_packets >> stat.tx_errors >> stat.tx_drops;
        
        stats.push_back(stat);
    }
    
    return stats;
}

std::vector<PortStats> PortHal::getStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    return readFromSysfs();
}

bool PortHal::isAvailable() {
    return !mock_mode_ || !mock_stats_.empty();
}

std::string PortHal::getDeviceName() {
    return "MTS-CR-9000-PORTS";
}

void PortHal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

void PortHal::setMockStats(const std::vector<PortStats>& stats) {
    mock_stats_ = stats;
}

} // namespace mts::cr9000::hal
