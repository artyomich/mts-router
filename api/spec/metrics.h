/**
 * MTS Router — Prometheus Metrics Collector
 * Unified metrics for all devices
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>

namespace mts {
namespace metrics {

// ============================================================
// Counter — monotonically increasing metric
// ============================================================
class Counter {
public:
    explicit Counter(std::string name);
    ~Counter() = default;

    void Increment(uint64_t amount = 1);
    uint64_t Get();
    std::string GetName() const;
    std::string ToPrometheusString() const;

private:
    std::string name_;
    std::atomic<uint64_t> value_;
    mutable std::mutex mutex_;
};

// ============================================================
// Gauge — value that can go up and down
// ============================================================
class Gauge {
public:
    explicit Gauge(std::string name);
    ~Gauge() = default;

    void Set(double value);
    void Increment(double amount = 1.0);
    void Decrement(double amount = 1.0);
    double Get();
    std::string GetName() const;
    std::string ToPrometheusString() const;

private:
    std::string name_;
    double value_;
    mutable std::mutex mutex_;
};

// ============================================================
// Histogram — distribution of values
// ============================================================
class Histogram {
public:
    explicit Histogram(std::string name);
    ~Histogram() = default;

    void Observe(double value);
    double GetSum();
    uint64_t GetCount();
    double GetMean();
    std::string GetName() const;
    std::string ToPrometheusString() const;

private:
    std::string name_;
    double sum_;
    uint64_t count_;
    mutable std::mutex mutex_;
};

// ============================================================
// MetricsRegistry — manages all metrics
// ============================================================
class MetricsRegistry {
public:
    static MetricsRegistry& GetInstance();
    MetricsRegistry(const MetricsRegistry&) = delete;
    MetricsRegistry& operator=(const MetricsRegistry&) = delete;

    std::shared_ptr<Counter> CreateCounter(const std::string& name);
    std::shared_ptr<Gauge> CreateGauge(const std::string& name);
    std::shared_ptr<Histogram> CreateHistogram(const std::string& name);

    std::string GetAllPrometheusMetrics() const;
    void ExportToHttpServer(int port);

private:
    MetricsRegistry() = default;
    ~MetricsRegistry() = default;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Counter>> counters_;
    std::unordered_map<std::string, std::shared_ptr<Gauge>> gauges_;
    std::unordered_map<std::string, std::shared_ptr<Histogram>> histograms_;
};

// ============================================================
// Pre-defined metric names for all devices
// ============================================================
namespace device {

// Core Router (CR-9000)
constexpr const char* CR_BGP_PEERS_UP = "mts_cr_bgp_peers_up";
constexpr const char* CR_BGP_PEERS_DOWN = "mts_cr_bgp_peers_down";
constexpr const char* CR_LSP_COUNT = "mts_cr_lsp_count";
constexpr const char* CR_LSP_ACTIVE = "mts_cr_lsp_active";
constexpr const char* CR_SRV6_SID_COUNT = "mts_cr_srv6_sid_count";
constexpr const char* CR_PORT_ERRORS = "mts_cr_port_errors_total";
constexpr const char* CR_PORT_PACKETS = "mts_cr_port_packets_total";
constexpr const char* CR_TABLE_MISS = "mts_cr_table_miss_total";
constexpr const char* CR_P4_PIPELINE_LATENCY_US = "mts_cr_p4_pipeline_latency_us";

// Mobile Core (MC-5000)
constexpr const char* MC_SESSIONS_ACTIVE = "mts_mc_sessions_active";
constexpr const char* MC_SESSIONS_CREATED = "mts_mc_sessions_created_total";
constexpr const char* MC_SESSIONS_DELETED = "mts_mc_sessions_deleted_total";
constexpr const char* MC_UPF_BYTES = "mts_mc_upf_bytes_total";
constexpr const char* MC_UPF_PACKETS = "mts_mc_upf_packets_total";
constexpr const char* MC_PFCP_HEARTBEATS = "mts_mc_pfcp_heartbeats_total";
constexpr const char* MC_GTP_UPTUNNEL = "mts_mc_gtp_up_tunnel_count";
constexpr const char* MC_GTP_Downtunnel = "mts_mc_gtp_down_tunnel_count";

// OLT GPON (OLT-2000)
constexpr const char* OLT_ONU_COUNT = "mts_olt_onu_count";
constexpr const char* OLT_ONU_ONLINE = "mts_olt_onu_online";
constexpr const char* OLT_ONU_OFFLINE = "mts_olt_onu_offline";
constexpr const char* OLT_GPON_POWER_LEVEL = "mts_olt_gpon_power_level";
constexpr const char* OLT_GPON_DISTANCE = "mts_olt_gpon_distance";
constexpr const char* OLT_OMCI_EVENTS = "mts_olt_omci_events_total";
constexpr const char* OLT_TR069_REQUESTS = "mts_olt_tr069_requests_total";
constexpr const char* OLT_TR069_ERRORS = "mts_olt_tr069_errors_total";

// Enterprise Router (ER-1000)
constexpr const char* ER_SDWAN_TUNNELS = "mts_er_sdwan_tunnel_count";
constexpr const char* ER_SDWAN_ACTIVE = "mts_er_sdwan_active";
constexpr const char* ER_MPLS_LSP_COUNT = "mts_er_mpls_lsp_count";
constexpr const char* ER_IPSEC_TUNNELS = "mts_er_ipsec_tunnel_count";
constexpr const char* ER_VRRP_STATE = "mts_er_vrrp_state";
constexpr const char* ER_BFD_SESSIONS = "mts_er_bfd_sessions";

// Residential Gateway (RG-500)
constexpr const char* RG_WIFI_CLIENTS = "mts_rg_wifi_clients";
constexpr const char* RG_WIFI_SIGNAL_STRENGTH = "mts_rg_wifi_signal_dbm";
constexpr const char* RG_VOIP_CALLS = "mts_rg_voip_calls_active";
constexpr const char* RG_VOIP_MOS_SCORE = "mts_rg_voip_mos_score";
constexpr const char* RG_IPTV_CHANNELS = "mts_rg_iptv_channels";
constexpr const char* RG_ONU_POWER = "mts_rg_onu_power_level";
constexpr const char* RG_TR069_REQUESTS = "mts_rg_tr069_requests_total";

} // namespace device

} // namespace metrics
} // namespace mts
