/**
 * MTS-MB-3000 Mobile Backhaul - PTP HAL
 * 
 * Hardware Abstraction Layer для PTP (Precision Time Protocol)
 * Чтение данных из /sys/class/ptp/ и /var/lib/ptp4l/
 * 
 * Уровень реализации: 
 * - Чтение из sysfs (Linux kernel interface)
 * - Парсинг файлов ptp4l (LinuxPTP daemon)
 * - Мок-режим для тестирования
 */

#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <mutex>
#include <atomic>

// C-типы для PTP HAL
typedef enum {
    MTS_MB_PTP_GRANDMASTER = 0,
    MTS_MB_PTP_BOUNDARY,
    MTS_MB_PTP_ORDINARY
} mts_mb_ptp_mode_t;

typedef enum {
    MTS_MB_PTP_ACTIVE = 0,
    MTS_MB_PTP_INACTIVE,
    MTS_MB_PTP_FAULT
} mts_mb_ptp_status_code_t;

/* PTP status struct for C API */
typedef struct {
    char device_name[64];
    int mode;
    int64_t current_time_ns;
    int64_t offset_from_master_ns;
    int64_t mean_path_delay_ns;
    double frequency_offset_ppm;
    mts_mb_ptp_status_code_t status;
} mts_mb_ptp_status_t;

namespace mts {
namespace hal {

// Структура статуса PTP
struct PtpStatus {
    std::string device_name;      // e.g., "ptp4l"
    std::string mode;             // grandmaster, boundary, ordinary
    int64_t current_time;         // current PTP time in nanoseconds
    int64_t offset_from_master;   // offset from master clock (ns)
    int64_t mean_path_delay;      // mean path delay (ns)
    double frequency_offset;      // frequency offset (ppm)
    std::string status;           // active, inactive, fault
    double phase_offset;          // phase offset (ns)
};

// HAL для PTP
class PtpHal {
public:
    PtpHal();
    ~PtpHal();

    // Получить текущий статус PTP
    PtpStatus getStatus();

    // Установить режим Grandmaster
    bool setGrandmasterMode(bool enable);

    // Проверить доступность PTP subsystem
    bool isAvailable();

    // Получить имя PTP device
    std::string getDeviceName();

private:
    // Чтение из sysfs
    bool readFromSysfs();
    
    // Парсинг файлов ptp4l
    bool parsePtp4lLogs();
    
    // Мок-данные для тестирования
    void setMockMode(bool enable);
    void applyMockData();

    // Внутренние данные
    PtpStatus status_;
    std::mutex mutex_;
    std::string ptp_device_;
    std::string ptp_sysfs_path_;
    bool mock_mode_;
    std::atomic<bool> available_;
};

} // namespace hal
} // namespace mts
