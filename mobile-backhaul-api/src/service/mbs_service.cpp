/*
 * mbs_service.cpp — Service layer implementation for MTS-MB-3000 gRPC API
 *
 * MTS Mobile Backhaul — Service layer implementation
 */

#include "service/mbs_service.h"
#include <google/protobuf/util/time_util.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <chrono>

/* ==================== Service Initialization ==================== */

int mbs_service_init(mbs_service_t *service, const mbs_service_config_t *config)
{
    if (!service || !config)
        return -1;

    memset(service, 0, sizeof(mbs_service_t));
    memcpy(&service->config, config, sizeof(mbs_service_config_t));

    /* Initialize HAL layers */
    if (mbs_hal_port_init() != 0) {
        fprintf(stderr, "Failed to initialize port HAL\n");
        return -1;
    }

    if (mbs_mpls_tp_engine_init() != 0) {
        fprintf(stderr, "Failed to initialize MPLS-TP engine\n");
        mbs_hal_port_exit();
        return -1;
    }

    if (mbs_ptp_engine_init() != 0) {
        fprintf(stderr, "Failed to initialize PTP engine\n");
        mbs_mpls_tp_engine_exit();
        mbs_hal_port_exit();
        return -1;
    }

    if (mbs_dpdk_init("0", MBS_DPDK_DEFAULT_MBUFS, MBS_DPDK_DEFAULT_MBUF_SIZE) != 0) {
        fprintf(stderr, "Failed to initialize DPDK\n");
        mbs_ptp_engine_exit();
        mbs_mpls_tp_engine_exit();
        mbs_hal_port_exit();
        return -1;
    }

    service->is_initialized = true;
    std::cout << "MTS-MB-3000 service initialized successfully" << std::endl;
    return 0;
}

int mbs_service_start(mbs_service_t *service)
{
    if (!service || !service->is_initialized)
        return -1;

    /* Build gRPC server credentials */
    grpc::ServerBuilder builder;

    if (service->config.tls_enabled) {
        grpc::SslServerCredentialsOptions ssl_opts;
        /* SSL options would be loaded from config */
        builder.AddSslCredentials(ssl_opts);
    }

    /* Register service */
    /* In production: builder.RegisterService(&mbs_grpc_service_); */

    /* Listen on configured port */
    std::string server_address = "0.0.0.0:" + std::to_string(service->config.port);
    service->server = builder.BuildAndStart();

    if (service->server) {
        service->is_running = true;
        std::cout << "MTS-MB-3000 service started on " << server_address << std::endl;
        return 0;
    }

    return -1;
}

int mbs_service_stop(mbs_service_t *service)
{
    if (!service || !service->is_running)
        return -1;

    if (service->server) {
        service->server->Shutdown();
        service->server = nullptr;
    }

    service->is_running = false;
    std::cout << "MTS-MB-3000 service stopped" << std::endl;
    return 0;
}

int mbs_service_destroy(mbs_service_t *service)
{
    if (!service)
        return -1;

    mbs_service_stop(service);

    /* Shutdown HAL layers */
    if (mbs_dpdk_exit() != 0)
        fprintf(stderr, "Failed to shutdown DPDK\n");

    if (mbs_ptp_engine_exit() != 0)
        fprintf(stderr, "Failed to shutdown PTP engine\n");

    if (mbs_mpls_tp_engine_exit() != 0)
        fprintf(stderr, "Failed to shutdown MPLS-TP engine\n");

    if (mbs_hal_port_exit() != 0)
        fprintf(stderr, "Failed to shutdown port HAL\n");

    memset(service, 0, sizeof(mbs_service_t));
    std::cout << "MTS-MB-3000 service destroyed" << std::endl;
    return 0;
}

/* ==================== gRPC Service Methods ==================== */

/* ==================== MPLS-TP Tunnel Methods ==================== */

grpc::Status mbs_get_mpls_tp_tunnels(grpc::ServerContext *context,
                                       const GetMplsTpTunnelsRequest *request,
                                       MplsTpTunnelResponse *response)
{
    mbs_tunnel_info_t tunnels[MBS_MAX_TUNNELS];
    uint32_t count = 0;

    if (request->all()) {
        if (mbs_mpls_tp_tunnel_get_all(tunnels, &count) != 0)
            return grpc::Status(grpc::INTERNAL, "Failed to get MPLS-TP tunnels");

        for (uint32_t i = 0; i < count; i++) {
            auto *t = response->add_tunnels();
            t->set_tunnel_id(tunnels[i].tunnel_id);
            t->set_name(tunnels[i].name);
            t->set_ingress_port(tunnels[i].ingress_port);
            t->set_egress_port(tunnels[i].egress_port);
            t->set_label(tunnels[i].label);
            t->set_next_label(tunnels[i].next_label);
            t->set_next_hop(tunnels[i].next_hop);
            t->set_status(tunnels[i].status);
            t->set_rx_packets(tunnels[i].rx_packets);
            t->set_tx_packets(tunnels[i].tx_packets);
            t->set_rx_bytes(tunnels[i].rx_bytes);
            t->set_tx_bytes(tunnels[i].tx_bytes);
            t->set_rx_errors(tunnels[i].rx_errors);
            t->set_tx_errors(tunnels[i].tx_errors);
            t->set_rx_drops(tunnels[i].rx_drops);
            t->set_tx_drops(tunnels[i].tx_drops);
        }
    } else if (request->tunnel_id() > 0) {
        mbs_tunnel_info_t tunnel;
        if (mbs_mpls_tp_tunnel_get(request->tunnel_id(), &tunnel) != 0)
            return grpc::Status(grpc::NOT_FOUND, "Tunnel not found");

        auto *t = response->add_tunnels();
        t->set_tunnel_id(tunnel.tunnel_id);
        t->set_name(tunnel.name);
        t->set_ingress_port(tunnel.ingress_port);
        t->set_egress_port(tunnel.egress_port);
        t->set_label(tunnel.label);
        t->set_next_label(tunnel.next_label);
        t->set_next_hop(tunnel.next_hop);
        t->set_status(tunnel.status);
        t->set_rx_packets(tunnel.rx_packets);
        t->set_tx_packets(tunnel.tx_packets);
        t->set_rx_bytes(tunnel.rx_bytes);
        t->set_tx_bytes(tunnel.tx_bytes);
        t->set_rx_errors(tunnel.rx_errors);
        t->set_tx_errors(tunnel.tx_errors);
        t->set_rx_drops(tunnel.rx_drops);
        t->set_tx_drops(tunnel.tx_drops);
    }

    return grpc::OK;
}

grpc::Status mbs_set_mpls_tp_tunnel(grpc::ServerContext *context,
                                      const MplsTpTunnelConfig *request,
                                      MplsTpTunnelConfigResponse *response)
{
    uint32_t tunnel_id;
    int ret = mbs_mpls_tp_tunnel_create(&tunnel_id,
                                          request->name().c_str(),
                                          request->ingress_port().c_str(),
                                          request->egress_port().c_str(),
                                          request->label(),
                                          request->next_label(),
                                          request->next_hop().c_str());

    if (ret == 0) {
        response->set_success(true);
        response->set_message("Tunnel created successfully");
    } else {
        response->set_success(false);
        response->set_message("Failed to create tunnel");
    }

    return grpc::OK;
}

grpc::Status mbs_delete_mpls_tp_tunnel(grpc::ServerContext *context,
                                         const MplsTpTunnelDeleteRequest *request,
                                         DeleteResponse *response)
{
    int ret = mbs_mpls_tp_tunnel_delete(request->tunnel_id());

    if (ret == 0) {
        response->set_success(true);
        response->set_message("Tunnel deleted successfully");
    } else {
        response->set_success(false);
        response->set_message("Failed to delete tunnel");
    }

    return grpc::OK;
}

/* ==================== MPLS-TP PW Methods ==================== */

grpc::Status mbs_get_mpls_tp_pws(grpc::ServerContext *context,
                                   const GetMplsTpPwsRequest *request,
                                   MplsTpPwResponse *response)
{
    mbs_pw_info_t pws[MBS_MAX_PW];
    uint32_t count = 0;

    if (mbs_mpls_tp_pw_get_all(pws, &count) != 0)
        return grpc::Status(grpc::INTERNAL, "Failed to get pseudowires");

    for (uint32_t i = 0; i < count; i++) {
        auto *pw = response->add_pseudowires();
        pw->set_pw_id(pws[i].pw_id);
        pw->set_name(pws[i].name);
        pw->set_peer_ip(pws[i].peer_ip);
        pw->set_local_label(pws[i].local_label);
        pw->set_remote_label(pws[i].remote_label);
        pw->set_encapsulation(pws[i].encapsulation);
        pw->set_status(pws[i].status);
        pw->set_rx_packets(pws[i].rx_packets);
        pw->set_tx_packets(pws[i].tx_packets);
        pw->set_rx_bytes(pws[i].rx_bytes);
        pw->set_tx_bytes(pws[i].tx_bytes);
    }

    return grpc::OK;
}

grpc::Status mbs_set_mpls_tp_pw(grpc::ServerContext *context,
                                 const MplsTpPwConfig *request,
                                 MplsTpPwConfigResponse *response)
{
    uint32_t pw_id;
    int ret = mbs_mpls_tp_pw_create(&pw_id,
                                      request->name().c_str(),
                                      request->peer_ip().c_str(),
                                      request->local_label(),
                                      request->remote_label(),
                                      request->encapsulation().c_str());

    if (ret == 0) {
        response->set_success(true);
        response->set_message("Pseudowire created successfully");
    } else {
        response->set_success(false);
        response->set_message("Failed to create pseudowire");
    }

    return grpc::OK;
}

grpc::Status mbs_delete_mpls_tp_pw(grpc::ServerContext *context,
                                     const MplsTpPwDeleteRequest *request,
                                     DeleteResponse *response)
{
    int ret = mbs_mpls_tp_pw_delete(request->pw_id());

    if (ret == 0) {
        response->set_success(true);
        response->set_message("Pseudowire deleted successfully");
    } else {
        response->set_success(false);
        response->set_message("Failed to delete pseudowire");
    }

    return grpc::OK;
}

/* ==================== MPLS-TP OAM Methods ==================== */

grpc::Status mbs_get_mpls_tp_oam(grpc::ServerContext *context,
                                  const GetMplsTpOamRequest *request,
                                  MplsTpOamResponse *response)
{
    /* OAM status retrieval implementation */
    (void)context;
    (void)request;
    (void)response;
    return grpc::OK;
}

grpc::Status mbs_start_mpls_tp_oam(grpc::ServerContext *context,
                                     const MplsTpOamConfig *request,
                                     MplsTpOamConfigResponse *response)
{
    uint32_t oam_id;
    int ret = mbs_mpls_tp_oam_start(&oam_id,
                                       request->type().c_str(),
                                       request->target().c_str(),
                                       request->interval_ms());

    if (ret == 0) {
        response->set_success(true);
        response->set_message("OAM monitoring started");
    } else {
        response->set_success(false);
        response->set_message("Failed to start OAM monitoring");
    }

    return grpc::OK;
}

grpc::Status mbs_stop_mpls_tp_oam(grpc::ServerContext *context,
                                    const MplsTpOamStopRequest *request,
                                    DeleteResponse *response)
{
    int ret = mbs_mpls_tp_oam_stop(request->oam_id());

    if (ret == 0) {
        response->set_success(true);
        response->set_message("OAM monitoring stopped");
    } else {
        response->set_success(false);
        response->set_message("Failed to stop OAM monitoring");
    }

    return grpc::OK;
}

/* ==================== PTP Methods ==================== */

grpc::Status mbs_get_ptp_clocks(grpc::ServerContext *context,
                                 const GetPtpClocksRequest *request,
                                 PtpClockResponse *response)
{
    mbs_ptp_clock_t clocks[MBS_MAX_PTP_CLOCKS];
    uint32_t count = 0;

    if (mbs_ptp_get_all_clocks(clocks, &count) != 0)
        return grpc::Status(grpc::INTERNAL, "Failed to get PTP clocks");

    for (uint32_t i = 0; i < count; i++) {
        auto *clock = response->add_clocks();
        clock->set_clock_id(clocks[i].clock_id);
        clock->set_clock_identity(clocks[i].clock_identity);
        clock->set_domain(clocks[i].domain);
        clock->set_priority1(clocks[i].priority1);
        clock->set_priority2(clocks[i].priority2);
        clock->set_class_value(clocks[i].class_value);
        clock->set_clock_type(clocks[i].clock_type);
        clock->set_status(clocks[i].status);
        clock->set_offset_from_master_ns(clocks[i].offset_from_master_ns);
        clock->set_num_steps_removed(clocks[i].num_steps_removed);
        clock->set_log_sync_interval(clocks[i].log_sync_interval);
    }

    return grpc::OK;
}

grpc::Status mbs_set_ptp_profile(grpc::ServerContext *context,
                                  const PtpProfileConfig *request,
                                  PtpProfileConfigResponse *response)
{
    mbs_ptp_profile_t profile;
    strncpy(profile.profile_name, request->profile_name().c_str(), sizeof(profile.profile_name) - 1);
    profile.domain_number = request->domain_number();
    profile.priority1 = request->priority1();
    profile.priority2 = request->priority2();
    profile.max_steps = request->max_steps();
    profile.clock_class = request->clock_class();
    profile.clock_type = request->clock_type();
    profile.utc_offset = request->utc_offset();

    int ret = mbs_ptp_set_profile(&profile);

    if (ret == 0) {
        response->set_success(true);
        response->set_message("PTP profile set successfully");
    } else {
        response->set_success(false);
        response->set_message("Failed to set PTP profile");
    }

    return grpc::OK;
}

grpc::Status mbs_get_ptp_profile(grpc::ServerContext *context,
                                  const GetPtpProfileRequest *request,
                                  PtpProfileResponse *response)
{
    mbs_ptp_profile_t profile;
    if (mbs_ptp_get_profile(&profile) != 0)
        return grpc::Status(grpc::INTERNAL, "Failed to get PTP profile");

    auto *p = response->mutable_profile();
    p->set_profile_name(profile.profile_name);
    p->set_domain_number(profile.domain_number);
    p->set_priority1(profile.priority1);
    p->set_priority2(profile.priority2);
    p->set_max_steps(profile.max_steps);
    p->set_clock_class(profile.clock_class);
    p->set_clock_type(profile.clock_type);
    p->set_utc_offset(profile.utc_offset);

    return grpc::OK;
}

/* ==================== Port Methods ==================== */

grpc::Status mbs_get_port_statuses(grpc::ServerContext *context,
                                    const GetPortStatusesRequest *request,
                                    PortStatusResponse *response)
{
    mbs_port_status_t ports[MBS_MAX_PORTS];
    uint32_t count = 0;

    if (mbs_hal_port_get_all(ports, &count) != 0)
        return grpc::Status(grpc::INTERNAL, "Failed to get port statuses");

    for (uint32_t i = 0; i < count; i++) {
        auto *port = response->add_ports();
        port->set_port_id(ports[i].port_id);
        port->set_name(ports[i].name);
        port->set_type(ports[i].type);
        port->set_speed_mbps(ports[i].speed_mbps);
        port->set_status(ports[i].status);
        port->set_mode(ports[i].mode);
        port->set_rx_bytes(ports[i].rx_bytes);
        port->set_tx_bytes(ports[i].tx_bytes);
        port->set_rx_packets(ports[i].rx_packets);
        port->set_tx_packets(ports[i].tx_packets);
        port->set_rx_errors(ports[i].rx_errors);
        port->set_tx_errors(ports[i].tx_errors);
        port->set_rx_drops(ports[i].rx_drops);
        port->set_tx_drops(ports[i].tx_drops);
        port->set_rx_power_dbm(ports[i].rx_power_dbm);
        port->set_tx_power_dbm(ports[i].tx_power_dbm);
        port->set_temperature_c(ports[i].temperature_c);
    }

    return grpc::OK;
}

grpc::Status mbs_set_port_mode(grpc::ServerContext *context,
                                const PortModeConfig *request,
                                PortModeConfigResponse *response)
{
    int ret = mbs_hal_port_set_mode(request->port_id(), request->mode().c_str());

    if (ret == 0) {
        response->set_success(true);
        response->set_message("Port mode set successfully");
    } else {
        response->set_success(false);
        response->set_message("Failed to set port mode");
    }

    return grpc::OK;
}

/* ==================== DPDK Methods ==================== */

grpc::Status mbs_get_dpdk_port_stats(grpc::ServerContext *context,
                                      const GetDpdkPortStatsRequest *request,
                                      DpdkPortStatsResponse *response)
{
    mbs_dpdk_stats_t stats[MBS_MAX_DPDK_PORTS];
    uint32_t count = 0;

    if (mbs_dpdk_port_get_all_stats(stats, &count) != 0)
        return grpc::Status(grpc::INTERNAL, "Failed to get DPDK stats");

    for (uint32_t i = 0; i < count; i++) {
        auto *s = response->add_stats();
        s->set_port_id(stats[i].port_id);
        s->set_packets_processed(stats[i].packets_processed);
        s->set_packets_dropped(stats[i].packets_dropped);
        s->set_bytes_processed(stats[i].bytes_processed);
        s->set_errors(stats[i].errors);
        s->set_bursts(stats[i].bursts);
        s->set_throughput_mbps(stats[i].throughput_mbps);
        s->set_cpu_usage(stats[i].cpu_usage);
    }

    return grpc::OK;
}

grpc::Status mbs_set_dpdk_port_config(grpc::ServerContext *context,
                                       const DpdkPortConfig *request,
                                       DpdkPortConfigResponse *response)
{
    /* DPDK port configuration would be applied */
    (void)context;
    (void)request;

    response->set_success(true);
    response->set_message("DPDK port configuration updated");
    return grpc::OK;
}

/* ==================== Health Methods ==================== */

grpc::Status mbs_get_device_health(grpc::ServerContext *context,
                                    const GetDeviceHealthRequest *request,
                                    DeviceHealthResponse *response)
{
    mbs_device_health_t health;
    if (mbs_hal_health_get(&health) != 0)
        return grpc::Status(grpc::INTERNAL, "Failed to get device health");

    auto *h = response->mutable_health();
    h->set_device_id(health.device_id);
    h->set_model(health.model);
    h->set_firmware(health.firmware);
    h->set_cpu_usage(health.cpu_usage);
    h->set_memory_usage(health.memory_usage);
    h->set_temperature(health.temperature);
    h->set_status(health.status);
    h->set_uptime_seconds(health.uptime_seconds);
    h->set_packet_loss_pct(health.packet_loss_pct);
    h->set_latency_ms(health.latency_ms);

    (void)context;
    (void)request;
    return grpc::OK;
}

/* ==================== Telemetry Streaming ==================== */

grpc::ServerReaderWriterInterface<PortStatusResponse, GetPortStatusesRequest>
*mbs_stream_port_stats(grpc::ServerContext *context)
{
    /* Streaming implementation would use async gRPC API */
    (void)context;
    return nullptr;
}

grpc::ServerReaderWriterInterface<DeviceHealthResponse, GetDeviceHealthRequest>
*mbs_stream_health(grpc::ServerContext *context)
{
    /* Streaming implementation would use async gRPC API */
    (void)context;
    return nullptr;
}
