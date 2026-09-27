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

#include "grpc_generated/mts_olt_gpon.grpc.pb.h"
#include "mts_olt_gpon.pb.h"

#include "hal/gpon_hal.h"
#include "hal/onu_hal.h"
#include "hal/omci_hal.h"
#include "hal/tr069_hal.h"

// C++ HAL wrappers
#include "hal/gpon_hal_cpp.h"
#include "hal/onu_hal_cpp.h"
#include "hal/omci_hal_cpp.h"
#include "hal/tr069_hal_cpp.h"

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace mts::olt2000::service {

class OltGponServiceImpl final : public mts::olt2000::OltGponService::Service {
public:
    OltGponServiceImpl();
    ~OltGponServiceImpl() override = default;

    grpc::Status GetOltStatus(grpc::ServerContext* ctx,
                               const Empty* req,
                               OltStatusResponse* resp) override;
    
    grpc::Status GetOnu(grpc::ServerContext* ctx,
                               const OnuIdRequest* req,
                               OnuResponse* resp) override;
    
    grpc::Status UpdateOnuConfig(grpc::ServerContext* ctx,
                                 const UpdateOnuConfigRequest* req,
                                 UpdateOnuConfigResponse* resp) override;
    
    grpc::Status ResetOnu(grpc::ServerContext* ctx,
                          const OnuIdRequest* req,
                          ResetOnuResponse* resp) override;
    
    grpc::Status GetOmciStatus(grpc::ServerContext* ctx,
                                 const Empty* req,
                                 OmciStatusResponse* resp) override;
    
    grpc::Status GetTr069Config(grpc::ServerContext* ctx,
                                 const Empty* req,
                                 Tr069ConfigResponse* resp) override;
    
    grpc::Status GetHealth(grpc::ServerContext* ctx,
                           const Empty* req,
                           DeviceHealthResponse* resp) override;
    
    grpc::Status HealthCheck(grpc::ServerContext* ctx,
                             const HealthCheckRequest* req,
                             HealthCheckResponse* resp) override;
    
    grpc::Status SetOnu(grpc::ServerContext* ctx,
                        const SetOnuRequest* req,
                        SetOnuResponse* resp) override;
    
    grpc::Status SetTr069Config(grpc::ServerContext* ctx,
                                 const Tr069ConfigRequest* req,
                                 Tr069ConfigResponse* resp) override;
    
    grpc::Status GetWdmStatus(grpc::ServerContext* ctx,
                              const Empty* req,
                              WdmStatusResponse* resp) override;
    
    grpc::Status ListOnus(grpc::ServerContext* ctx,
                          const Empty* req,
                          OnuListResponse* resp) override;

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
