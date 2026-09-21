/**
 * MTS-MB-3000 Mobile Backhaul - MPLS-TP HAL
 * 
 * Hardware Abstraction Layer для MPLS-TP (Multiprotocol Label Switching - 
 * Transport Profile) pseudowires
 * Интеграция с Linux subsystem:
 * - /sys/class/net/ethX/ - интерфейс Ethernet
 * - /proc/net/ - network statistics
 * - iproute2 для управления MPLS
 * 
 * Уровень реализации:
 * - Чтение из sysfs для мониторинга
 * - iproute2 для настройки MPLS
 * - Парсинг /proc/net для статистики
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

namespace mts {
namespace hal {

// Структура статуса MPLS-TP pseudowire
struct MplsTpPwStatus {
    uint32_t pw_id;               // pseudowire ID
    std::string ingress_port;     // ingress port name
    std::string egress_port;      // egress port name
    std::string encapsulation;    // eth, hdlc, unstructured
    uint32_t qos_class;           // QoS class
    uint64_t rx_bytes;            // received bytes
    uint64_t tx_bytes;            // transmitted bytes
    uint64_t rx_packets;          // received packets
    uint64_t tx_packets;          // transmitted packets
    uint64_t rx_errors;           // received errors
    uint64_t tx_errors;           // transmitted errors
    std::string status;           // up, down, initializing
};

// HAL для MPLS-TP
class MplsTpHal {
public:
    MplsTpHal();
    ~MplsTpHal();

    // Получить статус всех pseudowires
    std::vector<MplsTpPwStatus> getStatus();

    // Создать новый pseudowire
    bool createPw(uint32_t pw_id, const std::string& ingress,
                  const std::string& egress, const std::string& encap,
                  uint32_t qos_class);

    // Удалить pseudowire
    bool deletePw(uint32_t pw_id);

    // Проверить доступность MPLS
    bool isAvailable();

    // Получить список pseudowires
    std::vector<uint32_t> getPwList();

private:
    // Чтение из sysfs
    bool readFromSysfs();
    
    // Парсинг /proc/net
    bool parseProcNet();
    
    // Настройка через iproute2
    bool configurePw(const std::string& command);

    // Внутренние данные
    std::vector<MplsTpPwStatus> pw_list_;
    std::mutex mutex_;
    std::vector<uint32_t> pw_id_list_;
    std::atomic<bool> available_;
};

} // namespace hal
} // namespace mts
