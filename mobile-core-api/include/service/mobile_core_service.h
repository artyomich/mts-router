/**
 * MTS-MC-5000 Mobile Core — Service Header
 * gRPC service implementation for MTS-MC-5000
 * 
 * Implements MtsMobileCoreService with 12 RPC methods:
 * - GetUpfStatus — UPF status (CPU, memory, sessions, throughput)
 * - GetPduSessions — PDU session list
 * - CreatePduSession — create PDU session
 * - DeletePduSession — delete PDU session
 * - GetPfcpSessions — PFCP session list
 * - CreatePfcpSteering — PFCP steering rule
 * - GetGtpTunnels — GTP tunnel list
 * - GetFiveQIConfigs — standard 5QI configs (3GPP TS 23.501)
 * - UpdateFiveQI — update 5QI config
 * - GetNrfRegistry — 5GC NRF registry
 * - GetDeviceHealth — device health (CPU, memory, temp)
 * - SubscribeTelemetry — streaming telemetry
 */

#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <google/protobuf/empty.pb.h>

#include "proto/mts_mobile_core.grpc.pb.h"
#include "proto/mts_mobile_core.pb.h"

#include "hal/upf_hal.h"
#include "hal/sm_hal.h"
#include "hal/pfcp_hal.h"
#include "hal/gtp_hal.h"

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace mts::mc5000::service {

class MobileCoreService final : public MtsMobileCoreService::Service {
public:
    MobileCoreService();
    ~MobileCoreService() override = default;

    grpc::Status GetUpfStatus(grpc::ServerContext* ctx,
                               const google::protobuf::Empty* req,
                               UpfStatusResponse* resp) override;
    
    grpc::Status GetPduSessions(grpc::ServerContext* ctx,
                                 const google::protobuf::Empty* req,
                                 PduSessionResponse* resp) override;
    
    grpc::Status CreatePduSession(grpc::ServerContext* ctx,
                                   const CreatePduSessionRequest* req,
                                   CreatePduSessionResponse* resp) override;
    
    grpc::Status DeletePduSession(grpc::ServerContext* ctx,
                                   const DeletePduSessionRequest* req,
                                   DeletePduSessionResponse* resp) override;
    
    grpc::Status GetPfcpSessions(grpc::ServerContext* ctx,
                                  const google::protobuf::Empty* req,
                                  PfcpSessionResponse* resp) override;
    
    grpc::Status CreatePfcpSteering(grpc::ServerContext* ctx,
                                     const CreatePfcpSteeringRequest* req,
                                     CreatePfcpSteeringResponse* resp) override;
    
    grpc::Status GetGtpTunnels(grpc::ServerContext* ctx,
                                const google::protobuf::Empty* req,
                                GtpTunnelResponse* resp) override;
    
    grpc::Status GetFiveQIConfigs(grpc::ServerContext* ctx,
                                   const google::protobuf::Empty* req,
                                   FiveQIConfigResponse* resp) override;
    
    grpc::Status UpdateFiveQI(grpc::ServerContext* ctx,
                               const UpdateFiveQIRequest* req,
                               UpdateFiveQIResponse* resp) override;
    
    grpc::Status GetNrfRegistry(grpc::ServerContext* ctx,
                                  const google::protobuf::Empty* req,
                                  NrfRegistryResponse* resp) override;
    
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
    std::unique_ptr<hal::UpfHal> upf_hal_;
    std::unique_ptr<hal::SmfHal> smf_hal_;
    std::unique_ptr<hal::PfcpHal> pfcp_hal_;
    std::unique_ptr<hal::GtpHal> gtp_hal_;

    // Health monitor
    std::atomic<bool> monitor_running_{false};
    std::thread monitor_thread_;
};

} // namespace mts::mc5000::service
