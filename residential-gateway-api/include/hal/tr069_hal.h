/**
 * MTS-RG-500 Residential Gateway — TR-069 HAL
 * CWMP (CPE WAN Management Protocol) agent monitoring
 * 
 * Интеграция с Linux subsystem:
 * - /proc/net/tcp — TR-069 connection monitoring
 * - /proc/net/ — TCP connection statistics
 * - cwmpd daemon — TR-069 state management
 * - /var/log/cwmpd.log — TR-069 event logs
 * - SNMP — ACS connectivity monitoring
 * 
 * Уровень реализации:
 * - Чтение из /proc/net/tcp для мониторинга ACS connections
 * - Парсинь cwmpd logs для event tracking
 * - SNMP monitoring для ACS connectivity
 * - Мок-режим для тестирования без ACS
 * - Thread-safe доступ к общим ресурсам
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <cstdint>

namespace mts::rg500::hal {

struct Tr069Status {
    std::string device_id;
    std::string url;
    bool enabled;
    uint32_t polling_interval;
    int64_t last_poll;
    int64_t next_poll;
    std::string status;     // "active", "inactive", "error"
};

class ITr069Hal {
public:
    virtual ~ITr069Hal() = default;
    virtual Tr069Status getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class Tr069Hal : public ITr069Hal {
public:
    Tr069Hal();
    ~Tr069Hal() override = default;

    Tr069Status getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    void setMockMode(bool enabled);

private:
    bool monitorCwmpd();
    bool updateTcpConnections();
    bool updateFromCwmpd();
    Tr069Status applyMockStatus();

    Tr069Status tr069_status_;
    std::string acs_url_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
    bool cwmpd_running_;
};

} // namespace mts::rg500::hal
