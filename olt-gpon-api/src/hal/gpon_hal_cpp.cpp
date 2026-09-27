/**
 * MTS-OLT-2000 GPON HAL — C++ Wrapper Implementation
 */

#include "hal/gpon_hal_cpp.h"
#include <cstring>
#include <iostream>
#include <algorithm>

namespace mts::olt2000::hal {

// Mock data
static bool g_initialized = false;
static mts_olt2000_olt_status_t g_mock_olt = {};
static mts_olt2000_pon_port_t g_mock_ports[MTS_OLT2000_MAX_PON_PORTS] = {};
static mts_olt2000_onu_t g_mock_onus[MTS_OLT2000_MAX_ONUS] = {};
static uint32_t g_mock_onu_count = 0;

static void initMockData() {
    if (g_initialized) return;
    
    memset(&g_mock_olt, 0, sizeof(g_mock_olt));
    memset(g_mock_ports, 0, sizeof(g_mock_ports));
    memset(g_mock_onus, 0, sizeof(g_mock_onus));
    
    strlcpy(g_mock_olt.olt_id, "MTS-OLT-2000-001", sizeof(g_mock_olt.olt_id));
    g_mock_olt.status = MTS_OLT2000_PON_PORT_UP;
    strlcpy(g_mock_olt.firmware_version, "1.0.0", sizeof(g_mock_olt.firmware_version));
    g_mock_olt.pon_ports = MTS_OLT2000_MAX_PON_PORTS;
    g_mock_olt.total_onu = 0;
    g_mock_olt.online_onu = 0;
    
    for (uint32_t i = 0; i < MTS_OLT2000_MAX_PON_PORTS; i++) {
        g_mock_ports[i].port_id = i;
        g_mock_ports[i].status = MTS_OLT2000_PON_PORT_UP;
        g_mock_ports[i].rx_power_dbm = -15.0;
        g_mock_ports[i].tx_power_dbm = 18.0;
        g_mock_ports[i].temperature_c = 45.0;
        g_mock_ports[i].active_onu = (i < 2) ? 5 : 0;
        g_mock_ports[i].max_onu = MTS_OLT2000_MAX_ONU_PER_PORT;
    }
    
    // Mock ONU data
    const char* mock_serials[] = {"SN001", "SN002", "SN003", "SN004", "SN005"};
    for (uint32_t i = 0; i < 5; i++) {
        strlcpy(g_mock_onus[i].onu_id, "ONU-0001", sizeof(g_mock_onus[i].onu_id));
        strlcpy(g_mock_onus[i].serial_number, mock_serials[i], sizeof(g_mock_onus[i].serial_number));
        g_mock_onus[i].pon_port = 0;
        g_mock_onus[i].status = MTS_OLT2000_ONU_ONLINE;
        g_mock_onus[i].power_level_dbm = -20;
        g_mock_onus[i].distance_m = 2000;
        g_mock_onus[i].vlan = 100 + i;
        g_mock_onus[i].rx_bytes = 1000000;
        g_mock_onus[i].tx_bytes = 500000;
    }
    g_mock_onu_count = 5;
    g_mock_olt.online_onu = 5;
    
    g_initialized = true;
}

GponHal::GponHal() : available_(true), mock_mode_(false) {
    std::lock_guard<std::mutex> lock(mutex_);
    initMockData();
    std::cout << "[GponHal] Constructed" << std::endl;
}

GponHal::~GponHal() {
    std::cout << "[GponHal] Destructed" << std::endl;
}

OltStatus GponHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (mock_mode_.load()) {
        return applyMockStatus();
    }
    
    mts_olt2000_olt_status_t c_status;
    if (mts_olt2000_gpon_get_olt_status(&c_status) != 0) {
        return applyMockStatus();
    }
    
    OltStatus status;
    status.olt_id = c_status.olt_id;
    status.status = (c_status.status == MTS_OLT2000_PON_PORT_UP) ? "up" : "down";
    status.firmware_version = c_status.firmware_version;
    status.total_onu = c_status.total_onu;
    status.online_onu = c_status.online_onu;
    status.pon_ports = c_status.pon_ports;
    status.rx_bytes = c_status.rx_bytes;
    status.tx_bytes = c_status.tx_bytes;
    return status;
}

std::vector<PonPortInfo> GponHal::getPonPorts() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (mock_mode_.load()) {
        return applyMockPonPorts();
    }
    
    std::vector<PonPortInfo> result;
    mts_olt2000_pon_port_t port;
    
    for (uint32_t i = 0; i < MTS_OLT2000_MAX_PON_PORTS; i++) {
        if (mts_olt2000_gpon_get_pon_port(i, &port) == 0) {
            PonPortInfo info;
            info.port_id = port.port_id;
            info.status = (port.status == MTS_OLT2000_PON_PORT_UP) ? "up" : "down";
            info.rx_power_dbm = port.rx_power_dbm;
            info.tx_power_dbm = port.tx_power_dbm;
            info.temperature_c = port.temperature_c;
            info.active_onu = port.active_onu;
            info.max_onu = port.max_onu;
            result.push_back(info);
        }
    }
    return result;
}

std::vector<OnuInfo> GponHal::getOnuList() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (mock_mode_.load()) {
        return applyMockOnuList();
    }
    
    std::vector<OnuInfo> result;
    mts_olt2000_onu_t onu;
    
    for (uint32_t i = 0; i < g_mock_onu_count; i++) {
        if (mts_olt2000_gpon_get_onu(g_mock_onus[i].onu_id, &onu) == 0) {
            OnuInfo info;
            info.onu_id = onu.onu_id;
            info.pon_port = onu.pon_port;
            info.status = (onu.status == MTS_OLT2000_ONU_ONLINE) ? "online" : "offline";
            info.serial_number = onu.serial_number;
            info.mac_address = onu.mac_address;
            info.firmware_version = onu.firmware_version;
            info.power_level_dbm = onu.power_level_dbm;
            info.distance_m = onu.distance_m;
            info.vlan = onu.vlan;
            info.qos_profile = onu.qos_profile;
            info.bandwidth_up_mbps = onu.bandwidth_up_mbps;
            info.bandwidth_down_mbps = onu.bandwidth_down_mbps;
            info.rx_bytes = onu.rx_bytes;
            info.tx_bytes = onu.tx_bytes;
            result.push_back(info);
        }
    }
    return result;
}

bool GponHal::configureOnu(const std::string& onu_id, uint32_t pon_port, uint32_t vlan,
                           const std::string& qos_profile, uint32_t bw_up, uint32_t bw_down) {
    std::lock_guard<std::mutex> lock(mutex_);
    return mts_olt2000_gpon_set_onu(onu_id.c_str(), pon_port, vlan,
                                     qos_profile.c_str(), bw_up, bw_down) == 0;
}

bool GponHal::resetOnu(const std::string& onu_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return mts_olt2000_gpon_reset_onu(onu_id.c_str()) == 0;
}

bool GponHal::isAvailable() {
    return available_;
}

void GponHal::setMockMode(bool enabled) {
    mock_mode_.store(enabled);
}

bool GponHal::checkInitialized() {
    return g_initialized;
}

OltStatus GponHal::applyMockStatus() {
    OltStatus status;
    status.olt_id = "MTS-OLT-2000-MOCK";
    status.status = "up";
    status.firmware_version = "1.0.0-mock";
    status.total_onu = 50;
    status.online_onu = 25;
    status.pon_ports = 16;
    status.rx_bytes = 1000000000;
    status.tx_bytes = 500000000;
    return status;
}

std::vector<PonPortInfo> GponHal::applyMockPonPorts() {
    std::vector<PonPortInfo> result;
    for (uint32_t i = 0; i < 16; i++) {
        PonPortInfo info;
        info.port_id = i;
        info.status = "up";
        info.rx_power_dbm = -15.0;
        info.tx_power_dbm = 18.0;
        info.temperature_c = 45.0;
        info.active_onu = 5;
        info.max_onu = 128;
        result.push_back(info);
    }
    return result;
}

std::vector<OnuInfo> GponHal::applyMockOnuList() {
    std::vector<OnuInfo> result;
    for (uint32_t i = 0; i < 5; i++) {
        OnuInfo info;
        info.onu_id = "ONU-MOCK-000" + std::to_string(i);
        info.pon_port = 0;
        info.status = "online";
        info.serial_number = "SN-MOCK-000" + std::to_string(i);
        info.power_level_dbm = -20;
        info.distance_m = 2000;
        info.vlan = 100 + i;
        info.rx_bytes = 1000000;
        info.tx_bytes = 500000;
        result.push_back(info);
    }
    return result;
}

} // namespace mts::olt2000::hal
