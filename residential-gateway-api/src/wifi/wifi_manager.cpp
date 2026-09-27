/**
 * MTS-RG-500 WiFi Manager — WiFi 6 (MT76) monitoring
 * Provides mock implementation for WiFi BSS and client tracking
 */

#include "hal/wifi_hal.h"
#include <cstdio>
#include <cstring>
#include <chrono>

namespace mts::rg500::hal {

WifiHal::WifiHal() : mock_mode_(false), available_(true) {
    memset(&wifi_status_, 0, sizeof(wifi_status_));
    wifi_status_.device_id = "MTS-RG-500-WIFI";
    wifi_status_.status = "active";
    wifi_status_.total_bss = 2;
    wifi_status_.active_bss = 2;
    wifi_status_.total_clients = 0;
    wifi_status_.total_rx_bytes = 0;
    wifi_status_.total_tx_bytes = 0;
    wifi_status_.temperature = 45.0;
}

std::vector<WifiBssInfo> WifiHal::getBssInfo() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockBssInfo();
    }
    return getBssInfoInternal();
}

std::vector<WifiClientInfo> WifiHal::getClientInfo() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockClients();
    }
    return getClientListFromSysfs();
}

bool WifiHal::isAvailable() {
    return available_;
}

std::string WifiHal::getDeviceName() {
    return "MTS-RG-500-WIFI-MT76";
}

bool WifiHal::updateBssConfig(const std::string& bss_id, const std::string& config) {
    (void)bss_id;
    (void)config;
    return true;
}

void WifiHal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

std::vector<WifiBssInfo> WifiHal::getBssInfoInternal() {
    std::vector<WifiBssInfo> bss_list;
    WifiBssInfo bss2g, bss5g;

    bss2g.bss_id = "wlan0";
    bss2g.ssid = "MTS-RG-500-2.4";
    bss2g.band = "2.4ghz";
    bss2g.channel = 6;
    bss2g.bandwidth = 40;
    bss2g.security = "wpa2";
    bss2g.mode = "ap";
    bss2g.status = "up";
    bss2g.num_clients = 5;
    bss2g.rx_bytes = 524288;
    bss2g.tx_bytes = 262144;
    bss2g.temperature = 42.0;

    bss5g.bss_id = "wlan1";
    bss5g.ssid = "MTS-RG-500-5";
    bss5g.band = "5ghz";
    bss5g.channel = 36;
    bss5g.bandwidth = 80;
    bss5g.security = "wpa2";
    bss5g.mode = "ap";
    bss5g.status = "up";
    bss5g.num_clients = 3;
    bss5g.rx_bytes = 1048576;
    bss5g.tx_bytes = 524288;
    bss5g.temperature = 48.0;

    bss_list.push_back(bss2g);
    bss_list.push_back(bss5g);
    return bss_list;
}

std::vector<WifiClientInfo> WifiHal::getClientListFromSysfs() {
    std::vector<WifiClientInfo> clients;
    return clients;
}

std::vector<WifiClientInfo> WifiHal::applyMockClients() {
    std::vector<WifiClientInfo> clients;
    WifiClientInfo client;
    client.client_id = "client-001";
    client.mac = "AA:BB:CC:DD:EE:FF";
    client.ssid = "MTS-RG-500-2.4";
    client.band = "2.4ghz";
    client.channel = 6;
    client.signal = -55;
    client.rx_rate = 144;
    client.tx_rate = 72;
    client.rx_bytes = 102400;
    client.tx_bytes = 51200;
    client.connected = true;
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    client.last_seen = now;
    client.connected_at = now - 3600;
    clients.push_back(client);
    return clients;
}

std::vector<WifiBssInfo> WifiHal::applyMockBssInfo() {
    std::vector<WifiBssInfo> bss_list;
    WifiBssInfo bss;
    bss.bss_id = "MOCK-wlan0";
    bss.ssid = "MOCK-SSID";
    bss.band = "2.4ghz";
    bss.channel = 1;
    bss.bandwidth = 20;
    bss.security = "wpa3";
    bss.mode = "ap";
    bss.status = "up";
    bss.num_clients = 1;
    bss.rx_bytes = 0;
    bss.tx_bytes = 0;
    bss.temperature = 40.0;
    bss_list.push_back(bss);
    return bss_list;
}

bool WifiHal::readBssFromSysfs() { return true; }
bool WifiHal::readWirelessStats() { return true; }
bool WifiHal::readTemperature() { return true; }
std::vector<WifiClientInfo> WifiHal::readClientListFromHostapd() { return {}; }
std::vector<std::string> WifiHal::getBssListFromSysfs() { return {"wlan0", "wlan1"}; }
std::string WifiHal::findTemperatureSource() { return "/sys/class/thermal/thermal_zone0"; }

} // namespace mts::rg500::hal
