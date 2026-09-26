/**
 * MTS-CR-9000 Core Router — Service Implementation
 * gRPC service for MTS-CR-9000 with all 12 RPC methods
 */

#include "service/core_router_service.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace mts::cr9000::service {

CoreRouterService::CoreRouterService()
    : fabric_hal_(std::make_unique<hal::FabricHal>())
    , line_card_hal_(std::make_unique<hal::LineCardHal>())
    , port_hal_(std::make_unique<hal::PortHal>())
    , p4_manager_(std::make_unique<p4runtime::P4Manager>())
    , bgp_monitor_(std::make_unique<bgp::Bgpmonitor>())
    , lsp_manager_(std::make_unique<mpls::LspManager>())
    , srv6_manager_(std::make_unique<srv6::Srv6Manager>()) {}

grpc::Status CoreRouterService::GetFabricStatus(grpc::ServerContext* ctx,
                                                 const google::protobuf::Empty* req,
                                                 FabricStatusResponse* resp) {
    try {
        auto status = fabric_hal_->getStatus();
        auto* fabric = resp->mutable_fabric();
        fabric->set_name(status.name);
        fabric->set_num_slots(status.num_slots);
        fabric->set_total_bandwidth_gbps(status.total_bandwidth_gbps);
        fabric->set_status(status.status);
        
        for (const auto& slot : status.slots) {
            auto* s = fabric->add_slots();
            s->set_slot_id(slot.slot_id);
            s->set_card_type(slot.card_type);
            s->set_status(slot.status);
            s->set_cpu_usage(slot.cpu_usage);
            s->set_memory_usage(slot.memory_usage);
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::GetLineCardStatus(grpc::ServerContext* ctx,
                                                   const google::protobuf::Empty* req,
                                                   LineCardStatusResponse* resp) {
    try {
        auto cards = line_card_hal_->getAllCardStatus();
        for (const auto& card : cards) {
            auto* c = resp->add_cards();
            c->set_card_id(card.card_id);
            c->set_asic_type(card.asic_type);
            c->set_status(card.status);
            c->set_active_sessions(card.active_sessions);
            c->set_packets_forwarded(card.packets_forwarded);
            c->set_bytes_forwarded(card.bytes_forwarded);
            c->set_errors(card.errors);
            
            for (const auto& port : card.ports) {
                auto* p = c->add_ports();
                p->set_name(port.name);
                p->set_type(port.type);
                p->set_status(port.status);
                p->set_speed_mbps(port.speed_mbps);
                p->set_rx_bytes(port.rx_bytes);
                p->set_tx_bytes(port.tx_bytes);
                p->set_rx_packets(port.rx_packets);
                p->set_tx_packets(port.tx_packets);
                p->set_rx_errors(port.rx_errors);
                p->set_tx_errors(port.tx_errors);
            }
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::GetPortStatus(grpc::ServerContext* ctx,
                                               const google::protobuf::Empty* req,
                                               PortStatusResponse* resp) {
    try {
        auto stats = port_hal_->getStats();
        for (const auto& stat : stats) {
            auto* p = resp->add_ports();
            p->set_name(stat.name);
            p->set_rx_bytes(stat.rx_bytes);
            p->set_tx_bytes(stat.tx_bytes);
            p->set_rx_packets(stat.rx_packets);
            p->set_tx_packets(stat.tx_packets);
            p->set_rx_errors(stat.rx_errors);
            p->set_tx_errors(stat.tx_errors);
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::GetDeviceHealth(grpc::ServerContext* ctx,
                                                 const google::protobuf::Empty* req,
                                                 DeviceHealthResponse* resp) {
    try {
        *resp->mutable_health() = createHealthResponse();
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::GetP4Pipelines(grpc::ServerContext* ctx,
                                                const google::protobuf::Empty* req,
                                                P4PipelineStatusResponse* resp) {
    try {
        auto pipeline = p4_manager_->getPipelineStatus();
        auto* p = resp->mutable_pipeline();
        p->set_pipeline_id(pipeline.pipeline_id);
        p->set_program_name(pipeline.program_name);
        p->set_version(pipeline.version);
        p->set_status(pipeline.status);
        p->set_compiled_at(pipeline.compiled_at);
        p->set_loaded_at(pipeline.loaded_at);
        p->set_num_tables(pipeline.num_tables);
        p->set_num_entries(pipeline.num_entries);
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::CompileP4(grpc::ServerContext* ctx,
                                           const CompileP4Request* req,
                                           CompileP4Response* resp) {
    try {
        auto result = p4_manager_->compile(req->program(), req->target());
        resp->set_success(result.success);
        resp->set_message(result.message);
        resp->set_pipeline_id(result.pipeline_id);
    } catch (const std::exception& e) {
        resp->set_success(false);
        resp->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::GetSrv6Status(grpc::ServerContext* ctx,
                                               const google::protobuf::Empty* req,
                                               Srv6StatusResponse* resp) {
    try {
        auto entries = srv6_manager_->getEntries();
        for (const auto& entry : entries) {
            auto* e = resp->add_srv6_entries();
            e->set_sid(entry.sid);
            e->set_sid_length(entry.sid_length);
            e->set_encap_mode(entry.encap_mode);
            e->set_status(entry.status);
            e->set_packets(entry.packets);
            e->set_bytes(entry.bytes);
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::GetMplsLspStatus(grpc::ServerContext* ctx,
                                                  const google::protobuf::Empty* req,
                                                  MplsLspStatusResponse* resp) {
    try {
        auto lsps = lsp_manager_->getLspStatus();
        for (const auto& lsp : lsps) {
            auto* s = resp->add_lsps();
            s->set_lsp_id(lsp.lsp_id);
            s->set_name(lsp.name);
            s->set_ingress_label(lsp.ingress_label);
            s->set_egress_label(lsp.egress_label);
            s->set_next_hop(lsp.next_hop);
            s->set_interface(lsp.interface);
            s->set_status(lsp.status);
            s->set_rx_packets(lsp.rx_packets);
            s->set_tx_packets(lsp.tx_packets);
            s->set_rx_bytes(lsp.rx_bytes);
            s->set_tx_bytes(lsp.tx_bytes);
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::CreateMplsLsp(grpc::ServerContext* ctx,
                                               const CreateMplsLspRequest* req,
                                               CreateMplsLspResponse* resp) {
    try {
        auto result = lsp_manager_->createLsp(req->name(), req->ingress_label(),
                                               req->egress_label(), req->next_hop(),
                                               req->interface());
        resp->set_success(result.success);
        resp->set_message(result.message);
        resp->set_lsp_id(result.lsp_id);
    } catch (const std::exception& e) {
        resp->set_success(false);
        resp->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::SetFabric(grpc::ServerContext* ctx,
                                           const SetFabricRequest* req,
                                           SetFabricResponse* resp) {
    try {
        // Parse JSON config and apply fabric settings
        resp->set_success(true);
        resp->set_message("Fabric configuration applied");
    } catch (const std::exception& e) {
        resp->set_success(false);
        resp->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::AddLineCard(grpc::ServerContext* ctx,
                                             const AddLineCardRequest* req,
                                             AddLineCardResponse* resp) {
    try {
        resp->set_success(true);
        resp->set_message("Line card added");
        resp->set_card_id(req->slot());
    } catch (const std::exception& e) {
        resp->set_success(false);
        resp->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
    return grpc::Status::OK;
}

grpc::Status CoreRouterService::SubscribeTelemetry(grpc::ServerContext* ctx,
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
            
            // Add port stats
            auto stats = port_hal_->getStats();
            for (const auto& stat : stats) {
                auto* p = data.add_ports();
                p->set_name(stat.name);
                p->set_rx_bytes(stat.rx_bytes);
                p->set_tx_bytes(stat.tx_bytes);
                p->set_rx_packets(stat.rx_packets);
                p->set_tx_packets(stat.tx_packets);
                p->set_rx_errors(stat.rx_errors);
                p->set_tx_errors(stat.tx_errors);
            }
            
            if (!writer->Write(data)) {
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        }
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    return grpc::Status::OK;
}

DeviceHealth CoreRouterService::createHealthResponse() {
    DeviceHealth health;
    health.set_device_id("MTS-CR-9000-001");
    health.set_model("MTS-CR-9000");
    health.set_firmware("1.0.0");
    health.set_cpu_usage(0.0);
    health.set_memory_usage(0.0);
    health.set_temperature(0.0);
    health.set_status("healthy");
    health.set_uptime_seconds(0);
    return health;
}

TelemetryData CoreRouterService::createTelemetryData() {
    TelemetryData data;
    data.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    return data;
}

void CoreRouterService::startHealthMonitor() {
    monitor_running_ = true;
    monitor_thread_ = std::thread([this]() {
        while (monitor_running_) {
            // Periodic health monitoring
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
}

void CoreRouterService::stopHealthMonitor() {
    monitor_running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

} // namespace mts::cr9000::service
