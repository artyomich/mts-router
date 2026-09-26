/**
 * MTS-MC-5000 — UPF HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/upf_hal.h"

using namespace mts::mc5000::hal;

TEST(UpfHalTest, DefaultConstructor) {
    UpfHal hal;
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-MC-5000-UPF");
}

TEST(UpfHalTest, GetStatus) {
    UpfHal hal;
    auto status = hal.getStatus();
    
    EXPECT_EQ(status.upf_id, "MTS-MC-5000-UPF-001");
    EXPECT_EQ(status.status, "active");
    EXPECT_EQ(status.max_sessions, 100000);
    EXPECT_GE(status.cpu_usage, 0.0);
    EXPECT_GE(status.memory_usage, 0.0);
}

TEST(UpfHalTest, CreatePduSession) {
    UpfHal hal;
    
    PduSession session;
    session.session_id = "sess-001";
    session.ue_ip = "10.64.0.1";
    session.upf_ip = "192.168.10.1";
    session.teid = 0x12345;
    session.qfi = 1;
    session.five_qi = 9;
    session.pnni = "ims.mts.ru";
    session.status = "active";
    session.created_at = 0;
    session.last_active = 0;
    session.rx_bytes = 0;
    session.tx_bytes = 0;
    
    auto created = hal.createPduSession(session);
    EXPECT_TRUE(created);
    
    auto sessions = hal.getPduSessions();
    EXPECT_EQ(sessions.size(), 1);
    EXPECT_EQ(sessions[0].ue_ip, "10.64.0.1");
}

TEST(UpfHalTest, DeletePduSession) {
    UpfHal hal;
    
    PduSession session;
    session.session_id = "sess-001";
    session.ue_ip = "10.64.0.1";
    session.upf_ip = "192.168.10.1";
    session.teid = 0x12345;
    session.qfi = 1;
    session.five_qi = 9;
    session.pnni = "ims.mts.ru";
    session.status = "active";
    session.created_at = 0;
    session.last_active = 0;
    session.rx_bytes = 0;
    session.tx_bytes = 0;
    
    hal.createPduSession(session);
    auto deleted = hal.deletePduSession("sess-001");
    EXPECT_TRUE(deleted);
    
    auto sessions = hal.getPduSessions();
    EXPECT_TRUE(sessions.empty());
}

TEST(UpfHalTest, DeleteNonExistentSession) {
    UpfHal hal;
    auto deleted = hal.deletePduSession("nonexistent");
    EXPECT_FALSE(deleted);
}

TEST(UpfHalTest, MockMode) {
    UpfHal hal;
    hal.setMockMode(true);
    
    PduSession session;
    session.session_id = "mock-001";
    session.ue_ip = "10.64.0.1";
    session.upf_ip = "192.168.10.1";
    session.teid = 0x12345;
    session.qfi = 1;
    session.five_qi = 9;
    session.pnni = "ims.mts.ru";
    session.status = "active";
    session.created_at = 0;
    session.last_active = 0;
    session.rx_bytes = 0;
    session.tx_bytes = 0;
    
    hal.setMockSessions({session});
    
    auto sessions = hal.getPduSessions();
    EXPECT_EQ(sessions.size(), 1);
    EXPECT_EQ(sessions[0].session_id, "mock-001");
}
