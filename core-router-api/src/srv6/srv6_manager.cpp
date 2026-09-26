/**
 * MTS-CR-9000 Core Router — SRv6 Manager Implementation
 * Manages SRv6 SID list and encapsulation
 */

#include "srv6/srv6_manager.h"
#include <algorithm>

namespace mts::cr9000::srv6 {

Srv6Manager::Srv6Manager() : base_sid_("2001:db8::") {}

std::vector<Srv6Entry> Srv6Manager::getEntries() {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_;
}

bool Srv6Manager::addEntry(const std::string& sid, uint32_t sid_length, const std::string& encap_mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if already exists
    for (const auto& e : entries_) {
        if (e.sid == sid) return false;
    }
    
    Srv6Entry entry;
    entry.sid = sid;
    entry.sid_length = sid_length;
    entry.encap_mode = encap_mode;
    entry.status = "active";
    entry.packets = 0;
    entry.bytes = 0;
    
    entries_.push_back(entry);
    return true;
}

bool Srv6Manager::deleteEntry(const std::string& sid) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&sid](const Srv6Entry& e) { return e.sid == sid; });
    
    if (it == entries_.end()) return false;
    
    entries_.erase(it);
    return true;
}

bool Srv6Manager::updateEntry(const std::string& sid, const std::string& encap_mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& e : entries_) {
        if (e.sid == sid) {
            e.encap_mode = encap_mode;
            return true;
        }
    }
    return false;
}

std::string Srv6Manager::getBaseSid() {
    std::lock_guard<std::mutex> lock(mutex_);
    return base_sid_;
}

} // namespace mts::cr9000::srv6
