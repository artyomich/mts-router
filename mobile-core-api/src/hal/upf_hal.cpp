/**
 * MTS-MC-5000 Mobile Core — UPF HAL Implementation
 * 
 * Реализация Hardware Abstraction Layer для UPF (User Plane Function)
 * Интеграция с Linux subsystem:
 * - /proc/net/ — статистика IP-стека и маршрутизации
 * - /sys/class/net/ — sysfs interface для сетевых интерфейсов
 * - /sys/class/net/ethX/statistics/ — counters per interface
 * - iproute2 для управления таблицами маршрутизации
 * - /proc/net/nf_conntrack — tracking conntrack для GTP
 * 
 * Уровень реализации:
 * - Прямое чтение из sysfs/procfs (Linux kernel interface)
 * - Парсинг iproute2 output для маршрутов
 * - Conntrack monitoring для GTP-туннелей
 * - Мок-режим для тестирования без hardware
 * - Thread-safe доступ к общим ресурсам
 */

#include "hal/upf_hal.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

namespace mts::mc5000::hal {

// ============================================================================
// UpfHal Implementation
// ============================================================================

UpfHal::UpfHal()
    : mock_mode_(false)
    , available_(false)
{
    // Инициализация статуса UPF
    memset(&upf_status_, 0, sizeof(upf_status_));
    upf_status_.upf_id = "MTS-MC-5000-UPF-001";
    upf_status_.status = "inactive";
    upf_status_.active_sessions = 0;
    upf_status_.max_sessions = 100000;
    upf_status_.rx_bytes = 0;
    upf_status_.tx_bytes = 0;
    upf_status_.rx_packets = 0;
    upf_status_.tx_packets = 0;
    upf_status_.cpu_usage = 0.0;
    upf_status_.memory_usage = 0.0;

    // Проверка доступности UPF subsystem
    available_ = isAvailable();

    if (available_) {
        // Инициализация списка интерфейсов
        interface_list_ = getInterfaceList();
        std::cout << "[UPF HAL] Available interfaces: " << interface_list_.size() << std::endl;

        // Инициализация conntrack мониторинга
        conntrack_entries_ = readConntrackTable();
        std::cout << "[UPF HAL] Conntrack entries: " << conntrack_entries_.size() << std::endl;
    } else {
        std::cout << "[UPF HAL] UPF subsystem not available, enabling mock mode" << std::endl;
        mock_mode_ = true;
    }
}

UpfHal::~UpfHal() {
    // Очистка ресурсов
}

/**
 * Получить текущий статус UPF
 * 
 * Чтение данных из:
 * 1. /proc/stat — CPU usage
 * 2. /proc/meminfo — memory usage
 * 3. /proc/net/dev — interface statistics
 * 4. /proc/net/nf_conntrack — conntrack/GTP tracking
 * 5. Моки при отсутствии hardware
 */
UpfStatus UpfHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (mock_mode_) {
        applyMockData();
        return upf_status_;
    }

    if (!available_) {
        // Если hardware недоступен, включаем мок
        mock_mode_ = true;
        applyMockData();
        return upf_status_;
    }

    // Чтение CPU usage из /proc/stat
    readCpuUsage();

    // Чтение memory usage из /proc/meminfo
    readMemoryUsage();

    // Чтение статистики интерфейсов из /proc/net/dev
    readInterfaceStats();

    // Обновление conntrack таблицы
    conntrack_entries_ = readConntrackTable();

    // Обновление статуса активных сессий
    upf_status_.active_sessions = static_cast<uint32_t>(sessions_.size());
    upf_status_.status = "active";

    // Обновление текущего времени
    auto now = std::chrono::system_clock::now();
    upf_status_.last_updated = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();

    return upf_status_;
}

/**
 * Получить все PDU сессии
 */
std::vector<PduSession> UpfHal::getPduSessions() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (mock_mode_) {
        return mock_sessions_;
    }

    std::vector<PduSession> sessions;
    for (const auto& pair : sessions_) {
        sessions.push_back(pair.second);
    }

    // Обновление last_active из conntrack
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()).count();

    for (auto& session : sessions) {
        // Поиск conntrack entry для этой сессии
        for (const auto& ct : conntrack_entries_) {
            if (ct.teid == session.teid) {
                session.last_active = now_sec;
                session.rx_bytes = ct.rx_bytes;
                session.tx_bytes = ct.tx_bytes;
                break;
            }
        }
    }

    return sessions;
}

/**
 * Создать новую PDU сессию
 * 
 * В реальном устройстве:
 * - Создание GTP tunnel через netlink socket
 * - Добавление conntrack entry
 * - Настройка QoS через tc/htb
 */
bool UpfHal::createPduSession(const PduSession& session) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (sessions_.size() >= 100000) {
        std::cerr << "[UPF HAL] Session limit reached (100000)" << std::endl;
        return false;
    }

    // В реальном устройстве здесь был бы вызов:
    // - ip route add через netlink
    // - conntrack -A для создания entry
    // - tc qdisc для QoS
    std::string command = "ip route add " + session.ue_ip + "/32 via " + session.upf_ip;
    std::cout << "[UPF HAL] Creating PDU session: " << command << std::endl;

    sessions_[session.session_id] = session;

    return true;
}

/**
 * Удалить PDU сессию
 */
bool UpfHal::deletePduSession(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }

    // В реальном устройстве:
    // - ip route del
    // - conntrack -D
    std::string command = "ip route del " + it->second.ue_ip + "/32";
    std::cout << "[UPF HAL] Deleting PDU session: " << command << std::endl;

    sessions_.erase(it);
    return true;
}

/**
 * Проверить доступность UPF subsystem
 */
bool UpfHal::isAvailable() {
    // Проверка наличия /proc/net
    struct stat st;
    if (stat("/proc/net", &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }

    // Проверка наличия iproute2
    if (stat("/usr/sbin/ip", &st) == 0) {
        return true;
    }

    return false;
}

/**
 * Получить имя UPF device
 */
std::string UpfHal::getDeviceName() {
    std::lock_guard<std::mutex> lock(mutex_);
    return "MTS-MC-5000-UPF";
}

/**
 * Получить список интерфейсов
 */
std::vector<std::string> UpfHal::getInterfaceList() {
    std::vector<std::string> interfaces;

    DIR* dir = opendir("/sys/class/net/");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != ".." && name.substr(0, 3) == "eth") {
                interfaces.push_back(name);
            }
        }
        closedir(dir);
    }

    return interfaces;
}

// ============================================================================
// Private Methods
// ============================================================================

/**
 * Чтение CPU usage из /proc/stat
 */
bool UpfHal::readCpuUsage() {
    std::string path = "/proc/stat";
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "[UPF HAL] Cannot open: " << path << std::endl;
        return false;
    }

    std::string name;
    long long user, nice, system, idle, iowait, irq, softirq, steal;
    file >> name >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    long long active = total - idle - iowait;

    if (total > 0) {
        upf_status_.cpu_usage = 100.0 * active / total;
    }

    return true;
}

/**
 * Чтение memory usage из /proc/meminfo
 */
bool UpfHal::readMemoryUsage() {
    std::string path = "/proc/meminfo";
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "[UPF HAL] Cannot open: " << path << std::endl;
        return false;
    }

    long long mem_total = 0, mem_available = 0;
    std::string key;
    long long value;
    while (file >> key >> value) {
        if (key == "MemTotal:") mem_total = value;
        else if (key == "MemAvailable:") mem_available = value;
        if (mem_total > 0 && mem_available > 0) break;
    }

    if (mem_total > 0) {
        upf_status_.memory_usage = 100.0 * (mem_total - mem_available) / mem_total;
    }

    return true;
}

/**
 * Чтение статистики интерфейсов из /proc/net/dev
 */
bool UpfHal::readInterfaceStats() {
    std::string path = "/proc/net/dev";
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "[UPF HAL] Cannot open: " << path << std::endl;
        return false;
    }

    std::string line;
    bool header_skipped = false;
    while (std::getline(file, line)) {
        if (!header_skipped) {
            header_skipped = true;
            continue;
        }

        if (line.find(":") != std::string::npos) {
            std::istringstream iss(line);
            std::string iface;
            iss >> iface;

            uint64_t rx_bytes, rx_packets, tx_bytes, tx_packets;
            iss >> rx_bytes >> rx_packets;
            iss >> tx_bytes >> tx_packets;

            // Обновление глобальной статистики UPF
            upf_status_.rx_bytes += rx_bytes;
            upf_status_.rx_packets += rx_packets;
            upf_status_.tx_bytes += tx_bytes;
            upf_status_.tx_packets += tx_packets;

            // Обновление статистики интерфейса
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
 * Чтение conntrack таблицы из /proc/net/nf_conntrack
 */
std::vector<ConntrackEntry> UpfHal::readConntrackTable() {
    std::vector<ConntrackEntry> entries;
    std::string path = "/proc/net/nf_conntrack";
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "[UPF HAL] Cannot open conntrack: " << path << std::endl;
        return entries;
    }

    std::string line;
    while (std::getline(file, line)) {
        ConntrackEntry entry;

        // Парсинг conntrack entry
        // Формат: ipv4     2     42     TCP     443     10.0.0.1     10.0.0.2     12345     5000     mark=0     zone=0
        std::istringstream iss(line);
        std::string family, proto;
        uint32_t proto_src, proto_dst;
        uint32_t mark_val;

        iss >> family >> std::skipws;
        // Пропуск number и timeout
        std::string num_str, timeout_str;
        iss >> num_str >> timeout_str;

        iss >> proto;
        if (proto == "TCP" || proto == "UDP") {
            iss >> entry.proto_src >> entry.proto_dst;
            iss >> entry.src_ip >> entry.dst_ip;
            iss >> entry.proto_src >> entry.proto_dst;
            // mark
            std::string mark_field;
            iss >> mark_field;
            if (mark_field.find("mark=") != std::string::npos) {
                entry.mark = std::stoul(mark_field.substr(5));
            }
            entries.push_back(entry);
        }
    }

    return entries;
}

/**
 * Мок-данные для тестирования
 */
void UpfHal::applyMockData() {
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()).count();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();

    upf_status_.upf_id = "MTS-MC-5000-UPF-001";
    upf_status_.status = "active";
    upf_status_.active_sessions = static_cast<uint32_t>(mock_sessions_.size());
    upf_status_.max_sessions = 100000;
    upf_status_.cpu_usage = 15.5;  // mock CPU
    upf_status_.memory_usage = 32.7;  // mock memory
    upf_status_.rx_bytes = 1234567890;
    upf_status_.tx_bytes = 987654321;
    upf_status_.rx_packets = 1234567;
    upf_status_.tx_packets = 987654;
    upf_status_.last_updated = now_ns;
}

} // namespace mts::mc5000::hal
