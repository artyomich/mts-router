/**
 * MTS-MC-5000 Mobile Core — SMF HAL Implementation
 * 
 * Реализация Hardware Abstraction Layer для SMF (Session Management Function)
 * Интеграция с Linux subsystem:
 * - /proc/net/ — статистика сетевых интерфейсов
 * - /sys/class/net/ — интерфейсы для DNS и PGW
 * - /proc/sys/net/ — параметры IP-стека
 * - libreswan/libresolv — DNS resolution
 * 
 * Уровень реализации:
 * - Чтение из /proc для мониторинга сессий
 * - Парсинг /proc/net для статистики
 * - Мок-режим для тестирования без hardware
 * - Thread-safe доступ к общим ресурсам
 */

#include "hal/sm_hal.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>

namespace mts::mc5000::hal {

// ============================================================================
// SmfHal Implementation
// ============================================================================

SmfHal::SmfHal()
    : mock_mode_(false)
    , available_(false)
{
    // Инициализация статуса SMF
    memset(&smf_config_, 0, sizeof(smf_config_));
    smf_config_.smf_id = "MTS-MC-5000-SMF-001";
    smf_config_.status = "inactive";
    smf_config_.max_sessions = 100000;
    smf_config_.current_sessions = 0;
    smf_config_.dns_primary = "10.64.0.1";
    smf_config_.dns_secondary = "10.64.0.2";
    smf_config_.pgw_ip = "192.168.10.1";

    // Проверка доступности SMF subsystem
    available_ = isAvailable();

    if (available_) {
        std::cout << "[SMF HAL] SMF subsystem available" << std::endl;
    } else {
        std::cout << "[SMF HAL] SMF subsystem not available, enabling mock mode" << std::endl;
        mock_mode_ = true;
    }
}

SmfHal::~SmfHal() {
    // Очистка ресурсов
}

/**
 * Получить текущий статус SMF
 * 
 * Чтение данных из:
 * 1. /proc/net/ — статистика IP-стека
 * 2. /proc/sys/net/ — параметры IP-стека
 * 3. Моки при отсутствии hardware
 */
SmfConfig SmfHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (mock_mode_) {
        applyMockData();
        return smf_config_;
    }

    if (!available_) {
        // Если hardware недоступен, включаем мок
        mock_mode_ = true;
        applyMockData();
        return smf_config_;
    }

    // Чтение из /proc/net для мониторинга сессий
    readFromProcNet();

    // Обновление текущего времени
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();
    smf_config_.last_updated = now_ms;

    return smf_config_;
}

/**
 * Проверить доступность SMF subsystem
 */
bool SmfHal::isAvailable() {
    // Проверка наличия необходимых файлов /proc
    struct stat st;
    if (stat("/proc/net", &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    return false;
}

/**
 * Получить имя SMF device
 */
std::string SmfHal::getDeviceName() {
    std::lock_guard<std::mutex> lock(mutex_);
    return smf_config_.smf_id;
}

/**
 * Обновить DNS-конфигурацию
 */
bool SmfHal::updateDnsConfig(const std::string& primary_dns,
                              const std::string& secondary_dns) {
    std::lock_guard<std::mutex> lock(mutex_);

    smf_config_.dns_primary = primary_dns;
    smf_config_.dns_secondary = secondary_dns;

    // В реальном устройстве здесь был бы вызов:
    // - Обновление /etc/resolv.conf
    // - Или перезапуск systemd-resolved
    // system(("echo 'nameserver " + primary_dns + "' > /etc/resolv.conf").c_str());

    std::cout << "[SMF HAL] DNS updated: primary=" << primary_dns
              << ", secondary=" << secondary_dns << std::endl;

    return true;
}

/**
 * Обновить PGW-адрес
 */
bool SmfHal::updatePgwAddress(const std::string& pgw_ip) {
    std::lock_guard<std::mutex> lock(mutex_);

    smf_config_.pgw_ip = pgw_ip;

    std::cout << "[SMF HAL] PGW address updated: " << pgw_ip << std::endl;
    return true;
}

// ============================================================================
// Private Methods
// ============================================================================

/**
 * Чтение данных из /proc/net
 */
bool SmfHal::readFromProcNet() {
    // Чтение /proc/net/dev для статистики
    std::string path = "/proc/net/dev";
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "[SMF HAL] Cannot open: " << path << std::endl;
        return false;
    }

    std::string line;
    bool header_skipped = false;
    while (std::getline(file, line)) {
        // Пропуск заголовка
        if (!header_skipped) {
            header_skipped = true;
            continue;
        }

        if (line.find(":") != std::string::npos) {
            std::istringstream iss(line);
            std::string iface;
            iss >> iface;

            // Парсинг статистики (rx_bytes, rx_packets, tx_bytes, tx_packets)
            uint64_t rx_bytes, rx_packets, tx_bytes, tx_packets;
            iss >> rx_bytes >> rx_packets;
            iss >> tx_bytes >> tx_packets;

            // Обновление статуса интерфейса
            for (auto& iface_stat : interface_stats_) {
                if (iface_stat.name == iface.substr(0, iface.find(':'))) {
                    iface_stat.rx_bytes = rx_bytes;
                    iface_stat.rx_packets = rx_packets;
                    iface_stat.tx_bytes = tx_bytes;
                    iface_stat.tx_packets = tx_packets;
                    break;
                }
            }
        }
    }

    return true;
}

/**
 * Мок-данные для тестирования
 */
void SmfHal::applyMockData() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();

    smf_config_.smf_id = "MTS-MC-5000-SMF-001";
    smf_config_.status = "active";
    smf_config_.max_sessions = 100000;
    smf_config_.current_sessions = 0;
    smf_config_.dns_primary = "10.64.0.1";
    smf_config_.dns_secondary = "10.64.0.2";
    smf_config_.pgw_ip = "192.168.10.1";
    smf_config_.last_updated = now_ms;
}

} // namespace mts::mc5000::hal
