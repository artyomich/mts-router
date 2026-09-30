/*
 * mbs_service.h — Service layer for MTS-MB-3000 gRPC API
 *
 * MTS Mobile Backhaul — Service layer implementation
 */

#ifndef MBS_SERVICE_H
#define MBS_SERVICE_H

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <google/protobuf/empty.pb.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>

/* Generated headers */
#include "mts_mobile_backhaul.grpc.pb.h"
#include "mts_mobile_backhaul.pb.h"

/* HAL headers */
#include "mbs_hal.h"
#include "mbs_mpls_tp.h"
#include "mbs_ptp.h"
#include "mbs_dpdk.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Service configuration */
typedef struct {
    int port;
    bool tls_enabled;
    char tls_cert_path[512];
    char tls_key_path[512];
    char tls_ca_path[512];
    char log_level[16];
    char log_file[512];
    uint32_t telemetry_interval_ms;
    bool delta_encoding;
} mbs_service_config_t;

/* Service state */
typedef struct {
    grpc::Server *server;
    grpc::ServerContext context;
    mbs_service_config_t config;
    bool is_running;
    bool is_initialized;
} mbs_service_t;

/* ==================== Service Operations ==================== */

/* Initialize service */
int mbs_service_init(mbs_service_t *service, const mbs_service_config_t *config);

/* Start service */
int mbs_service_start(mbs_service_t *service);

/* Stop service */
int mbs_service_stop(mbs_service_t *service);

/* Destroy service */
int mbs_service_destroy(mbs_service_t *service);

/* ==================== gRPC Service Implementation ==================== */

/* MPLS-TP Tunnel service methods */
grpc::Status mbs_get_mpls_tp_tunnels(grpc::ServerContext *context,
                                       const GetMplsTpTunnelsRequest *request,
                                       MplsTpTunnelResponse *response);

grpc::Status mbs_set_mpls_tp_tunnel(grpc::ServerContext *context,
                                      const MplsTpTunnelConfig *request,
                                      MplsTpTunnelConfigResponse *response);

grpc::Status mbs_delete_mpls_tp_tunnel(grpc::ServerContext *context,
                                         const MplsTpTunnelDeleteRequest *request,
                                         DeleteResponse *response);

/* MPLS-TP PW service methods */
grpc::Status mbs_get_mpls_tp_pws(grpc::ServerContext *context,
                                   const GetMplsTpPwsRequest *request,
                                   MplsTpPwResponse *response);

grpc::Status mbs_set_mpls_tp_pw(grpc::ServerContext *context,
                                 const MplsTpPwConfig *request,
                                 MplsTpPwConfigResponse *response);

grpc::Status mbs_delete_mpls_tp_pw(grpc::ServerContext *context,
                                     const MplsTpPwDeleteRequest *request,
                                     DeleteResponse *response);

/* MPLS-TP OAM service methods */
grpc::Status mbs_get_mpls_tp_oam(grpc::ServerContext *context,
                                  const GetMplsTpOamRequest *request,
                                  MplsTpOamResponse *response);

grpc::Status mbs_start_mpls_tp_oam(grpc::ServerContext *context,
                                     const MplsTpOamConfig *request,
                                     MplsTpOamConfigResponse *response);

grpc::Status mbs_stop_mpls_tp_oam(grpc::ServerContext *context,
                                    const MplsTpOamStopRequest *request,
                                    DeleteResponse *response);

/* PTP service methods */
grpc::Status mbs_get_ptp_clocks(grpc::ServerContext *context,
                                 const GetPtpClocksRequest *request,
                                 PtpClockResponse *response);

grpc::Status mbs_set_ptp_profile(grpc::ServerContext *context,
                                  const PtpProfileConfig *request,
                                  PtpProfileConfigResponse *response);

grpc::Status mbs_get_ptp_profile(grpc::ServerContext *context,
                                  const GetPtpProfileRequest *request,
                                  PtpProfileResponse *response);

/* Port service methods */
grpc::Status mbs_get_port_statuses(grpc::ServerContext *context,
                                    const GetPortStatusesRequest *request,
                                    PortStatusResponse *response);

grpc::Status mbs_set_port_mode(grpc::ServerContext *context,
                                const PortModeConfig *request,
                                PortModeConfigResponse *response);

/* DPDK service methods */
grpc::Status mbs_get_dpdk_port_stats(grpc::ServerContext *context,
                                      const GetDpdkPortStatsRequest *request,
                                      DpdkPortStatsResponse *response);

grpc::Status mbs_set_dpdk_port_config(grpc::ServerContext *context,
                                       const DpdkPortConfig *request,
                                       DpdkPortConfigResponse *response);

/* Health service methods */
grpc::Status mbs_get_device_health(grpc::ServerContext *context,
                                    const GetDeviceHealthRequest *request,
                                    DeviceHealthResponse *response);

/* Telemetry streaming methods */
grpc::ServerReaderWriterInterface<PortStatusResponse, GetPortStatusesRequest>
*mbs_stream_port_stats(grpc::ServerContext *context);

grpc::ServerReaderWriterInterface<DeviceHealthResponse, GetDeviceHealthRequest>
*mbs_stream_health(grpc::ServerContext *context);

#ifdef __cplusplus
}
#endif

#endif /* MBS_SERVICE_H */
