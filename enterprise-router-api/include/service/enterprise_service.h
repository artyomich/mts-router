/**
 * MTS-ER-1000 Enterprise Router — Service Header
 * gRPC service implementation for MTS-ER-1000
 * 
 * Implements MtsEnterpriseService with 10 RPC methods:
 * - GetSdwanStatus, CreateSdwanPath, UpdateSdwanPath
 * - GetMplsLspStatus, CreateMplsLsp
 * - GetIpsecTunnels, CreateIpsecTunnel
 * - GetVrrpStatus, GetDeviceHealth, SubscribeTelemetry
 */

#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <google/protobuf/empty.pb.h>

#include "proto/mts_enterprise.grpc.pb.h"
#include "proto/mts_enterprise.pb.h"

#include "hal/sdwan_hal.h"
#include "hal/mpls_hal.h"
#include "hal/ipsec_hal.h"
#include "hal/vrrp_hal.h"

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace mts::er1000::service {

class EnterpriseService final : public MtsEnterpriseService::Service {
public:
    EnterpriseService();
    ~EnterpriseService() override = default;

    grpc::Status GetSdwanStatus(grpc::ServerContext* ctx,
                                 const google::protobuf::Empty* req,
                                 SdwanStatusResponse* resp) override;
    
    grpc::Status CreateSdwanPath(grpc::ServerContext* ctx,
                                  const CreateSdwanPathRequest* req,
                                  CreateSdwanPathResponse* resp) override;
    
    grpc::Status UpdateSdwanPath(grpc::ServerContext* ctx,
                                  const UpdateSdwanPathRequest* req,
                                  UpdateSdwanPathResponse* resp) override;
    
    grpc::Status GetMplsLspStatus(grpc::ServerContext* ctx,
                                   const google::protobuf::Empty* req,
                                   MplsLspStatusResponse* resp) override;
    
    grpc::Status CreateMplsLsp(grpc::ServerContext* ctx,
                                const CreateMplsLspRequest* req,
                                CreateMplsLspResponse* resp) override;
    
    grpc::Status GetIpsecTunnels(grpc::ServerContext* ctx,
                                  const google::protobuf::Empty* req,
                                  IpsecTunnelResponse* resp) override;
    
    grpc::Status CreateIpsecTunnel(grpc::ServerContext* ctx,
                                    const CreateIpsecTunnelRequest* req,
                                    CreateIpsecTunnelResponse* resp) override;
    
    grpc::Status GetVrrpStatus(grpc::ServerContext* ctx,
                                const google::protobuf::Empty* req,
                                VrrpStatusResponse* resp) override;
    
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
    std::unique_ptr<hal::SdwanHal> sdwan_hal_;
    std::unique_ptr<hal::MplsHal> mpls_hal_;
    std::unique_ptr<hal::IpsecHal> ipsec_hal_;
    std::unique_ptr<hal::VrrpHal> vrrp_hal_;

    // Health monitor
    std::atomic<bool> monitor_running_{false};
    std::thread monitor_thread_;
};

} // namespace mts::er1000::service
