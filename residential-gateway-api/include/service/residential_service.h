/**
 * MTS-RG-500 Residential Gateway — Service Header
 * gRPC service implementation for MTS-RG-500
 * 
 * Implements MtsResidentialService with 15 RPC methods:
 * - GetGponStatus, GetWifiStatus, UpdateWifi
 * - GetWifiClients, GetVoipStatus, UpdateVoip
 * - GetIptvStatus, GetTr069Status
 * - GetLanConfig, GetParentalControl, UpdateParentalControl
 * - BlockClient, UnblockClient
 * - GetDeviceHealth, SubscribeTelemetry
 */

#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <google/protobuf/empty.pb.h>

#include "proto/mts_residential.grpc.pb.h"
#include "proto/mts_residential.pb.h"

#include "hal/gpon_hal.h"
#include "hal/wifi_hal.h"
#include "hal/voip_hal.h"
#include "hal/iptv_hal.h"
#include "hal/tr069_hal.h"

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace mts::rg500::service {

class ResidentialService final : public MtsResidentialService::Service {
public:
    ResidentialService();
    ~ResidentialService() override = default;

    grpc::Status GetGponStatus(grpc::ServerContext* ctx,
                                const google::protobuf::Empty* req,
                                GponOnuStatusResponse* resp) override;
    
    grpc::Status GetWifiStatus(grpc::ServerContext* ctx,
                                const google::protobuf::Empty* req,
                                WifiBssInfoResponse* resp) override;
    
    grpc::Status UpdateWifi(grpc::ServerContext* ctx,
                             const UpdateWifiRequest* req,
                             UpdateWifiResponse* resp) override;
    
    grpc::Status GetWifiClients(grpc::ServerContext* ctx,
                                 const google::protobuf::Empty* req,
                                 WifiClientInfoResponse* resp) override;
    
    grpc::Status GetVoipStatus(grpc::ServerContext* ctx,
                                const google::protobuf::Empty* req,
                                VoipStatusResponse* resp) override;
    
    grpc::Status UpdateVoip(grpc::ServerContext* ctx,
                             const UpdateVoipRequest* req,
                             UpdateVoipResponse* resp) override;
    
    grpc::Status GetIptvStatus(grpc::ServerContext* ctx,
                                const google::protobuf::Empty* req,
                                IptvStatusResponse* resp) override;
    
    grpc::Status GetTr069Status(grpc::ServerContext* ctx,
                                 const google::protobuf::Empty* req,
                                 Tr069StatusResponse* resp) override;
    
    grpc::Status GetLanConfig(grpc::ServerContext* ctx,
                               const google::protobuf::Empty* req,
                               LanConfigResponse* resp) override;
    
    grpc::Status GetParentalControl(grpc::ServerContext* ctx,
                                     const google::protobuf::Empty* req,
                                     ParentalControlResponse* resp) override;
    
    grpc::Status UpdateParentalControl(grpc::ServerContext* ctx,
                                        const ParentalControl* req,
                                        BlockClientResponse* resp) override;
    
    grpc::Status BlockClient(grpc::ServerContext* ctx,
                              const BlockClientRequest* req,
                              BlockClientResponse* resp) override;
    
    grpc::Status UnblockClient(grpc::ServerContext* ctx,
                                const BlockClientRequest* req,
                                BlockClientResponse* resp) override;
    
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
    std::unique_ptr<hal::WifiHal> wifi_hal_;
    std::unique_ptr<hal::VoipHal> voip_hal_;
    std::unique_ptr<hal::IptvHal> iptv_hal_;
    std::unique_ptr<hal::Tr069Hal> tr069_hal_;

    // Health monitor
    std::atomic<bool> monitor_running_{false};
    std::thread monitor_thread_;
};

} // namespace mts::rg500::service
