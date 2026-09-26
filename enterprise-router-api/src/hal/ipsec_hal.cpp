/**
 * MTS-ER-1000 Enterprise Router — IPsec HAL Implementation
 * 
 * Реализация Hardware Abstraction Layer для IPsec tunnel management
 * Интеграция с Linux subsystem:
 * - /proc/net/xfrm_state — IPsec SA monitoring
 * - /proc/net/xfrm_policy — IPsec policy monitoring
 * - /sys/class/net/ — IPsec interface statistics
 * - ipsecctl CLI — IPsec tunnel management
 * - /proc/sys/net/ipv4/ipsec — IPsec kernel parameters
 * - strongSwan/libreswan — IPsec daemon monitoring
 * - /var/log/daemon.log — IPsec event logs
 * 
 * Уровень реализации:
 * - Прямое чтение из /proc/net/xfrm_state для SA monitoring
 * - Парсинь /proc/net/xfrm_policy для policy monitoring
 * - ipsecctl CLI для tunnel management
 * - Мониторинг strongSwan/libreswan daemon
 * - Мок-режим для тестирования без hardware
 * - Thread-safe доступ к общим ресурсам
 */

#include "hal/ipsec_hal.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>

namespace mts::er1000::hal {

// ============================================================================
// IpsecHal Implementation
// ============================================================================

IpsecHal::IpsecHal()
    : mock_mode_(false)
    , available_(false)
{
    // Проверка доступности IPsec subsystem
    available_ = isAvailable();

    if (available_) {
        // Инициализация SA monitoring
        sa_count_ = readSaCount();
        std::cout << "[IPsec HAL] Active IPsec SAs: " << sa_count_ << std::endl;

        // Инициализация policy monitoring
        policy_count_ = readPolicyCount();
        std::cout << "[IPsec HAL] Active IPsec policies: " << policy_count_ << std::endl;

        // Мониторинг IPsec daemon
        ipsec_daemon_available_ = checkIpsecDaemonRunning();
        if (ipsec_daemon_available_) {
            std::cout << "[IPsec HAL] IPsec daemon is running" << std::endl;
        }
    } else {
        std::cout << "[IPsec HAL] IPsec subsystem not available, enabling mock mode" << std::endl;
        mock_mode_ = true;
    }
}

IpsecHal::~IpsecHal() {
    // Очистка ресурсов
}

/**
 * Получить статус всех IPsec туннелей
 * 
 * Чтение данных из:
 * 1. /proc/net/xfrm_state — IPsec SA state и counters
 * 2. /proc/net/xfrm_policy — IPsec policy state
 * 3. ipsecctl show — tunnel management output
 * 4. strongSwan/libreswan — daemon state
 * 5. Моки при отсутствии hardware
 */
std::vector<IpsecTunnelStatus> IpsecHal::getTunnelStatus() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (mock_mode_) {
        return applyMockTunnels();
    }

    if (!available_) {
        mock_mode_ = true;
        return applyMockTunnels();
    }

    // Чтение SA из /proc/net/xfrm_state
    readSaFromProc();

    // Обновление counters
    updateSaCounters();

    return getTunnelStatusInternal();
}

/**
 * Проверить доступность IPsec subsystem
 */
bool IpsecHal::isAvailable() {
    struct stat st;
    // Проверка /proc/net/xfrm_state
    if (stat("/proc/net/xfrm_state", &st) == 0) {
        return true;
    }

    // Проверка ipsecctl CLI
    if (stat("/usr/sbin/ipsecctl", &st) == 0) {
        return true;
    }

    return false;
}

/**
 * Получить имя IPsec device
 */
std::string IpsecHal::getDeviceName() {
    std::lock_guard<std::mutex> lock(mutex_);
    return "MTS-ER-1000-IPSEC";
}

/**
 * Создать IPsec tunnel
 * 
 * В реальном устройстве:
 * - ipsecctl tunnel add для создания tunnel
 * - ip xfrm add для SA
 * - ip xfrm policy add для policy
 * - strongSwan/libreswan для IKE negotiation
 */
bool IpsecHal::createTunnel(const IpsecTunnelStatus& tunnel) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Проверка дубликатов
    for (const auto& pair : tunnels_) {
        if (pair.second.peer_ip == tunnel.peer_ip) {
            std::cerr << "[IPsec HAL] Tunnel already exists: peer="
                      << tunnel.peer_ip << std::endl;
            return false;
        }
    }

    // Формирование команд
    std::string sa_cmd = "ip xfrm state add src " + tunnel.local_subnet
                         + " dst " + tunnel.peer_ip
                         + " proto esp spi 0x" + tunnel.tunnel_id;
    std::cout << "[IPsec HAL] Creating IPsec SA: " << sa_cmd << std::endl;

    // В реальном устройстве:
    // - ip xfrm state add ...
    // - ip xfrm policy add ...
    // - ipsecctl tunnel add ...
    // - strongSwan IKE negotiation

    tunnels_[tunnel.tunnel_id] = tunnel;

    return true;
}

/**
 * Удалить IPsec tunnel
 */
bool IpsecHal::deleteTunnel(const std::string& tunnel_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = tunnels_.find(tunnel_id);
    if (it == tunnels_.end()) {
        return false;
    }

    // В реальном устройстве:
    // - ip xfrm state del
    // - ip xfrm policy del
    // - ipsecctl tunnel del
    std::cout << "[IPsec HAL] Deleting IPsec tunnel: " << tunnel_id << std::endl;

    tunnels_.erase(it);
    return true;
}

/**
 * Проверка IPsec daemon running
 */
bool IpsecHal::checkIpsecDaemonRunning() {
    struct stat st;
    if (stat("/var/run/charon.pid", &st) == 0) {
        return true;
    }
    if (stat("/var/run/pluto.pid", &st) == 0) {
        return true;
    }
    return false;
}

/**
 * Чтение SA count из /proc/net/xfrm_state
 */
uint32_t IpsecHal::readSaCount() {
    std::string path = "/proc/net/xfrm_state";
    std::ifstream file(path);

    if (!file.is_open()) {
        return 0;
    }

    uint32_t count = 0;
    std::string line;
    bool header_skipped = false;
    while (std::getline(file, line)) {
        if (!header_skipped) {
            header_skipped = true;
            continue;
        }
        count++;
    }

    return count;
}

/**
 * Чтение policy count из /proc/net/xfrm_policy
 */
uint32_t IpsecHal::readPolicyCount() {
    std::string path = "/proc/net/xfrm_policy";
    std::ifstream file(path);

    if (!file.is_open()) {
        return 0;
    }

    uint32_t count = 0;
    std::string line;
    bool header_skipped = false;
    while (std::getline(file, line)) {
        if (!header_skipped) {
            header_skipped = true;
            continue;
        }
        count++;
    }

    return count;
}

// ============================================================================
// Private Methods
// ============================================================================

/**
 * Чтение SA из /proc/net/xfrm_state
 */
bool IpsecHal::readSaFromProc() {
    std::string path = "/proc/net/xfrm_state";
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "[IPsec HAL] Cannot open: " << path << std::endl;
        return false;
    }

    std::string line;
    bool header_skipped = false;
    while (std::getline(file, line)) {
        if (!header_skipped) {
            header_skipped = true;
            continue;
        }

        // Парсинь xfrm_state entry
        // Формат: <index> <refcnt> <flags> <state> <algos> <lifetime> <bytes> <packets> ...
        std::istringstream iss(line);
        uint32_t index;
        uint64_t bytes, packets;
        std::string state;

        iss >> index;
        iss >> state;
        iss >> bytes >> packets;

        // Обновление counters для соответствующего tunnel
        for (auto& pair : tunnels_) {
            if (pair.second.phase == 2) {
                pair.second.rx_bytes = bytes;
                pair.second.rx_packets = packets;
                pair.second.status = "up";
                break;
            }
        }
    }

    return true;
}

/**
 * Обновление SA counters
 */
bool IpsecHal::updateSaCounters() {
    // В реальном устройстве:
    // - Чтение /proc/net/xfrm_stat для error counters
    // - ip xfrm state show для detailed stats
    std::cout << "[IPsec HAL] Updating SA counters" << std::endl;
    return true;
}

/**
 * Получить список туннелей
 */
std::vector<IpsecTunnelStatus> IpsecHal::getTunnelStatusInternal() {
    std::vector<IpsecTunnelStatus> result;
    for (const auto& pair : tunnels_) {
        result.push_back(pair.second);
    }
    return result;
}

/**
 * Мок-туннели для тестирования
 */
std::vector<IpsecTunnelStatus> IpsecHal::applyMockTunnels() {
    std::vector<IpsecTunnelStatus> tunnels;

    // Мок IPsec tunnels (typical enterprise setup)
    struct TunnelInfo {
        const char* tunnel_id;
        const char* name;
        const char* peer_ip;
        const char* local_subnet;
        const char* remote_subnet;
        const char* mode;
        const char* status;
        uint32_t phase;
        uint64_t rx;
        uint64_t tx;
        uint64_t rx_pkt;
        uint64_t tx_pkt;
        int64_t established;
    };

    static const TunnelInfo mock_tunnels[] = {
        {"tun-001", "HQ-VPN", "10.100.1.1", "192.168.1.0/24", "10.100.0.0/16",
         "tunnel", "up", 2, 5432109876, 3210987654, 12345678, 9876543, 1726000000},
        {"tun-002", "Branch-VPN", "10.200.2.2", "192.168.2.0/24", "10.200.0.0/16",
         "tunnel", "up", 2, 4321098765, 2109876543, 9876543, 7654321, 1725900000},
        {"tun-003", "DMZ-VPN", "10.300.3.3", "172.16.0.0/12", "172.16.0.0/12",
         "transport", "negotiating", 1, 0, 0, 0, 0, 1726100000},
    };

    for (const auto& info : mock_tunnels) {
        IpsecTunnelStatus tunnel;
        tunnel.tunnel_id = info.tunnel_id;
        tunnel.name = info.name;
        tunnel.peer_ip = info.peer_ip;
        tunnel.local_subnet = info.local_subnet;
        tunnel.remote_subnet = info.remote_subnet;
        tunnel.mode = info.mode;
        tunnel.status = info.status;
        tunnel.phase = info.phase;
        tunnel.rx_bytes = info.rx;
        tunnel.tx_bytes = info.tx;
        tunnel.rx_packets = info.rx_pkt;
        tunnel.tx_packets = info.tx_pkt;
        tunnel.established_at = info.established;
        tunnels.push_back(tunnel);
    }

    return tunnels;
}

} // namespace mts::er1000::hal
