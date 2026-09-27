/**
 * MTS-RG-500 VoIP Engine — VoIP (Asterisk) monitoring
 * Provides mock implementation for VoIP line status
 */

#include "hal/voip_hal.h"
#include <cstdio>
#include <cstring>

namespace mts::rg500::hal {

VoipHal::VoipHal() : mock_mode_(false), asterisk_available_(true), available_(true) {
    memset(&voip_status_, 0, sizeof(voip_status_));
    voip_status_.device_id = "MTS-RG-500-VOIP";
    voip_status_.status = "active";
    voip_status_.total_lines = 2;
    voip_status_.active_calls = 0;
    voip_status_.total_calls = 0;
    voip_status_.codec = "g711";
    voip_status_.sample_rate = 8000;
}

VoipStatus VoipHal::getStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mock_mode_) {
        return applyMockStatus();
    }
    return voip_status_;
}

bool VoipHal::isAvailable() {
    return asterisk_available_ && available_;
}

std::string VoipHal::getDeviceName() {
    return "MTS-RG-500-VOIP-Asterisk";
}

void VoipHal::setMockMode(bool enabled) {
    mock_mode_ = enabled;
}

bool VoipHal::checkAsteriskRunning() {
    return asterisk_available_;
}

std::vector<std::string> VoipHal::getLineListFromAsterisk() {
    return {"1", "2"};
}

bool VoipHal::readCallsFromAsterisk() { return true; }
bool VoipHal::readRtpStats() { return true; }
bool VoipHal::readAudioQuality() { return true; }

VoipStatus VoipHal::applyMockStatus() {
    VoipStatus mock;
    mock.device_id = "MOCK-VOIP-001";
    mock.status = "active";
    mock.total_lines = 2;
    mock.active_calls = 0;
    mock.total_calls = 0;
    mock.codec = "g711";
    mock.sample_rate = 8000;

    VoipLine line1, line2;
    line1.line_id = 1;
    line1.status = "idle";
    line1.codec = "g711";
    line1.rtp_port = 10000;

    line2.line_id = 2;
    line2.status = "idle";
    line2.codec = "g711";
    line2.rtp_port = 10002;

    mock.lines.push_back(line1);
    mock.lines.push_back(line2);
    return mock;
}

} // namespace mts::rg500::hal
