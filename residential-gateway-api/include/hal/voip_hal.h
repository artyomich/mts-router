/**
 * MTS-RG-500 Residential Gateway — VoIP HAL
 * Hardware Abstraction Layer for VoIP (Asterisk) monitoring
 * 
 * Responsibilities:
 * - Monitor POTS line status and call state
 * - Read RTP streams from /proc/net/udp
 * - Monitor Asterisk AMI for call events
 * - Track audio quality via ALSA
 * - Thread-safe with mock mode for testing
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <cstdint>

namespace mts::rg500::hal {

struct VoipLine {
    uint32_t line_id;
    std::string status;     // "idle", "ringing", "active", "busy"
    std::string caller_id;
    std::string callee_id;
    uint32_t duration_seconds;
    std::string codec;      // "g711", "g729", "opus"
    uint32_t rtp_port;
};

struct VoipStatus {
    std::string device_id;
    std::string status;     // "active", "inactive", "error"
    uint32_t total_lines;
    uint32_t active_calls;
    uint32_t total_calls;
    std::string codec;
    uint32_t sample_rate;
    std::vector<VoipLine> lines;
};

class IVoipHal {
public:
    virtual ~IVoipHal() = default;
    virtual VoipStatus getStatus() = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getDeviceName() = 0;
};

class VoipHal : public IVoipHal {
public:
    VoipHal();
    ~VoipHal() override = default;

    VoipStatus getStatus() override;
    bool isAvailable() override;
    std::string getDeviceName() override;

    void setMockMode(bool enabled);

private:
    bool checkAsteriskRunning();
    std::vector<std::string> getLineListFromAsterisk();
    bool readCallsFromAsterisk();
    bool readRtpStats();
    bool readAudioQuality();
    VoipStatus applyMockStatus();

    VoipStatus voip_status_;
    std::vector<std::string> line_list_;
    bool asterisk_available_;
    mutable std::mutex mutex_;
    std::atomic<bool> mock_mode_;
    bool available_;
};

} // namespace mts::rg500::hal
