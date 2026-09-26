/**
 * MTS-RG-500 Residential Gateway — WiFi HAL
 * Hardware Abstraction Layer for WiFi 6 (MediaTek MT76) monitoring
 * 
 * Responsibilities:
 * - Monitor 2 BSS (2.4GHz + 5GHz) status and statistics
 * - Read WiFi client connections from hostapd/sysfs
 * - Track WiFi chip temperature from thermal zones
 * - Manage BSS configuration (SSID, channel, security)
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>
#include <cstdint>
#include <cstring>

namespace mts::rg500::hal {

struct WifiBssInfo {
    std::string bss_id;
    std::string ssid;
    std::string band;     // "2.4ghz", "5ghz"
    uint32_t channel;
    uint32_t bandwidth;   // 20, 40, 80 MHz
    std::string security; // "none", "wep", "wpa", "wpa2", "wpa3"
    std::string mode;     // "ap", "sta", "monitor"
    std::string status;   // "up", "down", "error"
    uint32_t num_clients;
    double rx_bytes;
    double tx_bytes;
    double temperature;
};

struct WifiClientInfo {
    std::string client_id;
    std::string mac;
    std::string ssid;
    std::string band;
    uint32_t channel;
    int32_t signal;       // dBm
    uint32_t rx_rate;     // Mbps
    uint32_t tx_rate;     // Mbps
    double rx_bytes;
    double tx_bytes;
    bool connected;
    int64_t last_seen;
    int64_t connected_at;
};

struct WifiStatus {
    std::string device_id;
    std::string status;
    uint32_t total_bss;
    uint32_t active_bss;
    uint32_t total_clients;
    uint64_t total_rx_bytes;
    uint64_t total_tx_bytes;
    double temperature;
};

class IWifiHal {
public:
    virtual ~IWifiHal() = default;
    virtual std::vector<WifiBssInfo> getBssInfo() = 0;
    virtual std::vector<WifiClientInfo> getClientInfo() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class WifiHal : public IWifiHal {
public:
    WifiHal();
    ~WifiHal() override = default;

    std::vector<WifiBssInfo> getBssInfo() override;
    std::vector<WifiClientInfo> getClientInfo() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool updateBssConfig(const std::string& bss_id, const std::string& config);

    void setMockMode(bool enabled);

private:
    bool readBssFromSysfs();
    bool readWirelessStats();
    bool readTemperature();
    std::vector<WifiClientInfo> readClientListFromHostapd();
    std::vector<std::string> getBssListFromSysfs();
    std::vector<WifiClientInfo> getClientListFromSysfs();
    std::string findTemperatureSource();
    std::vector<WifiBssInfo> getBssInfoInternal();
    std::vector<WifiBssInfo> applyMockBssInfo();
    std::vector<WifiClientInfo> applyMockClients();

    WifiStatus wifi_status_;
    std::map<std::string, WifiBssInfo> bss_map_;
    std::vector<std::string> bss_list_;
    std::vector<WifiClientInfo> client_list_;
    std::string temperature_source_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::rg500::hal
