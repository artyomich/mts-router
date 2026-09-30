/*
 * rg_hal.h — Hardware Abstraction Layer for MTS-RG-500
 *
 * MTS Residential Gateway (MTS-RG-500) — HAL для MT7981
 */

#ifndef RG_HAL_H
#define RG_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum values */
#define RG_MAX_WIFI_BANDS 2
#define RG_MAX_WIFI_CLIENTS 64
#define RG_MAX_VOIP_ACCOUNTS 4
#define RG_MAX_VOIP_CALLS 4
#define RG_MAX_IPTV_SESSIONS 8
#define RG_MAX_IPTV_CHANNELS 512

/* WiFi band constants */
#define RG_WIFI_BAND_2G 2400
#define RG_WIFI_BAND_5G 5000

/* WiFi modes */
#define RG_WIFI_MODE_B "b"
#define RG_WIFI_MODE_G "g"
#define RG_WIFI_MODE_N "n"
#define RG_WIFI_MODE_A "a"
#define RG_WIFI_MODE_AC "ac"
#define RG_WIFI_MODE_AX "ax"
#define RG_WIFI_MODE_BGN "bgn"
#define RG_WIFI_MODE_ANACAX "a/n/ac/ax"

/* WiFi security types */
#define RG_WIFI_SECURITY_OPEN "open"
#define RG_WIFI_SECURITY_WPA2_PSK "wpa2-psk"
#define RG_WIFI_SECURITY_WPA3_SAE "wpa3-sae"

/* Device health status */
#define RG_HEALTH_HEALTHY "healthy"
#define RG_HEALTH_DEGRADED "degraded"
#define RG_HEALTH_CRITICAL "critical"

/* ==================== WiFi ==================== */

typedef struct {
    uint32_t band;
    char ssid[64];
    char channel[16];
    char mode[16];
    char bandwidth[16];
    char security[16];
    char psk[128];
    bool hidden_ssid;
    bool guest_network;
    uint32_t max_clients;
    uint32_t connected_clients;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    double signal_strength;
    double interference;
} rg_wifi_ap_t;

typedef struct {
    uint32_t client_id;
    char mac_address[18];
    char ssid[64];
    uint32_t band;
    char signal_strength[16];
    uint32_t data_rate_mbps;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    int64_t connected_at;
    char ip_address[64];
} rg_wifi_client_t;

int rg_hal_wifi_init(void);
int rg_hal_wifi_exit(void);
int rg_hal_wifi_get_aps(rg_wifi_ap_t *aps, uint32_t *count);
int rg_hal_wifi_set_ap(uint32_t band, const rg_wifi_ap_t *ap);
int rg_hal_wifi_get_clients(rg_wifi_client_t *clients, uint32_t *count);
int rg_hal_wifi_set_security(uint32_t band, const char *security, const char *psk);
int rg_hal_wifi_set_channel(uint32_t band, const char *channel);

/* ==================== VoIP ==================== */

typedef struct {
    uint32_t account_id;
    char username[64];
    char display_name[64];
    char registrar[128];
    char auth_username[64];
    char auth_password[64];
    char transport[8];
    uint32_t port;
    char codec[16];
    bool enabled;
    char status[16];
    uint32_t reregister_interval;
} rg_voip_account_t;

typedef struct {
    uint32_t call_id;
    char caller[64];
    char callee[64];
    char status[16];
    int64_t started_at;
    int64_t duration_seconds;
    char codec[16];
    double jitter_ms;
    double packet_loss_pct;
    double mos_score;
} rg_voip_call_t;

int rg_hal_voip_init(void);
int rg_hal_voip_exit(void);
int rg_hal_voip_get_accounts(rg_voip_account_t *accounts, uint32_t *count);
int rg_hal_voip_set_account(const rg_voip_account_t *account);
int rg_hal_voip_delete_account(uint32_t account_id);
int rg_hal_voip_get_calls(rg_voip_call_t *calls, uint32_t *count);

/* ==================== IPTV ==================== */

typedef struct {
    uint32_t channel_id;
    char name[128];
    uint32_t number;
    char video_codec[16];
    uint32_t resolution;
    uint32_t fps;
    char audio_codec[16];
    char epg_id[64];
    uint64_t viewers;
} rg_iptv_channel_t;

typedef struct {
    uint32_t session_id;
    char mac_address[18];
    char ip_address[64];
    uint32_t channel_id;
    char status[16];
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    int64_t started_at;
} rg_iptv_session_t;

int rg_hal_iptv_init(void);
int rg_hal_iptv_exit(void);
int rg_hal_iptv_get_channels(rg_iptv_channel_t *channels, uint32_t *count);
int rg_hal_iptv_get_sessions(rg_iptv_session_t *sessions, uint32_t *count);
int rg_hal_iptv_start_session(const rg_iptv_session_t *session);
int rg_hal_iptv_stop_session(uint32_t session_id);

/* ==================== TR-069 ==================== */

typedef struct {
    char status[16];
    char acs_url[256];
    uint32_t acs_port;
    uint32_t connection_requests;
    uint32_t max_connections;
    uint32_t active_sessions;
    int64_t last_connection;
    int64_t next_connection;
} rg_tr069_status_t;

typedef struct {
    char firmware_url[512];
    char file_name[128];
    uint64_t file_size;
    char file_type[32];
    char version[64];
    char status[16];
    double progress_pct;
    int64_t started_at;
    int64_t completed_at;
} rg_firmware_status_t;

int rg_hal_tr069_init(void);
int rg_hal_tr069_exit(void);
int rg_hal_tr069_get_status(rg_tr069_status_t *status);
int rg_hal_tr069_set_config(const char *acs_url, uint32_t port,
                              const char *username, const char *password);
int rg_hal_firmware_get_status(rg_firmware_status_t *status);
int rg_hal_firmware_trigger_download(const char *url, const char *file_type);

/* ==================== GPON ONU ==================== */

typedef struct {
    uint32_t port_id;
    char name[32];
    uint32_t state;
    uint32_t mode;
    int32_t tx_power;
    int32_t rx_power;
    uint32_t num_onu;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t tx_packets;
    uint64_t rx_packets;
} rg_gpon_onu_t;

int rg_hal_gpon_init(void);
int rg_hal_gpon_exit(void);
int rg_hal_gpon_get_status(rg_gpon_onu_t *onu);
int rg_hal_gpon_configure(const rg_gpon_onu_t *onu);

/* ==================== Health ==================== */

typedef struct {
    char device_id[64];
    char model[32];
    char firmware[32];
    double cpu_usage;
    double memory_usage;
    double temperature;
    char status[16];
    uint64_t uptime_seconds;
    uint64_t wan_rx_bytes;
    uint64_t wan_tx_bytes;
    uint64_t lan_rx_bytes;
    uint64_t lan_tx_bytes;
    uint64_t wifi_rx_bytes;
    uint64_t wifi_tx_bytes;
} rg_device_health_t;

int rg_hal_health_get(rg_device_health_t *health);

#ifdef __cplusplus
}
#endif

#endif /* RG_HAL_H */
