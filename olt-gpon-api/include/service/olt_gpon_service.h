/**
 * MTS-OLT-2000 OLT GPON — Service Header
 * gRPC service implementation for MTS-OLT-2000
 * 
 * Implements MtsOltGponService with 10 RPC methods:
 * - GetOltStatus, GetPonPorts, GetOnuList, GetOnuStatus
 * - UpdateOnuConfig, ResetOnu
 * - GetOmcisStatus, GetTr069Config
 * - GetDeviceHealth, SubscribeTelemetry
 */

#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <google/protobuf/empty.pb.h>

#include "proto/mts_olt_gpon.grpc.pb.h"
#include "proto/mts_olt_gpon.pb.h"

#include "hal/gpon_hal.h"
#include "hal/onu_hal.h"
#include "hal/omci_hal.h"
#include "hal/tr069_hal.h"

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace mts::olt2000::service {

class OltGponService final : public MtsOltGponService::Service {
public:
    OltGponService();
    ~OltGponService() override = default;

    grpc::Status GetOltStatus(grpc::ServerContext* ctx,
                               const google::protobuf::Empty* req,
                               OltStatusResponse* resp) override;
    
    grpc::Status GetPonPorts(grpc::ServerContext* ctx,
                              const google::protobuf::Empty* req,
                              PonPortInfoResponse* resp) override;
    
    grpc::Status GetOnuList(grpc::ServerContext* ctx,
                             const OnuListRequest* req,
                             grpc::ServerWriter<OnuInfo>* writer) override;
    
    grpc::Status GetOnuStatus(grpc::ServerContext* ctx,
                               const OnuStatusRequest* req,
                               OnuStatusResponse* resp) override;
    
    grpc::Status UpdateOnuConfig(grpc::ServerContext* ctx,
                                  const UpdateOnuConfigRequest* req,
                                  UpdateOnuConfigResponse* resp) override;
    
    grpc::Status ResetOnu(grpc::ServerContext* ctx,
                          const ResetOnuRequest* req,
                          ResetOnuResponse* resp) override;
    
    grpc::Status GetOmcisStatus(grpc::ServerContext* ctx,
                                 const google::protobuf::Empty* req,
                                 OmcisStatusResponse* resp) override;
    
    grpc::Status GetTr069Config(grpc::ServerContext* ctx,
                                 const google::protobuf::Empty* req,
                                 Tr069ConfigResponse* resp) override;
    
    grpc::Status GetDeviceHealth(grpc::ServerContext* ctx,
                                  const google::protobuf::Empty* req,
                                  DeviceHealthResponse* resp) override;
    
    grpc::Status SubscribeTelemetry(grpc::ServerContext* ctx,
                                     const TelemetrySubscription* req,
                                     grpc::ServerWriter<TelemetryData>* writer) override;

    void startHealthMonitor();
    void stopHealthMonitor();

private:
    DeviceHealth createHealthResponse();
    TelemetryData createTelemetryData();

    // HAL instances
    std::unique_ptr<hal::GponHal> gpon_hal_;
    std::unique_ptr<hal::OnuHal> onu_hal_;
    std::unique_ptr<hal::OmciHal> omci_hal_;
    std::unique_ptr<hal::Tr069Hal> tr069_hal_;

    // Health monitor
    std::atomic<bool> monitor_running_{false};
    std::thread monitor_thread_;
};

} // namespace mts::olt2000::service
