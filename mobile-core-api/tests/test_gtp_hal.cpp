/**
 * MTS-MC-5000 — GTP HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/gtp_hal.h"

using namespace mts::mc5000::hal;

TEST(GtpHalTest, DefaultConstructor) {
    GtpHal hal;
    auto tunnels = hal.getTunnels();
    EXPECT_TRUE(tunnels.empty());
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-MC-5000-GTP");
}

TEST(GtpHalTest, CreateTunnel) {
    GtpHal hal;
    
    GtpTunnel tunnel;
    tunnel.tunnel_id = "gtp-001";
    tunnel.local_ip = "10.64.0.1";
    tunnel.remote_ip = "192.168.10.1";
    tunnel.local_teid = 0x10000;
    tunnel.remote_teid = 0x20000;
    tunnel.type = "gtp-u";
    tunnel.status = "active";
    tunnel.rx_bytes = 0;
    tunnel.tx_bytes = 0;
    
    auto created = hal.createTunnel(tunnel);
    EXPECT_TRUE(created);
    
    auto tunnels = hal.getTunnels();
    EXPECT_EQ(tunnels.size(), 1);
    EXPECT_EQ(tunnels[0].local_ip, "10.64.0.1");
}

TEST(GtpHalTest, DeleteTunnel) {
    GtpHal hal;
    
    GtpTunnel tunnel;
    tunnel.tunnel_id = "gtp-001";
    tunnel.local_ip = "10.64.0.1";
    tunnel.remote_ip = "192.168.10.1";
    tunnel.local_teid = 0x10000;
    tunnel.remote_teid = 0x20000;
    tunnel.type = "gtp-u";
    tunnel.status = "active";
    tunnel.rx_bytes = 0;
    tunnel.tx_bytes = 0;
    
    hal.createTunnel(tunnel);
    auto deleted = hal.deleteTunnel("gtp-001");
    EXPECT_TRUE(deleted);
    
    auto tunnels = hal.getTunnels();
    EXPECT_TRUE(tunnels.empty());
}
