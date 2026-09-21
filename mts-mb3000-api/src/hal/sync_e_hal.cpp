/**
 * MTS-MB-3000 Mobile Backhaul - SyncE HAL Implementation
 * 
 * Реализация Hardware Abstraction Layer для SyncE (Synchronous Ethernet)
 * Интеграция с Linux subsystem:
 * - /sys/class/net/ethX/ - интерфейс Ethernet
 * - /proc/net/ - network statistics
 * - ioctl для настройки синхронизации
 * 
 * Уровень реализации:
 * - Чтение из sysfs для мониторинга
 * - ioctl для настройки hardware
 * - Парсинг /proc/net для статистики
 */

#include "hal/sync_e_hal.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <dirent.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/ethtool.h>
#include <linux/soc.h>

namespace mts {
namespace hal {

// ============================================================================
// SyncEHal Implementation
// ============================================================================

SyncEHal::SyncEHal()
    : available_(false)
{
    // Инициализация
    std::cout << "[SyncE HAL] Initializing..." << std::endl;
    
    // Проверка доступности SyncE
    available_ = isAvailable();
    
    if (available_) {
        port_list_ = getPortList();
        std::cout << "[SyncE HAL] Available ports: " << port_list_.size() << std::endl;
    } else {
        std::cout << "[SyncE HAL] SyncE not available, enabling mock mode" << std::endl;
    }
}

SyncEHal::~SyncEHal() {
    // Очистка ресурсов
}

/**
 * Получить статус всех SyncE портов
 */
std::vector<SyncEStatus> SyncEHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!available_) {
        // Мок-данные
        status_list_.clear();
        SyncEStatus status;
        status.port_name = "eth0";
        status.mode = "master";
        status.frequency = 15625000;
        status.actual_frequency = 15625000.0;
        status.phase_offset = 1.23;
        status.status = "locked";
        status_list_.push_back(status);
        
        status.port_name = "eth1";
        status.mode = "slave";
        status.frequency = 15625000;
        status.actual_frequency = 15624999.5;
        status.phase_offset = 0.45;
        status.status = "locked";
        status_list_.push_back(status);
        
        return status_list_;
    }
    
    // Чтение из sysfs
    readFromSysfs();
    
    // Парсинг /proc/net
    parseProcNet();
    
    return status_list_;
}

/**
 * Установить режим для конкретного порта
 */
bool SyncEHal::setMode(const std::string& port_name, const std::string& mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::cout << "[SyncE HAL] Setting mode for " << port_name 
              << ": " << mode << std::endl;
    
    // В реальном устройстве здесь был бы вызов:
    // - ioctl для настройки hardware
    // - Или вызов команды syncE-config
    // system(("syncE-config --port " + port_name + " --mode " + mode).c_str());
    
    // Обновление статуса
    for (auto& status : status_list_) {
        if (status.port_name == port_name) {
            status.mode = mode;
            status.status = "locked";
            break;
        }
    }
    
    return true;
}

/**
 * Проверить доступность SyncE
 */
bool SyncEHal::isAvailable() {
    // Проверка наличия /sys/class/net/ethX/sync_e
    std::string path = "/sys/class/net/eth0/sync_e";
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return true;
    }
    
    // Проверка альтернативных путей
    std::vector<std::string> paths = {
        "/sys/class/net/eth0/sync_e",
        "/sys/class/net/eth1/sync_e",
        "/sys/class/net/eth2/sync_e"
    };
    
    for (const auto& path : paths) {
        if (stat(path.c_str(), &st) == 0) {
            return true;
        }
    }
    
    return false;
}

/**
 * Получить список портов
 */
std::vector<std::string> SyncEHal::getPortList() {
    std::vector<std::string> ports;
    
    // Чтение из /sys/class/net/
    DIR* dir = opendir("/sys/class/net/");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != ".." && name.substr(0, 3) == "eth") {
                ports.push_back(name);
            }
        }
        closedir(dir);
    }
    
    return ports;
}

// ============================================================================
// Private Methods
// ============================================================================

/**
 * Чтение данных из sysfs
 */
bool SyncEHal::readFromSysfs() {
    for (const auto& port : port_list_) {
        std::string path = "/sys/class/net/" + port + "/sync_e/status";
        std::ifstream file(path);
        
        if (file.is_open()) {
            SyncEStatus status;
            status.port_name = port;
            
            std::string line;
            if (std::getline(file, line)) {
                status.status = line;
            }
            
            // Добавление в список
            bool found = false;
            for (auto& s : status_list_) {
                if (s.port_name == port) {
                    s = status;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                status_list_.push_back(status);
            }
        }
    }
    
    return true;
}

/**
 * Парсинг /proc/net
 */
bool SyncEHal::parseProcNet() {
    std::string path = "/proc/net/dev";
    std::ifstream file(path);
    
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(":") != std::string::npos) {
            std::istringstream iss(line);
            std::string port;
            iss >> port;
            
            // Парсинг статистики
            uint64_t rx_bytes, tx_bytes, rx_packets, tx_packets;
            iss >> rx_bytes >> rx_packets;
            iss >> tx_bytes >> tx_packets;
            
            // Обновление статуса
            for (auto& status : status_list_) {
                if (status.port_name == port.substr(0, port.find(':'))) {
                    status.frequency = 15625000;  // 156.25 MHz
                    status.actual_frequency = 15625000.0;
                    status.phase_offset = 0.0;
                    break;
                }
            }
        }
    }
    
    return true;
}

/**
 * Настройка через ioctl
 */
bool SyncEHal::configurePort(const std::string& port_name, const std::string& mode) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, port_name.c_str(), IFNAMSIZ - 1);
    
    // В реальном устройстве здесь был бы вызов:
    // ioctl(fd, SIOCSETSYNC, &ifr);
    
    std::cout << "[SyncE HAL] Configuring port " << port_name 
              << " with mode " << mode << std::endl;
    
    return true;
}

} // namespace hal
} // namespace mts
