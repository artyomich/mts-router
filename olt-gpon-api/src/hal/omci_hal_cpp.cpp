/**
 * MTS-OLT-2000 OMCI HAL — C++ Wrapper Implementation
 */

#include "hal/omci_hal_cpp.h"
#include <iostream>

namespace mts::olt2000::hal {

OmciHal::OmciHal() : available_(true), mock_mode_(false) {
    std::cout << "[OmciHal] Constructed" << std::endl;
}

OmciHal::~OmciHal() {
    std::cout << "[OmciHal] Destructed" << std::endl;
}

OmciStatus OmciHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (mock_mode_.load()) {
        OmciStatus status;
        status.status = "active";
        status.total_entities = 256;
        status.active_entities = 128;
        status.inactive_entities = 128;
        status.error_entities = 0;
        return status;
    }
    
    mts_olt2000_omci_status_t c_status;
    if (mts_olt2000_omci_get_status(&c_status) != 0) {
        OmciStatus status;
        status.status = "error";
        return status;
    }
    
    OmciStatus status;
    status.status = (c_status.status == MTS_OLT2000_OMCI_ENTITY_ACTIVE) ? "active" : "inactive";
    status.total_entities = c_status.total_entities;
    status.active_entities = c_status.active_entities;
    status.inactive_entities = c_status.inactive_entities;
    status.error_entities = c_status.error_entities;
    return status;
}

std::vector<OmciEntityInfo> OmciHal::getEntities() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<OmciEntityInfo> result;
    
    if (mock_mode_.load()) {
        for (uint32_t i = 1; i <= 10; i++) {
            OmciEntityInfo info;
            info.entity_id = i;
            info.type = "ONU";
            info.status = "active";
            info.name = "OMCI-Entity-" + std::to_string(i);
            result.push_back(info);
        }
        return result;
    }
    
    mts_olt2000_omci_entity_t entity;
    for (uint32_t i = 1; i <= 256; i++) {
        if (mts_olt2000_omci_get_entity(i, &entity) == 0) {
            OmciEntityInfo info;
            info.entity_id = entity.entity_id;
            result.push_back(info);
        }
    }
    return result;
}

bool OmciHal::getEntity(uint32_t entity_id, OmciEntityInfo& entity) {
    std::lock_guard<std::mutex> lock(mutex_);
    mts_olt2000_omci_entity_t c_entity;
    return mts_olt2000_omci_get_entity(entity_id, &c_entity) == 0;
}

bool OmciHal::createEntity(uint32_t type) {
    std::lock_guard<std::mutex> lock(mutex_);
    return mts_olt2000_omci_create_entity(static_cast<mts_olt2000_omci_entity_type_t>(type), nullptr) >= 0;
}

bool OmciHal::deleteEntity(uint32_t entity_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return mts_olt2000_omci_delete_entity(entity_id) >= 0;
}

bool OmciHal::isAvailable() {
    return available_;
}

void OmciHal::setMockMode(bool enabled) {
    mock_mode_.store(enabled);
}

} // namespace mts::olt2000::hal
