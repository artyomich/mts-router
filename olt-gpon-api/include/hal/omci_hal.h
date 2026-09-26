/**
 * MTS-OLT-2000 OLT GPON — OMCI HAL
 * OMCI management entity monitoring
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

namespace mts::olt2000::hal {

struct OmcisStatus {
    std::string device_id;
    std::string status;
    uint32_t active_sessions;
    uint32_t total_sessions;
};

class IOmciHal {
public:
    virtual ~IOmciHal() = default;
    virtual OmcisStatus getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class OmciHal : public IOmciHal {
public:
    OmciHal();
    ~OmciHal() override = default;

    OmcisStatus getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    void setMockMode(bool enabled);

private:
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
};

} // namespace mts::olt2000::hal
