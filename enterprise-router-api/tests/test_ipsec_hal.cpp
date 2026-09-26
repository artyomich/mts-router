/**
 * MTS-ER-1000 — IPsec HAL Unit Tests
 */

#include <gtest/gtest.h>
#include "hal/ipsec_hal.h"

using namespace mts::er1000::hal;

TEST(IpsecHalTest, DefaultConstructor) {
    IpsecHal hal;
    auto tunnels = hal.getTunnelStatus();
    EXPECT_TRUE(tunnels.empty());
    EXPECT_TRUE(hal.isAvailable());
    EXPECT_EQ(hal.getDeviceName(), "MTS-ER-1000-IPSEC");
}

TEST(IpsecHalTest, CreateTunnel) {
    IpsecHal hal;
    
    IpsecTunnelStatus tunnel;
    tunnel.tunnel_id = "tun-001";
    tunnel.name = "tunnel-001";
    tunnel.peer_ip = "203.0.113.1";
    tunnel.local_subnet = "192.168.1.0/24";
    tunnel.remote_subnet = "10.0.0.0/24";
    tunnel.mode = "tunnel";
    tunnel.status = "negotiating";
    tunnel.phase = 1;
    tunnel.rx_bytes = 0;
    tunnel.tx_bytes = 0;
    tunnel.rx_packets = 0;
    tunnel.tx_packets = 0;
    tunnel.established_at = 0;
    
    auto created = hal.createTunnel(tunnel);
    EXPECT_TRUE(created);
    
    auto tunnels = hal.getTunnelStatus();
    EXPECT_EQ(tunnels.size(), 1);
    EXPECT_EQ(tunnels[0].peer_ip, "203.0.113.1");
}

TEST(IpsecHalTest, DeleteTunnel) {
    IpsecHal hal;
    
    IpsecTunnelStatus tunnel;
    tunnel.tunnel_id = "tun-001";
    tunnel.name = "tunnel-001";
    tunnel.peer_ip = "203.0.113.1";
    tunnel.local_subnet = "192.168.1.0/24";
    tunnel.remote_subnet = "10.0.0.0/24";
    tunnel.mode = "tunnel";
    tunnel.status = "negotiating";
    tunnel.phase = 1;
    tunnel.rx_bytes = 0;
    tunnel.tx_bytes = 0;
    tunnel.rx_packets = 0;
    tunnel.tx_packets = 0;
    tunnel.established_at = 0;
    
    hal.createTunnel(tunnel);
    auto deleted = hal.deleteTunnel("tun-001");
    EXPECT_TRUE(deleted);
    
    auto tunnels = hal.getTunnelStatus();
    EXPECT_TRUE(tunnels.empty());
}
