/**
 * MTS-MB-3000 Mobile Backhaul - gRPC Service
 * 
 * Реализация gRPC сервиса для управления Mobile Backhaul
 * Интеграция с HAL слоями:
 * - PtpHal - PTP синхронизация
 * - SyncEHal - SyncE синхронизация
 * - MplsTpHal - MPLS-TP pseudowires
 * 
 * Уровень реализации:
 * - gRPC сервер на C++
 * - Thread-safe доступ к HAL
 * - Поддержка streaming telemetry
 */

#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/empty.pb.h>

#include "hal/ptp_hal.h"
#include "hal/sync_e_hal.h"
#include "hal/mpls_hal.h"

#include "mts_backhaul.grpc.pb.h"
#include "mts_backhaul.pb.h"

namespace mts {
namespace service {

// gRPC service implementation
class MtsBackhaulServiceImpl final : public mts::backhaul::v1::MtsBackhaulService::Service {
public:
    MtsBackhaulServiceImpl();
    ~MtsBackhaulServiceImpl() override;

    // PTP status
    grpc::Status GetPtpStatus(grpc::ServerContext* context,
                              const mts::backhaul::v1::Empty* request,
                              mts::backhaul::v1::PtpStatusResponse* response) override;

    // SyncE status
    grpc::Status GetSyncEStatus(grpc::ServerContext* context,
                                const mts::backhaul::v1::Empty* request,
                                mts::backhaul::v1::SyncEStatusResponse* response) override;

    // MPLS-TP pseudowires
    grpc::Status GetMplsTpPwStatus(grpc::ServerContext* context,
                                   const mts::backhaul::v1::Empty* request,
                                   mts::backhaul::v1::MplsTpPwResponse* response) override;

    // Port status
    grpc::Status GetPortStatus(grpc::ServerContext* context,
                               const mts::backhaul::v1::Empty* request,
                               mts::backhaul::v1::PortStatusResponse* response) override;

    // Device health
    grpc::Status GetDeviceHealth(grpc::ServerContext* context,
                                 const mts::backhaul::v1::Empty* request,
                                 mts::backhaul::v1::DeviceHealthResponse* response) override;

    // Set grandmaster
    grpc::Status SetGrandmaster(grpc::ServerContext* context,
                                const mts::backhaul::v1::SetGrandmasterRequest* request,
                                mts::backhaul::v1::SetGrandmasterResponse* response) override;

    // Create pseudowire
    grpc::Status CreatePw(grpc::ServerContext* context,
                          const mts::backhaul::v1::CreatePwRequest* request,
                          mts::backhaul::v1::CreatePwResponse* response) override;

    // Delete pseudowire
    grpc::Status DeletePw(grpc::ServerContext* context,
                          const mts::backhaul::v1::PwDeleteRequest* request,
                          mts::backhaul::v1::PwDeleteResponse* response) override;

    // Telemetry streaming
    grpc::Status SubscribeTelemetry(grpc::ServerContext* context,
                                    const mts::backhaul::v1::TelemetrySubscription* request,
                                    grpc::ServerWriter<mts::backhaul::v1::TelemetryData>* writer) override;

private:
    // HAL instances
    hal::PtpHal ptp_hal_;
    hal::SyncEHal sync_e_hal_;
    hal::MplsTpHal mpls_hal_;

    // Helper methods
    void populatePtpStatus(mts::backhaul::v1::PtpStatus* ptp_status);
    void populateSyncEStatus(mts::backhaul::v1::SyncEStatus* sync_e_status);
    void populateMplsTpPwStatus(mts::backhaul::v1::MplsTpPwStatus* pw_status);
    void populatePortStatus(mts::backhaul::v1::PortStatus* port_status);
    void populateDeviceHealth(mts::backhaul::v1::DeviceHealth* health);
};

} // namespace service
} // namespace mts
