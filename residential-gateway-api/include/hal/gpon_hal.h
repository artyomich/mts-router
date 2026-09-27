/**
 * MTS-RG-500 Residential Gateway — GPON HAL
 * Hardware Abstraction Layer for GPON ONU monitoring
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <cstdint>

// C-типы для GPON HAL
typedef struct {
    bool online;
    int power_level_dbm;
    int distance_m;
    uint32_t pon_port;
    uint32_t vlan;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} mts_rg_gpon_status_t;

namespace mts::rg500::hal {

struct GponOnuStatus {
    std::string onu_id;
    std::string status;     // "online", "offline", "error"
    int32_t power_level;
    int32_t distance;
    std::string pon_port;
    uint32_t vlan;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

class IGponHal {
public:
    virtual ~IGponHal() = default;
    virtual GponOnuStatus getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class GponHal : public IGponHal {
public:
    GponHal();
    ~GponHal() override;

    GponOnuStatus getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;
    void setMockMode(bool enabled);

private:
    GponOnuStatus gpon_status_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;

    GponOnuStatus applyMockStatus();
};

} // namespace mts::rg500::hal
