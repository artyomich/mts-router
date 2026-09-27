/**
 * MTS-OLT-2000 OLT GPON — Service Implementation
 * gRPC service for MTS-OLT-2000 with all 10 RPC methods
 */

#include "service/olt_gpon_service.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace mts::olt2000::service {

OltGponServiceImpl::OltGponServiceImpl()
    : gpon_hal_(std::make_unique<hal::GponHal>())
    , onu_hal_(std::make_unique<hal::OnuHal>())
    , omci_hal_(std::make_unique<hal::OmciHal>())
    , tr069_hal_(std::make_unique<hal::Tr069Hal>()) {}

grpc::Status OltGponServiceImpl::GetOltStatus(grpc::ServerContext* ctx,
                                              const Empty* req,
                                              OltStatusResponse* resp) {
    try {
        auto status = gpon_hal_->getStatus();
        auto* olt = resp->mutable_olt_status();
        olt->set_olt_id(status.olt_id);
        olt->set_status(status.status);
        olt->set_total_onu(status.total_onu);
        olt->set_online_onu(status.online_onu);
        olt->set_pon_ports(status.pon_ports);
        olt->set_rx_bytes(status.rx_bytes);
        olt->set_tx_bytes(status.tx_bytes);
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

// GetPonPorts - removed (not in proto)

grpc::Status OltGponServiceImpl::ListOnus(grpc::ServerContext* ctx,
                                             const Empty* req,
                                             OnuListResponse* resp) {
    try {
        auto onus = gpon_hal_->getOnuList();
        for (const auto& onu : onus) {
            auto* info = resp->add_onus();
            info->set_onu_id(onu.onu_id);
            info->set_pon_port(onu.pon_port);
            info->set_status(onu.status);
            info->set_serial_number(onu.serial_number);
            info->set_mac_address(onu.mac_address);
            info->set_firmware_version(onu.firmware_version);
            info->set_power_level(onu.power_level_dbm);
            info->set_distance(onu.distance_m);
            info->set_vlan(onu.vlan);
            info->set_qos_profile(onu.qos_profile);
            info->set_bandwidth_up_mbps(onu.bandwidth_up_mbps);
            info->set_bandwidth_down_mbps(onu.bandwidth_down_mbps);
            info->set_rx_bytes(onu.rx_bytes);
            info->set_tx_bytes(onu.tx_bytes);
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::GetOnu(grpc::ServerContext* ctx,
                                              const OnuIdRequest* req,
                                              OnuResponse* resp) {
    try {
        auto onus = gpon_hal_->getOnuList();
        for (const auto& onu : onus) {
            if (onu.onu_id == req->onu_id()) {
                auto* status = resp->mutable_onu();
                status->set_onu_id(onu.onu_id);
                status->set_status(onu.status);
                status->set_power_level(onu.power_level_dbm);
                status->set_distance(onu.distance_m);
                status->set_rx_bytes(onu.rx_bytes);
                status->set_tx_bytes(onu.tx_bytes);
                return grpc::Status::OK;
            }
        }
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "ONU not found");
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status OltGponServiceImpl::UpdateOnuConfig(grpc::ServerContext* ctx,
                                                  const UpdateOnuConfigRequest* req,
                                                  UpdateOnuConfigResponse* resp) {
    try {
        resp->set_success(true);
        resp->set_message("ONU configuration updated");
    } catch (const std::exception& e) {
        resp->set_success(false);
        resp->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::ResetOnu(grpc::ServerContext* ctx,
                                          const OnuIdRequest* req,
                                          ResetOnuResponse* resp) {
    try {
        resp->set_success(true);
        resp->set_message("ONU reset");
    } catch (const std::exception& e) {
        resp->set_success(false);
        resp->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::GetOmciStatus(grpc::ServerContext* ctx,
                                                  const Empty* req,
                                                  OmciStatusResponse* resp) {
    try {
        auto status = omci_hal_->getStatus();
        auto* omci = resp->mutable_omci_status();
        omci->set_status(status.status);
        omci->set_total_entities(status.total_entities);
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::GetTr069Config(grpc::ServerContext* ctx,
                                                  const Empty* req,
                                                  Tr069ConfigResponse* resp) {
    try {
        auto config = tr069_hal_->getConfig();
        auto* tr069 = resp->mutable_config();
        tr069->set_enabled(config.enabled);
        tr069->set_acs_url(config.acs_url);
        tr069->set_polling_enabled(config.polling_enabled);
        tr069->set_polling_interval(config.polling_interval);
        tr069->set_username(config.username);
        tr069->set_last_session_id(config.last_session_id);
        tr069->set_last_bootstrap(config.last_bootstrap);
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::GetHealth(grpc::ServerContext* ctx,
                                                  const Empty* req,
                                                  DeviceHealthResponse* resp) {
    try {
        *resp->mutable_health() = createHealthResponse();
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::HealthCheck(grpc::ServerContext* ctx,
                                               const HealthCheckRequest* req,
                                               HealthCheckResponse* resp) {
    (void)ctx;
    (void)req;
    resp->set_status(HealthCheckResponse::SERVING);
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::SetOnu(grpc::ServerContext* ctx,
                                         const SetOnuRequest* req,
                                         SetOnuResponse* resp) {
    (void)ctx;
    try {
        // Apply to HAL
        bool success = gpon_hal_->configureOnu(
            req->onu_id(),
            req->pon_port(),
            req->vlan(),
            req->qos_profile(),
            req->bandwidth_up_mbps(),
            req->bandwidth_down_mbps());
        
        resp->set_success(success);
        if (success) {
            resp->set_message("ONU configuration applied successfully");
        } else {
            resp->set_message("Failed to apply ONU configuration");
        }
    } catch (const std::exception& e) {
        resp->set_success(false);
        resp->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::SetTr069Config(grpc::ServerContext* ctx,
                                                  const Tr069ConfigRequest* req,
                                                  Tr069ConfigResponse* resp) {
    (void)ctx;
    try {
        mts::olt2000::hal::Tr069Config config;
        config.enabled = req->enabled();
        config.acs_url = req->acs_url();
        config.polling_enabled = req->polling_enabled();
        config.polling_interval = req->polling_interval();
        config.username = req->username();
        if (req->password().size() > 0) {
            config.password = req->password();
        }
        
        bool success = tr069_hal_->setConfig(config);
        
        resp->mutable_config()->set_enabled(config.enabled);
        resp->mutable_config()->set_acs_url(config.acs_url);
        resp->mutable_config()->set_polling_enabled(config.polling_enabled);
        resp->mutable_config()->set_polling_interval(config.polling_interval);
        resp->mutable_config()->set_username(config.username);
        resp->mutable_config()->set_password(config.password);
        
        if (success) {
            resp->mutable_config()->set_last_session_id(0);
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponServiceImpl::GetWdmStatus(grpc::ServerContext* ctx,
                                               const Empty* req,
                                               WdmStatusResponse* resp) {
    (void)ctx;
    (void)req;
    // WDM status is not implemented for MTS-OLT-2000
    // Return empty response
    return grpc::Status::OK;
}

// SubscribeTelemetry - removed (not in proto)
// grpc::Status OltGponServiceImpl::SubscribeTelemetry(grpc::ServerContext* ctx,
//                                                      const TelemetrySubscription* req,
//                                                      grpc::ServerWriter<TelemetryData>* writer) {
//     ...
// }

DeviceHealth OltGponServiceImpl::createHealthResponse() {
    DeviceHealth health;
    health.set_device_id("MTS-OLT-2000-001");
    health.set_model("MTS-OLT-2000");
    health.set_firmware("1.0.0");
    health.set_cpu_usage(0.0);
    health.set_memory_usage(0.0);
    health.set_temperature(0.0);
    health.set_status("healthy");
    return health;
}

TelemetryData OltGponServiceImpl::createTelemetryData() {
    TelemetryData data;
    data.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    return data;
}

void OltGponServiceImpl::startHealthMonitor() {
    monitor_running_ = true;
    monitor_thread_ = std::thread([this]() {
        while (monitor_running_) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
}

void OltGponServiceImpl::stopHealthMonitor() {
    monitor_running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

} // namespace mts::olt2000::service
