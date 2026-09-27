/**
 * MTS-OLT-2000 OMCI HAL — C++ Wrapper
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include "hal/omci_hal.h"

namespace mts::olt2000::hal {

struct OmciEntityInfo {
    uint32_t entity_id = 0;
    std::string type;
    std::string status;
    std::string name;
    std::vector<uint32_t> associated_entities;
};

struct OmciStatus {
    std::string status;
    uint32_t total_entities = 0;
    uint32_t active_entities = 0;
    uint32_t inactive_entities = 0;
    uint32_t error_entities = 0;
};

class OmciHal {
public:
    OmciHal();
    ~OmciHal();

    OmciStatus getStatus();
    std::vector<OmciEntityInfo> getEntities();
    bool getEntity(uint32_t entity_id, OmciEntityInfo& entity);
    bool createEntity(uint32_t type);
    bool deleteEntity(uint32_t entity_id);
    bool isAvailable();

    void setMockMode(bool enabled);

private:
    bool available_;
    std::atomic<bool> mock_mode_;
    mutable std::mutex mutex_;
};

} // namespace mts::olt2000::hal
