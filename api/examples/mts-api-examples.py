# MTS Router API — Примеры на Python

## 1. Базовый клиент

```python
"""
MTS Router REST API Client
Unified client for all MTS router devices:
- MTS-CR-9000 (Core Router)
- MTS-MC-5000 (Mobile Core)
- MTS-MB-3000 (Mobile Backhaul)
- MTS-OLT-2000 (OLT GPON)
- MTS-ER-1000 (Enterprise Router)
- MTS-RG-500 (Residential Gateway)
"""

import requests
import json
from typing import Optional, Dict, Any, List
from dataclasses import dataclass
from enum import Enum


class MtsDeviceType(Enum):
    CORE_ROUTER = "mts-cr9000"
    MOBILE_CORE = "mts-mc5000"
    MOBILE_BACKHAUL = "mts-mb3000"
    OLT_GPON = "mts-olt2000"
    ENTERPRISE = "mts-er1000"
    RESIDENTIAL = "mts-rg500"


class MtsAuthMethod(Enum):
    MTLS = "mtls"
    API_KEY = "api_key"
    OAUTH2 = "oauth2"
    JWT = "jwt"


@dataclass
class MtsConfig:
    host: str
    port: int = 443
    device_type: MtsDeviceType = MtsDeviceType.CORE_ROUTER
    auth_method: MtsAuthMethod = MtsAuthMethod.API_KEY
    api_key: Optional[str] = None
    cert_file: Optional[str] = None
    key_file: Optional[str] = None
    verify_ssl: bool = True
    timeout: int = 30


class MtsRouterClient:
    """Unified client for MTS Router REST API."""

    def __init__(self, config: MtsConfig):
        self.config = config
        self.base_url = f"https://{config.host}:{config.port}/api/v1"
        self.session = requests.Session()
        self.session.verify = config.verify_ssl

        # Set up authentication
        if config.auth_method == MtsAuthMethod.API_KEY:
            self.session.headers.update({
                "Authorization": f"Bearer {config.api_key}",
                "Content-Type": "application/json"
            })
        elif config.auth_method == MtsAuthMethod.MTLS:
            self.session.cert = (config.cert_file, config.key_file)
        elif config.auth_method == MtsAuthMethod.JWT:
            self._refresh_jwt_token()

    def _refresh_jwt_token(self):
        """Refresh JWT token if needed."""
        # Implementation depends on OAuth2 server
        pass

    def _request(self, method: str, endpoint: str, **kwargs) -> Dict[str, Any]:
        """Make HTTP request."""
        url = f"{self.base_url}{endpoint}"
        response = self.session.request(
            method, url, timeout=self.config.timeout, **kwargs
        )
        response.raise_for_status()
        return response.json()

    # ===== Health & System =====

    def health(self) -> Dict[str, Any]:
        """Get device health status."""
        return self._request("GET", "/health")

    def system_info(self) -> Dict[str, Any]:
        """Get system information."""
        return self._request("GET", "/system")

    # ===== Interfaces =====

    def get_interfaces(self) -> Dict[str, Any]:
        """Get all interfaces."""
        return self._request("GET", "/interfaces")

    def get_interface(self, name: str) -> Dict[str, Any]:
        """Get specific interface."""
        return self._request("GET", f"/interfaces/{name}")

    def get_interface_stats(self, name: str) -> Dict[str, Any]:
        """Get interface statistics."""
        return self._request("GET", f"/interfaces/{name}/stats")

    def set_interface(self, name: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Set interface configuration."""
        return self._request("PUT", f"/interfaces/{name}", json=config)

    # ===== Routing =====

    def get_routing_tables(self) -> Dict[str, Any]:
        """Get all routing tables."""
        return self._request("GET", "/routing/tables")

    def get_routing_table(self, table_id: str) -> Dict[str, Any]:
        """Get specific routing table."""
        return self._request("GET", f"/routing/tables/{table_id}")

    def add_route(self, table_id: str, route: Dict[str, Any]) -> Dict[str, Any]:
        """Add a route."""
        return self._request(
            "POST", f"/routing/tables/{table_id}/routes", json=route
        )

    def delete_route(self, table_id: str, route_id: str) -> Dict[str, Any]:
        """Delete a route."""
        return self._request(
            "DELETE", f"/routing/tables/{table_id}/routes/{route_id}"
        )

    # ===== BGP =====

    def get_bgp(self) -> Dict[str, Any]:
        """Get BGP status."""
        return self._request("GET", "/bgp")

    def get_bgp_neighbors(self) -> Dict[str, Any]:
        """Get BGP neighbors."""
        return self._request("GET", "/bgp/neighbors")

    def add_bgp_neighbor(self, neighbor: Dict[str, Any]) -> Dict[str, Any]:
        """Add BGP neighbor."""
        return self._request("POST", "/bgp/neighbors", json=neighbor)

    def delete_bgp_neighbor(self, peer: str) -> Dict[str, Any]:
        """Delete BGP neighbor."""
        return self._request("DELETE", f"/bgp/neighbors/{peer}")

    def update_bgp_neighbor(self, peer: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update BGP neighbor."""
        return self._request("PUT", f"/bgp/neighbors/{peer}", json=config)

    # ===== Telemetry =====

    def get_telemetry(self, paths: Optional[List[str]] = None) -> Dict[str, Any]:
        """Get telemetry data."""
        params = {"paths": ",".join(paths)} if paths else None
        return self._request("GET", "/telemetry", params=params)

    def subscribe_telemetry(self, paths: List[str], interval: int = 1) -> Any:
        """Subscribe to telemetry streaming (gRPC)."""
        # Implementation depends on gRPC client
        # Returns a stream of TelemetryData
        pass

    # ===== Configuration =====

    def get_config(self) -> Dict[str, Any]:
        """Get running configuration."""
        return self._request("GET", "/config")

    def get_config_backup(self) -> Dict[str, Any]:
        """Get configuration backup."""
        return self._request("GET", "/config/backup")

    def set_config(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """Set configuration."""
        return self._request("POST", "/config", json=config)

    def restore_config(self, backup: Dict[str, Any]) -> Dict[str, Any]:
        """Restore configuration from backup."""
        return self._request("POST", "/config/restore", json=backup)

    # ===== Firmware =====

    def get_firmware_status(self) -> Dict[str, Any]:
        """Get firmware status."""
        return self._request("GET", "/firmware/status")

    def upload_firmware(self, firmware_file: str) -> Dict[str, Any]:
        """Upload firmware image."""
        with open(firmware_file, "rb") as f:
            files = {"firmware": (firmware_file, f, "application/octet-stream")}
            return self._request("POST", "/firmware/upload", files=files)

    def apply_firmware(self) -> Dict[str, Any]:
        """Apply firmware update."""
        return self._request("POST", "/firmware/apply")

    # ===== Logging =====

    def get_logs(self, limit: int = 100) -> Dict[str, Any]:
        """Get system logs."""
        return self._request("GET", "/logs", params={"limit": limit})

    def get_log(self, log_id: str) -> Dict[str, Any]:
        """Get specific log entry."""
        return self._request("GET", f"/logs/{log_id}")

    # ===== Users =====

    def get_users(self) -> Dict[str, Any]:
        """Get all users."""
        return self._request("GET", "/users")

    def create_user(self, user: Dict[str, Any]) -> Dict[str, Any]:
        """Create user."""
        return self._request("POST", "/users", json=user)

    def update_user(self, username: str, user: Dict[str, Any]) -> Dict[str, Any]:
        """Update user."""
        return self._request("PUT", f"/users/{username}", json=user)

    def delete_user(self, username: str) -> Dict[str, Any]:
        """Delete user."""
        return self._request("DELETE", f"/users/{username}")


# ===== Device-specific methods =====


class MtsCoreRouterClient(MtsRouterClient):
    """Client for MTS-CR-9000 Core Router."""

    def get_fabric(self) -> Dict[str, Any]:
        """Get fabric status."""
        return self._request("GET", "/cr/fabric")

    def set_fabric(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """Set fabric configuration."""
        return self._request("PUT", "/cr/fabric", json=config)

    def get_line_cards(self) -> Dict[str, Any]:
        """Get line card status."""
        return self._request("GET", "/cr/line-cards")

    def add_line_card(self, card: Dict[str, Any]) -> Dict[str, Any]:
        """Add line card."""
        return self._request("POST", "/cr/line-cards", json=card)

    def get_p4_pipelines(self) -> Dict[str, Any]:
        """Get P4 pipelines."""
        return self._request("GET", "/cr/p4/pipelines")

    def update_p4_pipeline(self, pipeline_id: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update P4 pipeline."""
        return self._request("PUT", f"/cr/p4/pipelines/{pipeline_id}", json=config)

    def compile_p4(self, p4_code: str) -> Dict[str, Any]:
        """Compile P4 program."""
        return self._request("POST", "/cr/p4/compile", json={"code": p4_code})

    def get_sr6(self) -> Dict[str, Any]:
        """Get SRv6 status."""
        return self._request("GET", "/cr/sr6")

    def set_sr6(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """Set SRv6 configuration."""
        return self._request("PUT", "/cr/sr6", json=config)

    def get_mpls_lsps(self) -> Dict[str, Any]:
        """Get MPLS LSPs."""
        return self._request("GET", "/cr/mpls/lsps")

    def create_mpls_lsp(self, lsp: Dict[str, Any]) -> Dict[str, Any]:
        """Create MPLS LSP."""
        return self._request("POST", "/cr/mpls/lsps", json=lsp)


class MtsMobileCoreClient(MtsRouterClient):
    """Client for MTS-MC-5000 Mobile Core."""

    def get_upf(self) -> Dict[str, Any]:
        """Get UPF status."""
        return self._request("GET", "/mc/upf")

    def create_upf_session(self, session: Dict[str, Any]) -> Dict[str, Any]:
        """Create UPF session."""
        return self._request("POST", "/mc/upf/sessions", json=session)

    def delete_upf_session(self, session_id: str) -> Dict[str, Any]:
        """Delete UPF session."""
        return self._request("DELETE", f"/mc/upf/sessions/{session_id}")

    def get_pfcp(self) -> Dict[str, Any]:
        """Get PFCP status."""
        return self._request("GET", "/mc/pfcp")

    def create_pfcp_steering(self, steering: Dict[str, Any]) -> Dict[str, Any]:
        """Create PFCP steering rule."""
        return self._request("POST", "/mc/pfcp/steering", json=steering)

    def get_gtp(self) -> Dict[str, Any]:
        """Get GTP status."""
        return self._request("GET", "/mc/gtp")

    def get_5qis(self) -> Dict[str, Any]:
        """Get 5QI configurations."""
        return self._request("GET", "/mc/5qi")

    def update_5qi(self, qi_id: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update 5QI configuration."""
        return self._request("PUT", f"/mc/5qi/{qi_id}", json=config)

    def get_nrf(self) -> Dict[str, Any]:
        """Get NRF registry."""
        return self._request("GET", "/mc/nrf")

    def get_nf_registry(self) -> Dict[str, Any]:
        """Get network function registry."""
        return self._request("GET", "/mc/nf/registry")


class MtsMobileBackhaulClient(MtsRouterClient):
    """Client for MTS-MB-3000 Mobile Backhaul."""

    def get_sync(self) -> Dict[str, Any]:
        """Get sync status."""
        return self._request("GET", "/mb/sync")

    def set_grandmaster(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """Set grandmaster configuration."""
        return self._request("PUT", "/mb/sync/grandmaster", json=config)

    def get_ptp(self) -> Dict[str, Any]:
        """Get PTP status."""
        return self._request("GET", "/mb/ptp")

    def get_mpls_tp(self) -> Dict[str, Any]:
        """Get MPLS-TP status."""
        return self._request("GET", "/mb/mpls-tp")

    def create_mpls_tp_pe(self, pe: Dict[str, Any]) -> Dict[str, Any]:
        """Create MPLS-TP pseudowire."""
        return self._request("POST", "/mb/mpls-tp/pe", json=pe)

    def get_sync_e(self) -> Dict[str, Any]:
        """Get SyncE status."""
        return self._request("GET", "/mb/sync-e")


class MtsOltGponClient(MtsRouterClient):
    """Client for MTS-OLT-2000 OLT GPON."""

    def get_olt(self) -> Dict[str, Any]:
        """Get OLT status."""
        return self._request("GET", "/olt")

    def get_pon_ports(self) -> Dict[str, Any]:
        """Get PON ports status."""
        return self._request("GET", "/olt/pon-ports")

    def get_onu(self) -> Dict[str, Any]:
        """Get ONU list."""
        return self._request("GET", "/olt/onu")

    def get_onu_by_id(self, onu_id: str) -> Dict[str, Any]:
        """Get specific ONU."""
        return self._request("GET", f"/olt/onu/{onu_id}")

    def update_onu(self, onu_id: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update ONU configuration."""
        return self._request("PUT", f"/olt/onu/{onu_id}", json=config)

    def reset_onu(self, onu_id: str) -> Dict[str, Any]:
        """Reset ONU."""
        return self._request("POST", f"/olt/onu/{onu_id}/reset")

    def get_omci(self) -> Dict[str, Any]:
        """Get OMCI status."""
        return self._request("GET", "/olt/omci")

    def get_tr069(self) -> Dict[str, Any]:
        """Get TR-069 configuration."""
        return self._request("GET", "/olt/tr069")

    def get_wdm(self) -> Dict[str, Any]:
        """Get WDM status."""
        return self._request("GET", "/olt/wdm")


class MtsEnterpriseRouterClient(MtsRouterClient):
    """Client for MTS-ER-1000 Enterprise Router."""

    def get_sdwan(self) -> Dict[str, Any]:
        """Get SD-WAN status."""
        return self._request("GET", "/er/sdwan")

    def create_sdwan_path(self, path: Dict[str, Any]) -> Dict[str, Any]:
        """Create SD-WAN path."""
        return self._request("POST", "/er/sdwan/path", json=path)

    def update_sdwan_path(self, path_id: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update SD-WAN path."""
        return self._request("PUT", f"/er/sdwan/path/{path_id}", json=config)

    def get_mpls(self) -> Dict[str, Any]:
        """Get MPLS status."""
        return self._request("GET", "/er/mpls")

    def create_mpls_lsp(self, lsp: Dict[str, Any]) -> Dict[str, Any]:
        """Create MPLS LSP."""
        return self._request("POST", "/er/mpls/lsp", json=lsp)

    def get_ipsec(self) -> Dict[str, Any]:
        """Get IPsec status."""
        return self._request("GET", "/er/ipsec")

    def create_ipsec_tunnel(self, tunnel: Dict[str, Any]) -> Dict[str, Any]:
        """Create IPsec tunnel."""
        return self._request("POST", "/er/ipsec/tunnel", json=tunnel)

    def get_vrrp(self) -> Dict[str, Any]:
        """Get VRRP status."""
        return self._request("GET", "/er/vrrp")


class MtsResidentialGatewayClient(MtsRouterClient):
    """Client for MTS-RG-500 Residential Gateway."""

    def get_gpon(self) -> Dict[str, Any]:
        """Get GPON status."""
        return self._request("GET", "/rg/gpon")

    def get_wifi(self) -> Dict[str, Any]:
        """Get WiFi status."""
        return self._request("GET", "/rg/wifi")

    def update_wifi(self, band: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update WiFi configuration."""
        return self._request("PUT", f"/rg/wifi/{band}", json=config)

    def get_voip(self) -> Dict[str, Any]:
        """Get VoIP status."""
        return self._request("GET", "/rg/voip")

    def update_voip(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update VoIP configuration."""
        return self._request("PUT", "/rg/voip", json=config)

    def get_lan(self) -> Dict[str, Any]:
        """Get LAN configuration."""
        return self._request("GET", "/rg/lan")

    def update_lan(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update LAN configuration."""
        return self._request("PUT", "/rg/lan", json=config)

    def get_iptv(self) -> Dict[str, Any]:
        """Get IPTV status."""
        return self._request("GET", "/rg/iptv")

    def get_tr069(self) -> Dict[str, Any]:
        """Get TR-069 configuration."""
        return self._request("GET", "/rg/tr069")

    def get_parental(self) -> Dict[str, Any]:
        """Get parental control status."""
        return self._request("GET", "/rg/parental")

    def update_parental(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update parental control configuration."""
        return self._request("PUT", "/rg/parental", json=config)


# ===== Usage examples =====

def example_core_router():
    """Example usage for MTS-CR-9000."""
    config = MtsConfig(
        host="mts-cr001.mts.ru",
        device_type=MtsDeviceType.CORE_ROUTER,
        api_key="your-api-key-here"
    )
    client = MtsCoreRouterClient(config)

    # Health check
    health = client.health()
    print(f"Health: {health}")

    # Get BGP status
    bgp = client.get_bgp()
    print(f"BGP AS: {bgp['bgp']['as']}")

    # Add route
    route = {
        "prefix": "10.0.0.0/8",
        "next_hop": "192.168.1.1",
        "interface": "eth0",
        "protocol": "static",
        "priority": 10
    }
    result = client.add_route("default", route)
    print(f"Route added: {result}")


def example_mobile_core():
    """Example usage for MTS-MC-5000."""
    config = MtsConfig(
        host="mts-mc001.mts.ru",
        device_type=MtsDeviceType.MOBILE_CORE,
        api_key="your-api-key-here"
    )
    client = MtsMobileCoreClient(config)

    # Create PDU session
    session = {
        "ue_ip": "10.64.0.1",
        "pnni": "ims.mts.ru",
        "qfi": 1,
        "5qi": 9,
        "upf_ip": "192.168.10.1",
        "teid": 0x12345
    }
    result = client.create_upf_session(session)
    print(f"Session created: {result}")


def example_olt():
    """Example usage for MTS-OLT-2000."""
    config = MtsConfig(
        host="mts-olt001.mts.ru",
        device_type=MtsDeviceType.OLT_GPON,
        api_key="your-api-key-here"
    )
    client = MtsOltGponClient(config)

    # Configure ONU
    onu_config = {
        "pon_port": 1,
        "onu_id": 10,
        "vlan": 100,
        "qos_profile": "gold",
        "bandwidth_up": "100 Mbps",
        "bandwidth_down": "300 Mbps"
    }
    result = client.update_onu("001", onu_config)
    print(f"ONU configured: {result}")


def example_residential():
    """Example usage for MTS-RG-500."""
    config = MtsConfig(
        host="mts-rg001.local",
        device_type=MtsDeviceType.RESIDENTIAL,
        api_key="your-api-key-here"
    )
    client = MtsResidentialGatewayClient(config)

    # Configure WiFi
    wifi_config = {
        "ssid": "MTS_Home_5G",
        "channel": 36,
        "bandwidth": "80MHz",
        "security": "wpa3",
        "password": "secure-password"
    }
    result = client.update_wifi("5ghz", wifi_config)
    print(f"WiFi configured: {result}")


if __name__ == "__main__":
    example_core_router()
    example_mobile_core()
    example_olt()
    example_residential()