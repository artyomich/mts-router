/*
 * mts-rest.h — Основные структуры REST API Gateway
 *
 * MTS Router — Единый REST API Gateway
 */

#ifndef MTS_REST_H
#define MTS_REST_H

#include <stdint.h>

#define MTS_REST_MAX_DEVICES 64
#define MTS_REST_MAX_INTERFACES 128
#define MTS_REST_MAX_ROUTES 256
#define MTS_REST_MAX_LOGS 1024
#define MTS_REST_MAX_CLIENTS 256
#define MTS_REST_MAX_API_KEYS 64

/* Device types */
enum mts_device_type {
    MTS_DEVICE_CORE_ROUTER = 1,
    MTS_DEVICE_MOBILE_CORE = 2,
    MTS_DEVICE_MOBILE_BACKHAUL = 3,
    MTS_DEVICE_OLT_GPON = 4,
    MTS_DEVICE_ENTERPRISE = 5,
    MTS_DEVICE_RESIDENTIAL = 6
};

/* Device status */
enum mts_device_status {
    MTS_DEVICE_OFFLINE = 0,
    MTS_DEVICE_ONLINE = 1,
    MTS_DEVICE_MAINTENANCE = 2,
    MTS_DEVICE_ERROR = 3
};

/* API key */
struct mts_api_key {
    char key[64];
    char name[64];
    int64_t created;
    int64_t expires;
    int64_t last_used;
    int64_t usage_count;
    int active;
};

/* REST device */
struct mts_rest_device {
    uint32_t id;
    char name[64];
    enum mts_device_type type;
    enum mts_device_status status;
    char ip_address[64];
    char firmware_version[32];
    int64_t last_seen;
    int64_t created;
    double cpu_usage;
    double memory_usage;
    uint32_t num_interfaces;
    uint32_t num_routes;
    uint32_t uptime_seconds;
};

/* REST interface */
struct mts_rest_interface {
    uint32_t id;
    char name[32];
    char mac[18];
    char ip_address[64];
    double speed;     /* Mbps */
    double duplex;    /* half/full */
    int32_t status;   /* UP/DOWN */
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_drops;
    uint64_t tx_drops;
};

/* REST route */
struct mts_rest_route {
    uint32_t id;
    char destination[64];
    char gateway[64];
    char interface[32];
    uint32_t metric;
    uint32_t type;    /* static/connected/ospf/bgp/isis */
    int32_t status;
    uint64_t packets;
    uint64_t bytes;
};

/* REST health */
struct mts_rest_health {
    int32_t cpu_temp;
    int32_t gpu_temp;
    int32_t fan_speed;
    int32_t voltage_core;
    int32_t voltage_mem;
    double cpu_usage;
    double memory_usage;
    double disk_usage;
    uint32_t load_avg_1;
    uint32_t load_avg_5;
    uint32_t load_avg_15;
    uint64_t uptime_seconds;
    int32_t battery_level; /* for portable devices */
};

/* REST log */
struct mts_rest_log {
    uint32_t id;
    int64_t timestamp;
    int32_t level;    /* debug/info/warning/error/critical */
    char component[32];
    char message[512];
};

/* REST performance */
struct mts_rest_performance {
    double cpu_usage;
    double memory_usage;
    double network_throughput;
    double packet_rate;
    double latency_ms;
    double packet_loss_pct;
    double qos_score;
};

/* REST response */
struct mts_rest_response {
    int32_t status_code;
    char status_text[32];
    char data[4096];
    char meta[512];
    char error_code[16];
    char error_message[256];
};

/* REST config */
struct mts_rest_config {
    char bind_address[64];
    uint16_t port;
    int32_t tls_enabled;
    char tls_cert[256];
    char tls_key[256];
    int32_t auth_enabled;
    int32_t rate_limit_enabled;
    uint32_t rate_limit_requests;
    uint32_t rate_limit_window;
    int32_t logging_enabled;
    int32_t metrics_enabled;
    char log_file[256];
};

#endif /* MTS_REST_H */
