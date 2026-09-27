/**
 * MTS-MB-3000 Mobile Backhaul - SyncE HAL
 * 
 * Hardware Abstraction Layer для SyncE (Synchronous Ethernet)
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

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

// C-типы для SyncE HAL
typedef enum {
    MTS_MB_SYNCE_MASTER = 0,
    MTS_MB_SYNCE_SLAVE,
    MTS_MB_SYNCE_TRANSPARENT
} mts_mb_synce_mode_t;

typedef enum {
    MTS_MB_SYNCE_LOCKED = 0,
    MTS_MB_SYNCE_UNLOCKED,
    MTS_MB_SYNCE_HOLD_OVER
} mts_mb_synce_status_t;

typedef struct {
    char port_name[32];
    mts_mb_synce_mode_t mode;
    int32_t target_frequency;
    double actual_frequency;
    double phase_offset_ns;
    mts_mb_synce_status_t status;
} mts_mb_synce_port_t;

#define MTS_MB_SYNCE_MAX_PORTS 8

#include "mpls_hal.h"

namespace mts {
namespace hal {

// Структура статуса SyncE
struct SyncEStatus {
    std::string port_name;      // e.g., "eth0"
    std::string mode;           // master, slave, transparent
    int32_t frequency;          // target frequency (Hz)
    double actual_frequency;    // actual frequency (Hz)
    double phase_offset;        // phase offset (ns)
    std::string status;         // locked, unlocked, holdover
};

// HAL для SyncE
class SyncEHal {
public:
    SyncEHal();
    ~SyncEHal();

    // Получить статус всех SyncE портов
    std::vector<SyncEStatus> getStatus();

    // Установить режим для конкретного порта
    bool setMode(const std::string& port_name, const std::string& mode);

    // Проверить доступность SyncE
    bool isAvailable();

    // Получить список портов
    std::vector<std::string> getPortList();

private:
    // Чтение из sysfs
    bool readFromSysfs();
    
    // Парсинг /proc/net
    bool parseProcNet();
    
    // Настройка через ioctl
    bool configurePort(const std::string& port_name, const std::string& mode);

    // Внутренние данные
    std::vector<SyncEStatus> status_list_;
    std::mutex mutex_;
    std::vector<std::string> port_list_;
    std::atomic<bool> available_;
};

} // namespace hal
} // namespace mts
