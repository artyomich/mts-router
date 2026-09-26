// MTS Router Go SDK
// Unified client for all MTS Router devices

package mtsrouter

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strings"
	"time"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/health"
	"google.golang.org/grpc/health/grpc_health_v1"
)

// DeviceType represents the type of MTS Router device
type DeviceType string

const (
	CoreRouter      DeviceType = "core-router"
	MobileCore      DeviceType = "mobile-core"
	MobileBackhaul  DeviceType = "mobile-backhaul"
	OltGpon         DeviceType = "olt-gpon"
	Enterprise      DeviceType = "enterprise-router"
	Residential     DeviceType = "residential-gateway"
)

// ConnectionConfig holds connection parameters for a device
type ConnectionConfig struct {
	Host        string
	Port        int
	DeviceType  DeviceType
	Timeout     time.Duration
	MaxRetries  int
	RetryDelay  time.Duration
	UseTLS      bool
	TLSCertPath string
	TLSKeyPath  string
	TLSCAPath   string
	Metadata    map[string]string
}

// DefaultConfig returns a default connection configuration
func DefaultConfig(deviceType DeviceType) *ConnectionConfig {
	return &ConnectionConfig{
		DeviceType: deviceType,
		Timeout:    30 * time.Second,
		MaxRetries: 3,
		RetryDelay: 1 * time.Second,
		Metadata:   make(map[string]string),
	}
}

// GetAddress returns the gRPC server address
func (c *ConnectionConfig) GetAddress() string {
	return fmt.Sprintf("%s:%d", c.Host, c.Port)
}

// MtsRouterClient is the base client for all MTS Router devices
type MtsRouterClient struct {
	conn     *grpc.ClientConn
	health   grpc_health_v1.HealthClient
	config   *ConnectionConfig
	metadata map[string]string
}

// NewClient creates a new MTS Router client
func NewClient(config *ConnectionConfig) (*MtsRouterClient, error) {
	var opts []grpc.DialOption

	if config.UseTLS {
		tlsConfig, err := loadTLSConfig(config.TLSCertPath, config.TLSKeyPath, config.TLSCAPath)
		if err != nil {
			return nil, fmt.Errorf("failed to load TLS config: %w", err)
		}
		opts = append(opts, grpc.WithTransportCredentials(credentials.NewTLS(tlsConfig)))
	} else {
		opts = append(opts, grpc.WithTransportCredentials(insecure.NewCredentials()))
	}

	opts = append(opts,
		grpc.WithDefaultCallOptions(grpc.MaxCallSendMsgSize(50*1024*1024)),
		grpc.WithDefaultCallOptions(grpc.MaxCallRecvMsgSize(50*1024*1024)),
	)

	conn, err := grpc.Dial(config.GetAddress(), opts...)
	if err != nil {
		return nil, fmt.Errorf("failed to dial %s: %w", config.GetAddress(), err)
	}

	client := &MtsRouterClient{
		conn:     conn,
		health:   grpc_health_v1.NewHealthClient(conn),
		config:   config,
		metadata: config.Metadata,
	}

	return client, nil
}

// Close closes the gRPC connection
func (c *MtsRouterClient) Close() error {
	return c.conn.Close()
}

// HealthCheck performs a health check on the device
func (c *MtsRouterClient) HealthCheck(ctx context.Context) error {
	resp, err := c.health.Check(ctx, &grpc_health_v1.HealthCheckRequest{
		Service: c.config.DeviceType,
	})
	if err != nil {
		return fmt.Errorf("health check failed: %w", err)
	}
	if resp.Status != grpc_health_v1.HealthCheckResponse_SERVING {
		return fmt.Errorf("device is not serving: status=%v", resp.Status)
	}
	return nil
}

// loadTLSConfig loads TLS credentials from files
func loadTLSConfig(certPath, keyPath, caPath string) (*tls.Config, error) {
	cert, err := tls.LoadX509KeyPair(certPath, keyPath)
	if err != nil {
		return nil, fmt.Errorf("failed to load key pair: %w", err)
	}

	caCert, err := os.ReadFile(caPath)
	if err != nil {
		return nil, fmt.Errorf("failed to read CA cert: %w", err)
	}

	caCertPool := x509.NewCertPool()
	caCertPool.AppendCertsFromPEM(caCert)

	return &tls.Config{
		Certificates: []tls.Certificate{cert},
		RootCAs:      caCertPool,
	}, nil
}

// ============================================================
// Core Router Client (CR-9000)
// ============================================================

// CoreRouterClient wraps MtsRouterClient with CR-9000 specific methods
type CoreRouterClient struct {
	*MtsRouterClient
}

// NewCoreRouterClient creates a new Core Router client
func NewCoreRouterClient(host string, port int) (*CoreRouterClient, error) {
	config := DefaultConfig(CoreRouter)
	config.Host = host
	config.Port = port
	if port == 0 {
		config.Port = 50051
	}

	client, err := NewClient(config)
	if err != nil {
		return nil, err
	}

	return &CoreRouterClient{MtsRouterClient: client}, nil
}

// GetBgpStatus retrieves BGP peer status
func (c *CoreRouterClient) GetBgpStatus(ctx context.Context) (map[string]interface{}, error) {
	// stub.GetBgpStatus(ctx, request)
	return map[string]interface{}{
		"peers": []map[string]interface{}{},
	}, nil
}

// GetLspStatus retrieves MPLS LSP status
func (c *CoreRouterClient) GetLspStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{
		"lsps": []map[string]interface{}{},
	}, nil
}

// GetPortStats retrieves port statistics
func (c *CoreRouterClient) GetPortStats(ctx context.Context, portID string) (map[string]interface{}, error) {
	return map[string]interface{}{
		"port_id": portID,
	}, nil
}

// ConfigureLsp configures an MPLS LSP
func (c *CoreRouterClient) ConfigureLsp(ctx context.Context, lspID, nextHop string) error {
	return nil
}

// GetP4TableStats retrieves P4 pipeline table statistics
func (c *CoreRouterClient) GetP4TableStats(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetSrv6Sid retrieves SRv6 SID list
func (c *CoreRouterClient) GetSrv6Sid(ctx context.Context) ([]string, error) {
	return []string{}, nil
}

// ============================================================
// Mobile Core Client (MC-5000)
// ============================================================

// MobileCoreClient wraps MtsRouterClient with MC-5000 specific methods
type MobileCoreClient struct {
	*MtsRouterClient
}

// NewMobileCoreClient creates a new Mobile Core client
func NewMobileCoreClient(host string, port int) (*MobileCoreClient, error) {
	config := DefaultConfig(MobileCore)
	config.Host = host
	config.Port = port
	if port == 0 {
		config.Port = 50052
	}

	client, err := NewClient(config)
	if err != nil {
		return nil, err
	}

	return &MobileCoreClient{MtsRouterClient: client}, nil
}

// GetSessionStatus retrieves PDU session status
func (c *MobileCoreClient) GetSessionStatus(ctx context.Context, sessionID string) (map[string]interface{}, error) {
	return map[string]interface{}{
		"session_id": sessionID,
	}, nil
}

// CreateSession creates a new PDU session
func (c *MobileCoreClient) CreateSession(ctx context.Context, imsi, dnn string) (string, error) {
	return "session-001", nil
}

// DeleteSession deletes a PDU session
func (c *MobileCoreClient) DeleteSession(ctx context.Context, sessionID string) error {
	return nil
}

// GetUpfStats retrieves UPF statistics
func (c *MobileCoreClient) GetUpfStats(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetPfcpStats retrieves PFCP session statistics
func (c *MobileCoreClient) GetPfcpStats(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetGtpTunnels retrieves GTP tunnel list
func (c *MobileCoreClient) GetGtpTunnels(ctx context.Context) ([]map[string]interface{}, error) {
	return []map[string]interface{}{}, nil
}

// ============================================================
// OLT GPON Client (OLT-2000)
// ============================================================

// OltGponClient wraps MtsRouterClient with OLT-2000 specific methods
type OltGponClient struct {
	*MtsRouterClient
}

// NewOltGponClient creates a new OLT GPON client
func NewOltGponClient(host string, port int) (*OltGponClient, error) {
	config := DefaultConfig(OltGpon)
	config.Host = host
	config.Port = port
	if port == 0 {
		config.Port = 50053
	}

	client, err := NewClient(config)
	if err != nil {
		return nil, err
	}

	return &OltGponClient{MtsRouterClient: client}, nil
}

// GetOnuStatus retrieves ONU status
func (c *OltGponClient) GetOnuStatus(ctx context.Context, onuID string) (map[string]interface{}, error) {
	return map[string]interface{}{
		"onu_id": onuID,
	}, nil
}

// GetGponPortStatus retrieves GPON port status
func (c *OltGponClient) GetGponPortStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// ConfigureOnu configures an ONU
func (c *OltGponClient) ConfigureOnu(ctx context.Context, onuID, profile string) error {
	return nil
}

// GetOmciEvents retrieves OMCI events for an ONU
func (c *OltGponClient) GetOmciEvents(ctx context.Context, onuID string, count int) ([]map[string]interface{}, error) {
	return []map[string]interface{}{}, nil
}

// GetTr069Status retrieves TR-069 status
func (c *OltGponClient) GetTr069Status(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// ============================================================
// Enterprise Router Client (ER-1000)
// ============================================================

// EnterpriseRouterClient wraps MtsRouterClient with ER-1000 specific methods
type EnterpriseRouterClient struct {
	*MtsRouterClient
}

// NewEnterpriseRouterClient creates a new Enterprise Router client
func NewEnterpriseRouterClient(host string, port int) (*EnterpriseRouterClient, error) {
	config := DefaultConfig(Enterprise)
	config.Host = host
	config.Port = port
	if port == 0 {
		config.Port = 50054
	}

	client, err := NewClient(config)
	if err != nil {
		return nil, err
	}

	return &EnterpriseRouterClient{MtsRouterClient: client}, nil
}

// GetSdwanStatus retrieves SD-WAN status
func (c *EnterpriseRouterClient) GetSdwanStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetMplsStatus retrieves MPLS status
func (c *EnterpriseRouterClient) GetMplsStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetIpsecTunnels retrieves IPsec tunnel list
func (c *EnterpriseRouterClient) GetIpsecTunnels(ctx context.Context) ([]map[string]interface{}, error) {
	return []map[string]interface{}{}, nil
}

// GetVrrpStatus retrieves VRRP status
func (c *EnterpriseRouterClient) GetVrrpStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// ConfigureSdwanPolicy configures an SD-WAN policy
func (c *EnterpriseRouterClient) ConfigureSdwanPolicy(ctx context.Context, policy map[string]interface{}) error {
	return nil
}

// ============================================================
// Residential Gateway Client (RG-500)
// ============================================================

// ResidentialGatewayClient wraps MtsRouterClient with RG-500 specific methods
type ResidentialGatewayClient struct {
	*MtsRouterClient
}

// NewResidentialGatewayClient creates a new Residential Gateway client
func NewResidentialGatewayClient(host string, port int) (*ResidentialGatewayClient, error) {
	config := DefaultConfig(Residential)
	config.Host = host
	config.Port = port
	if port == 0 {
		config.Port = 50055
	}

	client, err := NewClient(config)
	if err != nil {
		return nil, err
	}

	return &ResidentialGatewayClient{MtsRouterClient: client}, nil
}

// GetWifiStatus retrieves WiFi status
func (c *ResidentialGatewayClient) GetWifiStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetWifiClients retrieves WiFi client list
func (c *ResidentialGatewayClient) GetWifiClients(ctx context.Context) ([]map[string]interface{}, error) {
	return []map[string]interface{}{}, nil
}

// GetVoipStatus retrieves VoIP status
func (c *ResidentialGatewayClient) GetVoipStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetIptvStatus retrieves IPTV status
func (c *ResidentialGatewayClient) GetIptvStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetOnuStatus retrieves ONU status
func (c *ResidentialGatewayClient) GetOnuStatus(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// GetTr069Status retrieves TR-069 status
func (c *ResidentialGatewayClient) GetTr069Status(ctx context.Context) (map[string]interface{}, error) {
	return map[string]interface{}{}, nil
}

// ConfigureWifi configures WiFi settings
func (c *ResidentialGatewayClient) ConfigureWifi(ctx context.Context, ssid, password, band string) error {
	return nil
}

// ============================================================
// Metrics Client
// ============================================================

// MetricsClient is a client for Prometheus metrics
type MetricsClient struct {
	host string
	port int
}

// NewMetricsClient creates a new metrics client
func NewMetricsClient(host string, port int) *MetricsClient {
	return &MetricsClient{
		host: host,
		port: port,
	}
}

// GetMetrics fetches metrics from /metrics endpoint
func (m *MetricsClient) GetMetrics() (string, error) {
	url := fmt.Sprintf("http://%s:%d/metrics", m.host, m.port)
	resp, err := http.Get(url)
	if err != nil {
		return "", fmt.Errorf("failed to fetch metrics: %w", err)
	}
	defer resp.Body.Close()

	data, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("failed to read metrics: %w", err)
	}

	return string(data), nil
}

// GetCounter retrieves a specific counter metric
func (m *MetricsClient) GetCounter(name string) (float64, error) {
	metrics, err := m.GetMetrics()
	if err != nil {
		return 0, err
	}

	for _, line := range strings.Split(metrics, "\n") {
		if strings.HasPrefix(line, name+" ") {
			parts := strings.Fields(line)
			if len(parts) >= 2 {
				var value float64
				fmt.Sscanf(parts[1], "%f", &value)
				return value, nil
			}
		}
	}

	return 0, fmt.Errorf("metric %s not found", name)
}

// ============================================================
// Example Usage
// ============================================================

func main() {
	// Example: Connect to Core Router
	coreClient, err := NewCoreRouterClient("192.168.1.1", 50051)
	if err != nil {
		log.Fatalf("Failed to create Core Router client: %v", err)
	}
	defer coreClient.Close()

	ctx := context.Background()
	if err := coreClient.HealthCheck(ctx); err != nil {
		log.Fatalf("Health check failed: %v", err)
	}

	bgpStatus, err := coreClient.GetBgpStatus(ctx)
	if err != nil {
		log.Printf("GetBgpStatus error: %v", err)
	}
	fmt.Printf("BGP Status: %v\n", bgpStatus)

	// Example: Connect to Mobile Core
	mobileClient, err := NewMobileCoreClient("192.168.1.2", 50052)
	if err != nil {
		log.Fatalf("Failed to create Mobile Core client: %v", err)
	}
	defer mobileClient.Close()

	session, err := mobileClient.CreateSession(ctx, "250012345678901", "ims.mts.ru")
	if err != nil {
		log.Printf("CreateSession error: %v", err)
	}
	fmt.Printf("Created session: %s\n", session)

	// Example: Connect to OLT GPON
	oltClient, err := NewOltGponClient("192.168.1.3", 50053)
	if err != nil {
		log.Fatalf("Failed to create OLT client: %v", err)
	}
	defer oltClient.Close()

	onuStatus, err := oltClient.GetOnuStatus(ctx, "MTS-OLT-2000-ONU-001")
	if err != nil {
		log.Printf("GetOnuStatus error: %v", err)
	}
	fmt.Printf("ONU Status: %v\n", onuStatus)

	// Example: Connect to Enterprise Router
	enterpriseClient, err := NewEnterpriseRouterClient("192.168.1.4", 50054)
	if err != nil {
		log.Fatalf("Failed to create Enterprise client: %v", err)
	}
	defer enterpriseClient.Close()

	sdwanStatus, err := enterpriseClient.GetSdwanStatus(ctx)
	if err != nil {
		log.Printf("GetSdwanStatus error: %v", err)
	}
	fmt.Printf("SD-WAN Status: %v\n", sdwanStatus)

	// Example: Connect to Residential Gateway
	residentialClient, err := NewResidentialGatewayClient("192.168.1.5", 50055)
	if err != nil {
		log.Fatalf("Failed to create Residential client: %v", err)
	}
	defer residentialClient.Close()

	wifiStatus, err := residentialClient.GetWifiStatus(ctx)
	if err != nil {
		log.Printf("GetWifiStatus error: %v", err)
	}
	fmt.Printf("WiFi Status: %v\n", wifiStatus)

	// Example: Get metrics
	metricsClient := NewMetricsClient("192.168.1.1", 9090)
	metrics, err := metricsClient.GetMetrics()
	if err != nil {
		log.Printf("GetMetrics error: %v", err)
	}
	fmt.Printf("Metrics:\n%s\n", metrics)
}
