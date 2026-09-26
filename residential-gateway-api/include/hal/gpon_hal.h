/**
 * MTS-RG-500 Residential Gateway — GPON HAL
 * Hardware Abstraction Layer for GPON ONU monitoring
 */

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <cstdint>

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
    ~GponHal() override = default;

    GponOnuStatus getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;
    void setMockMode(bool enabled);

private:
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
};

} // namespace mts::rg500::hal
