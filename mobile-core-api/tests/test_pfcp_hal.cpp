/**
 * MTS-MC-5000 — PFCP HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/pfcp_hal.h"

using namespace mts::mc5000::hal;

TEST(PfcpHalTest, DefaultConstructor) {
    PfcpHal hal;
    auto sessions = hal.getStatus();
    EXPECT_TRUE(sessions.empty());
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-MC-5000-PFCP");
}

TEST(PfcpHalTest, AddSession) {
    PfcpHal hal;
    
    PfcpSession session;
    session.session_id = "pfcp-001";
    session.f_seid = "0x12345";
    session.peer_ip = "192.168.10.1";
    session.type = "upf";
    session.status = "established";
    
    auto added = hal.addSession(session);
    EXPECT_TRUE(added);
    
    auto sessions = hal.getStatus();
    EXPECT_EQ(sessions.size(), 1);
    EXPECT_EQ(sessions[0].peer_ip, "192.168.10.1");
}

TEST(PfcpHalTest, DeleteSession) {
    PfcpHal hal;
    
    PfcpSession session;
    session.session_id = "pfcp-001";
    session.f_seid = "0x12345";
    session.peer_ip = "192.168.10.1";
    session.type = "upf";
    session.status = "established";
    
    hal.addSession(session);
    auto deleted = hal.deleteSession("pfcp-001");
    EXPECT_TRUE(deleted);
    
    auto sessions = hal.getStatus();
    EXPECT_TRUE(sessions.empty());
}
