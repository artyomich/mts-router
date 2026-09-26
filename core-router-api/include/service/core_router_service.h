/**
 * MTS-CR-9000 Core Router — Service Header
 * gRPC service implementation for MTS-CR-9000
 * 
 * Implements MtsCoreRouterService with 12 RPC methods:
 * - GetFabricStatus, GetLineCardStatus, GetPortStatus
 * - GetDeviceHealth, GetP4Pipelines, CompileP4
 * - GetSrv6Status, GetMplsLspStatus, CreateMplsLsp
 * - SetFabric, AddLineCard, SubscribeTelemetry
 */

#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <google/protobuf/empty.pb.h>

#include "proto/mts_core_router.grpc.pb.h"
#include "proto/mts_core_router.pb.h"

#include "hal/fabric_hal.h"
#include "hal/line_card_hal.h"
#include "hal/port_hal.h"
#include "p4runtime/p4_manager.h"
#include "bgp/bgp_monitor.h"
#include "mpls/lsp_manager.h"
#include "srv6/srv6_manager.h"

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace mts::cr9000::service {

class CoreRouterService final : public MtsCoreRouterService::Service {
public:
    CoreRouterService();
    ~CoreRouterService() override = default;

    // RPC implementations
    grpc::Status GetFabricStatus(grpc::ServerContext* ctx,
                                  const google::protobuf::Empty* req,
                                  FabricStatusResponse* resp) override;
    
    grpc::Status GetLineCardStatus(grpc::ServerContext* ctx,
                                    const google::protobuf::Empty* req,
                                    LineCardStatusResponse* resp) override;
    
    grpc::Status GetPortStatus(grpc::ServerContext* ctx,
                                const google::protobuf::Empty* req,
                                PortStatusResponse* resp) override;
    
    grpc::Status GetDeviceHealth(grpc::ServerContext* ctx,
                                  const google::protobuf::Empty* req,
                                  DeviceHealthResponse* resp) override;
    
    grpc::Status GetP4Pipelines(grpc::ServerContext* ctx,
                                 const google::protobuf::Empty* req,
                                 P4PipelineStatusResponse* resp) override;
    
    grpc::Status CompileP4(grpc::ServerContext* ctx,
                           const CompileP4Request* req,
                           CompileP4Response* resp) override;
    
    grpc::Status GetSrv6Status(grpc::ServerContext* ctx,
                                const google::protobuf::Empty* req,
                                Srv6StatusResponse* resp) override;
    
    grpc::Status GetMplsLspStatus(grpc::ServerContext* ctx,
                                   const google::protobuf::Empty* req,
                                   MplsLspStatusResponse* resp) override;
    
    grpc::Status CreateMplsLsp(grpc::ServerContext* ctx,
                                const CreateMplsLspRequest* req,
                                CreateMplsLspResponse* resp) override;
    
    grpc::Status SetFabric(grpc::ServerContext* ctx,
                           const SetFabricRequest* req,
                           SetFabricResponse* resp) override;
    
    grpc::Status AddLineCard(grpc::ServerContext* ctx,
                              const AddLineCardRequest* req,
                              AddLineCardResponse* resp) override;
    
    grpc::Status SubscribeTelemetry(grpc::ServerContext* ctx,
                                     const TelemetrySubscription* req,
                                     grpc::ServerWriter<TelemetryData>* writer) override;

    // Start health monitoring thread
    void startHealthMonitor();
    // Stop health monitoring
    void stopHealthMonitor();

private:
    DeviceHealth createHealthResponse();
    TelemetryData createTelemetryData();

    // HAL instances
    std::unique_ptr<hal::FabricHal> fabric_hal_;
    std::unique_ptr<hal::LineCardHal> line_card_hal_;
    std::unique_ptr<hal::PortHal> port_hal_;
    std::unique_ptr<p4runtime::P4Manager> p4_manager_;
    std::unique_ptr<bgp::Bgpmonitor> bgp_monitor_;
    std::unique_ptr<mpls::LspManager> lsp_manager_;
    std::unique_ptr<srv6::Srv6Manager> srv6_manager_;

    // Health monitor
    std::atomic<bool> monitor_running_{false};
    std::thread monitor_thread_;
    std::string config_path_;
};

} // namespace mts::cr9000::service
