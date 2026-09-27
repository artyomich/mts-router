/**
 * MTS-RG-500 Residential Gateway — IPTV HAL
 * Hardware Abstraction Layer for IPTV multicast monitoring
 * 
 * Responsibilities:
 * - Monitor IPTV multicast channels (IGMP)
 * - Read multicast group state from /proc/net/igmp
 * - Track bandwidth per active channel
 * - Manage IGMP subscribe/unsubscribe
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <cstdint>

#define MTS_RG_IPTV_MAX_CHANNELS 50

// C-типы для IPTV HAL
typedef struct {
    uint32_t channel_id;
    char name[64];
    char multicast_ip[32];
    uint32_t multicast_port;
    int status;     // 0=inactive, 1=active
    uint32_t viewers;
} mts_rg_iptv_channel_t;

typedef struct {
    int active;
    uint32_t active_channels;
    uint32_t total_channels;
    double bandwidth_mbps;
} mts_rg_iptv_status_t;

typedef enum {
    MTS_RG_IPTV_CHANNEL_INACTIVE = 0,
    MTS_RG_IPTV_CHANNEL_ACTIVE
} mts_rg_iptv_channel_status_t;

namespace mts::rg500::hal {

struct IptvChannel {
    uint32_t channel_id;
    std::string name;
    std::string multicast_ip;
    uint32_t multicast_port;
    std::string status;     // "active", "inactive"
    uint32_t viewers;
};

struct IptvStatus {
    std::string device_id;
    std::string status;     // "active", "inactive", "error"
    uint32_t active_channels;
    uint32_t total_channels;
    double bandwidth_mbps;
    std::vector<IptvChannel> channels;
};

class IIptvHal {
public:
    virtual ~IIptvHal() = default;
    virtual IptvStatus getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class IptvHal : public IIptvHal {
public:
    IptvHal();
    ~IptvHal() override;

    IptvStatus getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    bool subscribeChannel(const std::string& multicast_ip,
                          uint32_t multicast_port);
    bool unsubscribeChannel(const std::string& multicast_ip);

    void setMockMode(bool enabled);

private:
    bool checkIgmpProxyRunning();
    std::vector<std::string> getMulticastGroupsFromProc();
    bool readMulticastGroups();
    bool readMulticastStats();
    double calculateBandwidth();
    IptvStatus applyMockStatus();

    IptvStatus iptv_status_;
    std::vector<IptvChannel> channels_;
    std::vector<std::string> multicast_groups_;
    bool igmp_available_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::rg500::hal
