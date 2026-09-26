// MTS Router Java SDK
// Unified client for all MTS Router devices

package mtsrouter;

import io.grpc.*;
import io.grpc.health.v1.HealthCheckRequest;
import io.grpc.health.v1.HealthCheckResponse;
import io.grpc.health.v1.HealthGrpc;
import io.grpc.netty.NettyChannelBuilder;
import io.grpc.netty.GrpcSslContexts;
import io.netty.handler.ssl.SslContext;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;
import java.io.FileInputStream;
import java.security.KeyStore;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;

/**
 * Connection configuration for MTS Router device.
 */
public class ConnectionConfig {
    private final String host;
    private final int port;
    private final DeviceType deviceType;
    private final long timeoutSeconds;
    private final int maxRetries;
    private final long retryDelayMs;
    private final boolean useTls;
    private final String tlsCertPath;
    private final String tlsKeyPath;
    private final String tlsCaPath;
    private final Map<String, String> metadata;

    public ConnectionConfig(String host, int port, DeviceType deviceType) {
        this(host, port, deviceType, 30, 3, 1000, false, null, null, null);
    }

    public ConnectionConfig(String host, int port, DeviceType deviceType, long timeoutSeconds,
                           int maxRetries, long retryDelayMs, boolean useTls,
                           String tlsCertPath, String tlsKeyPath, String tlsCaPath) {
        this.host = host;
        this.port = port;
        this.deviceType = deviceType;
        this.timeoutSeconds = timeoutSeconds;
        this.maxRetries = maxRetries;
        this.retryDelayMs = retryDelayMs;
        this.useTls = useTls;
        this.tlsCertPath = tlsCertPath;
        this.tlsKeyPath = tlsKeyPath;
        this.tlsCaPath = tlsCaPath;
        this.metadata = new HashMap<>();
    }

    public String getHost() { return host; }
    public int getPort() { return port; }
    public DeviceType getDeviceType() { return deviceType; }
    public long getTimeoutSeconds() { return timeoutSeconds; }
    public int getMaxRetries() { return maxRetries; }
    public long getRetryDelayMs() { return retryDelayMs; }
    public boolean isUseTls() { return useTls; }
    public String getTlsCertPath() { return tlsCertPath; }
    public String getTlsKeyPath() { return tlsKeyPath; }
    public String getTlsCaPath() { return tlsCaPath; }
    public Map<String, String> getMetadata() { return metadata; }

    public String getChannelAddress() {
        return host + ":" + port;
    }
}

/**
 * Device type enumeration.
 */
public enum DeviceType {
    CORE_ROUTER("core-router"),
    MOBILE_CORE("mobile-core"),
    MOBILE_BACKHAUL("mobile-backhaul"),
    OLT_GPON("olt-gpon"),
    ENTERPRISE("enterprise-router"),
    RESIDENTIAL("residential-gateway");

    private final String name;

    DeviceType(String name) { this.name = name; }
    public String getName() { return name; }
}

/**
 * Base client for all MTS Router devices.
 */
public abstract class MtsRouterClient implements AutoCloseable {
    protected Channel channel;
    protected HealthGrpc.HealthBlockingStub healthStub;
    protected ConnectionConfig config;
    protected boolean isConnected;

    protected MtsRouterClient(ConnectionConfig config) throws Exception {
        this.config = config;
        this.isConnected = false;
        this.channel = createChannel();
        this.healthStub = HealthGrpc.newBlockingStub(channel);
        this.isConnected = true;
    }

    private Channel createChannel() throws Exception {
        if (config.isUseTls()) {
            SslContext sslContext = loadSslContext();
            return NettyChannelBuilder.forAddress(config.getHost(), config.getPort())
                    .sslContext(sslContext)
                    .maxInboundMessageSize(50 * 1024 * 1024)
                    .build();
        } else {
            return NettyChannelBuilder.forAddress(config.getHost(), config.getPort())
                    .usePlaintext()
                    .maxInboundMessageSize(50 * 1024 * 1024)
                    .build();
        }
    }

    private SslContext loadSslContext() throws Exception {
        KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());
        try (FileInputStream fis = new FileInputStream(config.getTlsCaPath())) {
            trustStore.load(fis, null);
        }
        TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        tmf.init(trustStore);

        SSLContext sslContext = SSLContext.getInstance("TLS");
        sslContext.init(null, tmf.getTrustManagers(), null);

        return GrpcSslContexts.forClient()
                .trustManager(tmf)
                .build();
    }

    public boolean healthCheck() throws Exception {
        HealthCheckResponse response = healthStub.check(
                HealthCheckRequest.newBuilder()
                        .setService(config.getDeviceType().getName())
                        .build());
        return response.getStatus() == HealthCheckResponse.ServingStatus.SERVING;
    }

    @Override
    public void close() {
        if (channel != null && !channel.isShutdown()) {
            channel.shutdownNow();
        }
        isConnected = false;
    }

    public boolean isConnected() { return isConnected; }
}

/**
 * Core Router Client (CR-9000).
 */
public class CoreRouterClient extends MtsRouterClient {
    public CoreRouterClient(String host, int port) throws Exception {
        this(host, port, 50051);
    }

    public CoreRouterClient(String host, int port, int defaultPort) throws Exception {
        super(new ConnectionConfig(host, port == 0 ? defaultPort : port, DeviceType.CORE_ROUTER));
    }

    public Map<String, Object> getBgpStatus() throws Exception {
        // stub.GetBgpStatus(ctx, request)
        Map<String, Object> result = new HashMap<>();
        result.put("peers", new ArrayList<>());
        return result;
    }

    public Map<String, Object> getLspStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        result.put("lsps", new ArrayList<>());
        return result;
    }

    public Map<String, Object> getPortStats(String portId) throws Exception {
        Map<String, Object> result = new HashMap<>();
        result.put("port_id", portId);
        return result;
    }

    public boolean configureLsp(String lspId, String nextHop) throws Exception {
        // stub.ConfigureLsp(ctx, request)
        return true;
    }

    public Map<String, Object> getP4TableStats() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public List<String> getSrv6Sid() throws Exception {
        return new ArrayList<>();
    }
}

/**
 * Mobile Core Client (MC-5000).
 */
public class MobileCoreClient extends MtsRouterClient {
    public MobileCoreClient(String host, int port) throws Exception {
        this(host, port, 50052);
    }

    public MobileCoreClient(String host, int port, int defaultPort) throws Exception {
        super(new ConnectionConfig(host, port == 0 ? defaultPort : port, DeviceType.MOBILE_CORE));
    }

    public Map<String, Object> getSessionStatus(String sessionId) throws Exception {
        Map<String, Object> result = new HashMap<>();
        result.put("session_id", sessionId);
        return result;
    }

    public String createSession(String imsi, String dnn) throws Exception {
        // stub.CreateSession(ctx, request)
        return "session-001";
    }

    public boolean deleteSession(String sessionId) throws Exception {
        // stub.DeleteSession(ctx, request)
        return true;
    }

    public Map<String, Object> getUpfStats() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public Map<String, Object> getPfcpStats() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public List<Map<String, Object>> getGtpTunnels() throws Exception {
        return new ArrayList<>();
    }
}

/**
 * OLT GPON Client (OLT-2000).
 */
public class OltGponClient extends MtsRouterClient {
    public OltGponClient(String host, int port) throws Exception {
        this(host, port, 50053);
    }

    public OltGponClient(String host, int port, int defaultPort) throws Exception {
        super(new ConnectionConfig(host, port == 0 ? defaultPort : port, DeviceType.OLT_GPON));
    }

    public Map<String, Object> getOnuStatus(String onuId) throws Exception {
        Map<String, Object> result = new HashMap<>();
        result.put("onu_id", onuId);
        return result;
    }

    public Map<String, Object> getGponPortStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public boolean configureOnu(String onuId, String profile) throws Exception {
        // stub.ConfigureOnu(ctx, request)
        return true;
    }

    public List<Map<String, Object>> getOmciEvents(String onuId, int count) throws Exception {
        return new ArrayList<>();
    }

    public Map<String, Object> getTr069Status() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }
}

/**
 * Enterprise Router Client (ER-1000).
 */
public class EnterpriseRouterClient extends MtsRouterClient {
    public EnterpriseRouterClient(String host, int port) throws Exception {
        this(host, port, 50054);
    }

    public EnterpriseRouterClient(String host, int port, int defaultPort) throws Exception {
        super(new ConnectionConfig(host, port == 0 ? defaultPort : port, DeviceType.ENTERPRISE));
    }

    public Map<String, Object> getSdwanStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public Map<String, Object> getMplsStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public List<Map<String, Object>> getIpsecTunnels() throws Exception {
        return new ArrayList<>();
    }

    public Map<String, Object> getVrrpStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public boolean configureSdwanPolicy(Map<String, Object> policy) throws Exception {
        // stub.ConfigureSdwanPolicy(ctx, request)
        return true;
    }
}

/**
 * Residential Gateway Client (RG-500).
 */
public class ResidentialGatewayClient extends MtsRouterClient {
    public ResidentialGatewayClient(String host, int port) throws Exception {
        this(host, port, 50055);
    }

    public ResidentialGatewayClient(String host, int port, int defaultPort) throws Exception {
        super(new ConnectionConfig(host, port == 0 ? defaultPort : port, DeviceType.RESIDENTIAL));
    }

    public Map<String, Object> getWifiStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public List<Map<String, Object>> getWifiClients() throws Exception {
        return new ArrayList<>();
    }

    public Map<String, Object> getVoipStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public Map<String, Object> getIptvStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public Map<String, Object> getOnuStatus() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public Map<String, Object> getTr069Status() throws Exception {
        Map<String, Object> result = new HashMap<>();
        return result;
    }

    public boolean configureWifi(String ssid, String password, String band) throws Exception {
        // stub.ConfigureWifi(ctx, request)
        return true;
    }
}

/**
 * Prometheus Metrics Client.
 */
public class MetricsClient {
    private final String host;
    private final int port;

    public MetricsClient(String host, int port) {
        this.host = host;
        this.port = port;
    }

    public String getMetrics() {
        try (java.util.Scanner scanner = new java.util.Scanner(
                new java.net.URL("http://" + host + ":" + port + "/metrics").openStream())) {
            StringBuilder sb = new StringBuilder();
            while (scanner.hasNextLine()) {
                sb.append(scanner.nextLine()).append("\n");
            }
            return sb.toString();
        } catch (Exception e) {
            return "# Error fetching metrics: " + e.getMessage();
        }
    }

    public Double getCounter(String name) {
        String metrics = getMetrics();
        for (String line : metrics.split("\n")) {
            if (line.startsWith(name + " ")) {
                String[] parts = line.split(" ");
                if (parts.length >= 2) {
                    try {
                        return Double.parseDouble(parts[1]);
                    } catch (NumberFormatException e) {
                        return null;
                    }
                }
            }
        }
        return null;
    }
}

/**
 * Example usage.
 */
public class MtsRouterExample {
    public static void main(String[] args) {
        // Example: Connect to Core Router
        try (CoreRouterClient client = new CoreRouterClient("192.168.1.1", 50051)) {
            if (client.isConnected()) {
                try {
                    if (client.healthCheck()) {
                        System.out.println("Core Router health: OK");
                    }
                } catch (Exception e) {
                    System.err.println("Health check failed: " + e.getMessage());
                }

                var bgpStatus = client.getBgpStatus();
                System.out.println("BGP Status: " + bgpStatus);
            }
        } catch (Exception e) {
            System.err.println("Core Router client error: " + e.getMessage());
        }

        // Example: Connect to Mobile Core
        try (MobileCoreClient client = new MobileCoreClient("192.168.1.2", 50052)) {
            if (client.isConnected()) {
                try {
                    String session = client.createSession("250012345678901", "ims.mts.ru");
                    System.out.println("Created session: " + session);
                } catch (Exception e) {
                    System.err.println("CreateSession error: " + e.getMessage());
                }
            }
        } catch (Exception e) {
            System.err.println("Mobile Core client error: " + e.getMessage());
        }

        // Example: Connect to OLT GPON
        try (OltGponClient client = new OltGponClient("192.168.1.3", 50053)) {
            if (client.isConnected()) {
                try {
                    var onuStatus = client.getOnuStatus("MTS-OLT-2000-ONU-001");
                    System.out.println("ONU Status: " + onuStatus);
                } catch (Exception e) {
                    System.err.println("GetOnuStatus error: " + e.getMessage());
                }
            }
        } catch (Exception e) {
            System.err.println("OLT GPON client error: " + e.getMessage());
        }

        // Example: Connect to Enterprise Router
        try (EnterpriseRouterClient client = new EnterpriseRouterClient("192.168.1.4", 50054)) {
            if (client.isConnected()) {
                try {
                    var sdwanStatus = client.getSdwanStatus();
                    System.out.println("SD-WAN Status: " + sdwanStatus);
                } catch (Exception e) {
                    System.err.println("GetSdwanStatus error: " + e.getMessage());
                }
            }
        } catch (Exception e) {
            System.err.println("Enterprise client error: " + e.getMessage());
        }

        // Example: Connect to Residential Gateway
        try (ResidentialGatewayClient client = new ResidentialGatewayClient("192.168.1.5", 50055)) {
            if (client.isConnected()) {
                try {
                    var wifiStatus = client.getWifiStatus();
                    System.out.println("WiFi Status: " + wifiStatus);
                } catch (Exception e) {
                    System.err.println("GetWifiStatus error: " + e.getMessage());
                }
            }
        } catch (Exception e) {
            System.err.println("Residential client error: " + e.getMessage());
        }

        // Example: Get metrics
        MetricsClient metricsClient = new MetricsClient("192.168.1.1", 9090);
        String metrics = metricsClient.getMetrics();
        System.out.println("Metrics:\n" + metrics);
    }
}
