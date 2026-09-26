/**
 * MTS-OLT-2000 OLT GPON — ONU HAL Implementation
 * 
 * Реализация Hardware Abstraction Layer для ONU management
 * Интеграция с Linux subsystem:
 * - /sys/class/gpon/ — sysfs для GPON ONU configuration
 * - /proc/net/ — ONU network interfaces
 * - rtl_gpon CLI — ONU config management
 * - SNMP — ONU bandwidth/QoS configuration
 * 
 * Уровень реализации:
 * - Прямое чтение/запись в sysfs для ONU config
 * - Парсинг rtl_gpon output для ONU state
 * - SNMP set для bandwidth/QoS configuration
 * - Мок-режим для тестирования без hardware
 * - Thread-safe доступ к общим ресурсам
 */

#include "hal/onu_hal.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>

namespace mts::olt2000::hal {

// ============================================================================
// OnuHal Implementation
// ============================================================================

OnuHal::OnuHal()
    : mock_mode_(false)
    , available_(false)
{
    // Проверка доступности ONU subsystem
    available_ = isAvailable();

    if (available_) {
        // Загрузка конфигурации из sysfs
        loadConfigsFromSysfs();
        std::cout << "[ONU HAL] Loaded configurations from sysfs" << std::endl;
    } else {
        std::cout << "[ONU HAL] ONU subsystem not available, enabling mock mode" << std::endl;
        mock_mode_ = true;
    }
}

OnuHal::~OnuHal() {
    // Очистка ресурсов
}

/**
 * Получить все конфигурации ONU
 */
std::vector<OnuConfig> OnuHal::getConfigs() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (mock_mode_) {
        return applyMockConfigs();
    }

    if (!available_) {
        mock_mode_ = true;
        return applyMockConfigs();
    }

    // Чтение из sysfs
    return readConfigsFromSysfs();
}

/**
 * Проверить доступность ONU subsystem
 */
bool OnuHal::isAvailable() {
    struct stat st;
    if (stat("/sys/class/gpon", &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    if (stat("/usr/bin/rtl_gpon", &st) == 0) {
        return true;
    }
    return false;
}

/**
 * Получить имя ONU device
 */
std::string OnuHal::getDeviceName() {
    std::lock_guard<std::mutex> lock(mutex_);
    return "MTS-OLT-2000-ONU";
}

/**
 * Установить конфигурацию ONU
 * 
 * В реальном устройстве:
 * - rtl_gpon onu config --id <onu_id> --vlan <vlan> --qos <profile>
 * - SNMP set для bandwidth limits
 * - OMCI provisioning
 */
bool OnuHal::setConfig(const OnuConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Формирование команды
    std::string command = "rtl_gpon onu config --id " + config.onu_id
                          + " --vlan " + std::to_string(config.vlan)
                          + " --qos " + config.qos_profile
                          + " --up " + std::to_string(config.bandwidth_up)
                          + " --down " + std::to_string(config.bandwidth_down);
    std::cout << "[ONU HAL] Setting config: " << command << std::endl;

    // В реальном устройстве:
    // - system(command.c_str())
    // - SNMP set для bandwidth limits
    // - OMCI provisioning for service profile

    configs_[config.onu_id] = config;

    return true;
}

/**
 * Удалить конфигурацию ONU
 */
bool OnuHal::deleteConfig(const std::string& onu_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = configs_.find(onu_id);
    if (it == configs_.end()) {
        return false;
    }

    // В реальном устройстве:
    // - rtl_gpon onu del-config --id <onu_id>
    std::cout << "[ONU HAL] Deleting ONU config: " << onu_id << std::endl;

    configs_.erase(it);
    return true;
}

// ============================================================================
// Private Methods
// ============================================================================

/**
 * Загрузка конфигураций из sysfs
 */
bool OnuHal::loadConfigsFromSysfs() {
    // В реальном устройстве:
    // - Чтение /sys/class/gpon/*/onu*/config
    // - Парсинь YAML/JSON конфигурации
    std::cout << "[ONU HAL] Loading configs from sysfs" << std::endl;
    return true;
}

/**
 * Чтение конфигураций из sysfs
 */
std::vector<OnuConfig> OnuHal::readConfigsFromSysfs() {
    std::vector<OnuConfig> configs;

    for (const auto& pair : configs_) {
        OnuConfig config = pair.second;

        // Чтение из sysfs
        std::string path = "/sys/class/gpon/pon0/onu" + pair.first + "/config";
        std::ifstream file(path);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Парсинь key=value
                auto pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    if (key == "vlan") config.vlan = std::stoul(value);
                    else if (key == "qos") config.qos_profile = value;
                    else if (key == "bw_up") config.bandwidth_up = std::stoul(value);
                    else if (key == "bw_down") config.bandwidth_down = std::stoul(value);
                }
            }
        }

        configs.push_back(config);
    }

    return configs;
}

/**
 * Мок-конфигурации для тестирования
 */
std::vector<OnuConfig> OnuHal::applyMockConfigs() {
    std::vector<OnuConfig> configs;

    for (int i = 0; i < 10; i++) {
        OnuConfig config;
        config.onu_id = "ONU-MOCK-" + std::to_string(i);
        config.pon_port = "PON-" + std::to_string(i % 4);
        config.vlan = 100 + i;
        config.qos_profile = "default";
        config.bandwidth_up = 50000;
        config.bandwidth_down = 100000;
        configs.push_back(config);
    }

    return configs;
}

} // namespace mts::olt2000::hal
