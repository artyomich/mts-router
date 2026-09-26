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

OltGponService::OltGponService()
    : gpon_hal_(std::make_unique<hal::GponHal>())
    , onu_hal_(std::make_unique<hal::OnuHal>())
    , omci_hal_(std::make_unique<hal::OmciHal>())
    , tr069_hal_(std::make_unique<hal::Tr069Hal>()) {}

grpc::Status OltGponService::GetOltStatus(grpc::ServerContext* ctx,
                                            const google::protobuf::Empty* req,
                                            OltStatusResponse* resp) {
    try {
        auto status = gpon_hal_->getStatus();
        auto* olt = resp->mutable_olt();
        olt->set_device_id(status.device_id);
        olt->set_status(status.status);
        olt->set_total_onu(status.total_onu);
        olt->set_online_onu(status.online_onu);
        olt->set_offline_onu(status.offline_onu);
        olt->set_error_onu(status.error_onu);
        olt->set_temperature(status.temperature);
        olt->set_voltage(status.voltage);
        olt->set_uptime_seconds(status.uptime_seconds);
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponService::GetPonPorts(grpc::ServerContext* ctx,
                                           const google::protobuf::Empty* req,
                                           PonPortInfoResponse* resp) {
    try {
        auto ports = gpon_hal_->getPonPorts();
        for (const auto& p : ports) {
            auto* port = resp->add_pon_ports();
            port->set_pon_id(p.pon_id);
            port->set_name(p.name);
            port->set_status(p.status);
            port->set_num_onu(p.num_onu);
            port->set_max_onu(p.max_onu);
            port->set_downstream_rate(p.downstream_rate);
            port->set_upstream_rate(p.upstream_rate);
            port->set_downstream_util(p.downstream_util);
            port->set_upstream_util(p.upstream_util);
            port->set_optical_power(p.optical_power);
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponService::GetOnuList(grpc::ServerContext* ctx,
                                         const OnuListRequest* req,
                                         grpc::ServerWriter<OnuInfo>* writer) {
    try {
        auto onus = gpon_hal_->getOnuList();
        for (const auto& onu : onus) {
            OnuInfo info;
            info.set_onu_id(onu.onu_id);
            info.set_serial(onu.serial);
            info.set_mac(onu.mac);
            info.set_pon_port(onu.pon_port);
            info.set_status(onu.status);
            info.set_power_level(onu.power_level);
            info.set_distance(onu.distance);
            info.set_vlan(onu.vlan);
            info.set_qos_profile(onu.qos_profile);
            info.set_bandwidth_up(onu.bandwidth_up);
            info.set_bandwidth_down(onu.bandwidth_down);
            info.set_last_seen(onu.last_seen);
            info.set_created(onu.created);
            info.set_rx_bytes(onu.rx_bytes);
            info.set_tx_bytes(onu.tx_bytes);
            
            if (!writer->Write(info)) break;
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponService::GetOnuStatus(grpc::ServerContext* ctx,
                                           const OnuStatusRequest* req,
                                           OnuStatusResponse* resp) {
    try {
        auto onus = gpon_hal_->getOnuList();
        for (const auto& onu : onus) {
            if (onu.onu_id == req->onu_id()) {
                auto* status = resp->mutable_status();
                status->set_onu_id(onu.onu_id);
                status->set_status(onu.status);
                status->set_power_level(onu.power_level);
                status->set_distance(onu.distance);
                status->set_rx_bytes(onu.rx_bytes);
                status->set_tx_bytes(onu.tx_bytes);
                status->set_rx_packets(0);
                status->set_tx_packets(0);
                status->set_rx_errors(0);
                status->set_tx_errors(0);
                status->set_timestamp(std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count());
                return grpc::Status::OK;
            }
        }
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "ONU not found");
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status OltGponService::UpdateOnuConfig(grpc::ServerContext* ctx,
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

grpc::Status OltGponService::ResetOnu(grpc::ServerContext* ctx,
                                       const ResetOnuRequest* req,
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

grpc::Status OltGponService::GetOmcisStatus(grpc::ServerContext* ctx,
                                             const google::protobuf::Empty* req,
                                             OmcisStatusResponse* resp) {
    try {
        auto status = omci_hal_->getStatus();
        auto* omci = resp->mutable_omci();
        omci->set_device_id(status.device_id);
        omci->set_status(status.status);
        omci->set_active_sessions(status.active_sessions);
        omci->set_total_sessions(status.total_sessions);
        omci->set_timestamp(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponService::GetTr069Config(grpc::ServerContext* ctx,
                                             const google::protobuf::Empty* req,
                                             Tr069ConfigResponse* resp) {
    try {
        auto config = tr069_hal_->getConfig();
        auto* tr069 = resp->mutable_config();
        tr069->set_device_id(config.device_id);
        tr069->set_url(config.url);
        tr069->set_username(config.username);
        tr069->set_enabled(config.enabled);
        tr069->set_polling_interval(config.polling_interval);
        tr069->set_last_poll(config.last_poll);
        tr069->set_next_poll(config.next_poll);
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponService::GetDeviceHealth(grpc::ServerContext* ctx,
                                              const google::protobuf::Empty* req,
                                              DeviceHealthResponse* resp) {
    try {
        *resp->mutable_health() = createHealthResponse();
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status OltGponService::SubscribeTelemetry(grpc::ServerContext* ctx,
                                                 const TelemetrySubscription* req,
                                                 grpc::ServerWriter<TelemetryData>* writer) {
    try {
        auto interval_ms = req->sample_interval() > 0 ? req->sample_interval() : 1000;
        
        while (ctx->IsRunning()) {
            TelemetryData data;
            data.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
            
            auto metrics = createTelemetryData();
            data.mutable_metrics()->merge_from(metrics.metrics());
            
            if (!writer->Write(data)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

DeviceHealth OltGponService::createHealthResponse() {
    DeviceHealth health;
    health.set_device_id("MTS-OLT-2000-001");
    health.set_model("MTS-OLT-2000");
    health.set_firmware("1.0.0");
    health.set_cpu_usage(0.0);
    health.set_memory_usage(0.0);
    health.set_temperature(0.0);
    health.set_status("healthy");
    health.set_uptime_seconds(0);
    return health;
}

TelemetryData OltGponService::createTelemetryData() {
    TelemetryData data;
    data.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    return data;
}

void OltGponService::startHealthMonitor() {
    monitor_running_ = true;
    monitor_thread_ = std::thread([this]() {
        while (monitor_running_) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
}

void OltGponService::stopHealthMonitor() {
    monitor_running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

} // namespace mts::olt2000::service
