/**
 * MTS-CR-9000 Core Router — SRv6 Manager
 * Manages SRv6 SID list and encapsulation
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>

namespace mts::cr9000::srv6 {

struct Srv6Entry {
    std::string sid;
    uint32_t sid_length;
    std::string encap_mode;  // "end", "end.x", "end.dx"
    std::string status;      // "active", "inactive"
    uint64_t packets;
    uint64_t bytes;
};

class Srv6Manager {
public:
    Srv6Manager();
    ~Srv6Manager() = default;

    std::vector<Srv6Entry> getEntries();
    bool addEntry(const std::string& sid, uint32_t sid_length, const std::string& encap_mode);
    bool deleteEntry(const std::string& sid);
    bool updateEntry(const std::string& sid, const std::string& encap_mode);
    std::string getBaseSid();

private:
    std::mutex mutex_;
    std::vector<Srv6Entry> entries_;
    std::string base_sid_;
};

} // namespace mts::cr9000::srv6
