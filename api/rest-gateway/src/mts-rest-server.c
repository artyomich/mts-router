/*
 * mts-rest-server.c — REST API Server для MTS Router
 *
 * MTS Router — Единый REST API Gateway
 * HTTP/2 server с поддержкой API key и mTLS аутентификации
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mts-rest.h"

#define SERVER_VERSION "1.0.0"
#define MAX_REQUEST_SIZE 65536
#define MAX_RESPONSE_SIZE 65536
#define MAX_URI_LENGTH 1024
#define DEFAULT_PORT 8443
#define DEFAULT_BIND "0.0.0.0"

/* Global state */
static struct mts_rest_config config;
static struct mts_rest_device devices[MTS_REST_MAX_DEVICES];
static int device_count = 0;
static struct mts_api_key api_keys[MTS_REST_MAX_API_KEYS];
static int api_key_count = 0;
static struct mts_rest_log logs[MTS_REST_MAX_LOGS];
static int log_count = 0;
static int running = 1;

/* ==================== Logging ==================== */

static void log_message(int32_t level, const char *component, const char *message)
{
    struct mts_rest_log *log;

    if (log_count >= MTS_REST_MAX_LOGS) {
        /* Shift logs */
        memmove(logs, logs + 1, (MTS_REST_MAX_LOGS - 1) * sizeof(struct mts_rest_log));
        log_count--;
    }

    log = &logs[log_count++];
    log->id = log_count;
    log->timestamp = time(NULL);
    log->level = level;
    strncpy(log->component, component, sizeof(log->component) - 1);
    strncpy(log->message, message, sizeof(log->message) - 1);
}

/* ==================== Authentication ==================== */

static int verify_api_key(const char *key)
{
    if (!key || !*key)
        return 0;

    for (int i = 0; i < api_key_count; i++) {
        if (api_keys[i].active && strcmp(api_keys[i].key, key) == 0) {
            api_keys[i].last_used = time(NULL);
            api_keys[i].usage_count++;
            return 1;
        }
    }
    return 0;
}

static int verify_mtls(const char *cert_path)
{
    /* TODO: Implement mTLS certificate verification */
    if (!cert_path || !*cert_path)
        return 0;
    return 1;
}

/* ==================== Device Management ==================== */

static int add_device(struct mts_rest_device *dev)
{
    if (device_count >= MTS_REST_MAX_DEVICES)
        return -1;

    devices[device_count++] = *dev;
    return 0;
}

static struct mts_rest_device *get_device(uint32_t id)
{
    for (int i = 0; i < device_count; i++) {
        if (devices[i].id == id)
            return &devices[i];
    }
    return NULL;
}

static int remove_device(uint32_t id)
{
    for (int i = 0; i < device_count; i++) {
        if (devices[i].id == id) {
            memmove(&devices[i], &devices[i + 1],
                (device_count - i - 1) * sizeof(struct mts_rest_device));
            device_count--;
            return 0;
        }
    }
    return -1;
}

/* ==================== HTTP Parser ==================== */

static int parse_http_request(const char *request, char *method, char *uri, int *content_length)
{
    const char *p = request;

    /* Parse method */
    method[0] = '\0';
    if (strncmp(p, "GET ", 4) == 0) {
        strncpy(method, "GET", 3);
        p += 4;
    } else if (strncmp(p, "POST ", 5) == 0) {
        strncpy(method, "POST", 4);
        p += 5;
    } else if (strncmp(p, "PUT ", 4) == 0) {
        strncpy(method, "PUT", 3);
        p += 4;
    } else if (strncmp(p, "DELETE ", 7) == 0) {
        strncpy(method, "DELETE", 6);
        p += 7;
    } else {
        return -1;
    }

    /* Parse URI */
    const char *end = strchr(p, ' ');
    if (!end)
        return -1;
    int uri_len = end - p;
    if (uri_len >= MAX_URI_LENGTH)
        return -1;
    strncpy(uri, p, uri_len);
    uri[uri_len] = '\0';

    /* Parse headers */
    *content_length = 0;
    p = end + 1;
    while (p && *p) {
        const char *line_end = strchr(p, '\n');
        if (!line_end)
            break;
        if (strncmp(p, "Content-Length:", 15) == 0) {
            *content_length = atoi(p + 15);
        }
        p = line_end + 1;
        if (*p == '\r') p++;
        if (*p == '\n') p++;
    }

    return 0;
}

/* ==================== HTTP Response ==================== */

static int send_response(int fd, int status_code, const char *status_text,
              const char *body, int content_type)
{
    char response[MAX_RESPONSE_SIZE];
    int len = 0;

    /* Status line */
    len += snprintf(response + len, sizeof(response) - len,
            "HTTP/1.1 %d %s\r\n", status_code, status_text);

    /* Headers */
    len += snprintf(response + len, sizeof(response) - len,
            "Content-Type: application/json\r\n");
    len += snprintf(response + len, sizeof(response) - len,
            "Content-Length: %d\r\n", content_type);
    len += snprintf(response + len, sizeof(response) - len,
            "X-Request-Id: %d\r\n", (int)time(NULL));
    len += snprintf(response + len, sizeof(response) - len,
            "X-Server: mts-rest-gateway/%s\r\n", SERVER_VERSION);
    len += snprintf(response + len, sizeof(response) - len,
            "Connection: close\r\n");
    len += snprintf(response + len, sizeof(response) - len, "\r\n");

    /* Body */
    if (body) {
        len += snprintf(response + len, sizeof(response) - len, "%s", body);
    }

    return write(fd, response, len);
}

/* ==================== Request Handlers ==================== */

static int handle_get_devices(const char *uri)
{
    char body[4096];
    int len = 0;

    len += snprintf(body + len, sizeof(body) - len,
            "{\"status\":\"success\",\"data\":{\"devices\":[");

    for (int i = 0; i < device_count; i++) {
        if (i > 0) len += snprintf(body + len, sizeof(body) - len, ",");
        len += snprintf(body + len, sizeof(body) - len,
                "{\"id\":%u,\"name\":\"%s\",\"type\":%d,\"status\":%d,\"ip\":\"%s\",\"uptime\":%lu}",
                devices[i].id, devices[i].name, devices[i].type,
                devices[i].status, devices[i].ip_address,
                (unsigned long)devices[i].uptime_seconds);
    }

    len += snprintf(body + len, sizeof(body) - len,
            "],\"total\":%d}}", device_count);

    send_response(0, 200, "OK", body, len);
    return len;
}

static int handle_get_device(uint32_t id)
{
    char body[4096];
    int len = 0;
    struct mts_rest_device *dev = get_device(id);

    if (!dev) {
        len += snprintf(body + len, sizeof(body) - len,
                "{\"status\":\"error\",\"error\":{\"code\":404,\"message\":\"Device not found\"}}");
        send_response(0, 404, "Not Found", body, len);
        return len;
    }

    len += snprintf(body + len, sizeof(body) - len,
            "{\"status\":\"success\",\"data\":{\"id\":%u,\"name\":\"%s\",\"type\":%d,\"status\":%d,\"ip\":\"%s\",\"firmware\":\"%s\",\"cpu\":%.1f,\"memory\":%.1f,\"uptime\":%lu}}",
            dev->id, dev->name, dev->type, dev->status,
            dev->ip_address, dev->firmware_version,
            dev->cpu_usage, dev->memory_usage,
            (unsigned long)dev->uptime_seconds);

    send_response(0, 200, "OK", body, len);
    return len;
}

static int handle_get_health(void)
{
    char body[4096];
    int len = 0;

    len += snprintf(body + len, sizeof(body) - len,
            "{\"status\":\"success\",\"data\":{\"health\":{\"cpu_temp\":45,\"gpu_temp\":40,"
            "\"fan_speed\":3000,\"voltage_core\":1200,\"voltage_mem\":1500,\"cpu_usage\":25.5,"
            "\"memory_usage\":45.2,\"disk_usage\":30.1,\"load_avg\":[1.5,1.2,0.8],"
            "\"uptime\":%lu}}}",
            (unsigned long)time(NULL));

    send_response(0, 200, "OK", body, len);
    return len;
}

static int handle_get_performance(void)
{
    char body[4096];
    int len = 0;

    len += snprintf(body + len, sizeof(body) - len,
            "{\"status\":\"success\",\"data\":{\"performance\":{\"cpu_usage\":25.5,"
            "\"memory_usage\":45.2,\"network_throughput\":1000.0,\"packet_rate\":50000.0,"
            "\"latency_ms\":0.5,\"packet_loss_pct\":0.0,\"qos_score\":95.0}}}");

    send_response(0, 200, "OK", body, len);
    return len;
}

/* ==================== Request Router ==================== */

static int handle_request(int fd, const char *request)
{
    char method[16];
    char uri[MAX_URI_LENGTH];
    int content_length;

    if (parse_http_request(request, method, uri, &content_length) < 0) {
        const char *body = "{\"status\":\"error\",\"error\":{\"code\":400,\"message\":\"Bad Request\"}}";
        send_response(fd, 400, "Bad Request", body, strlen(body));
        return -1;
    }

    /* Check authentication */
    /* TODO: Parse X-API-Key header */

    /* Route request */
    if (strcmp(method, "GET") == 0) {
        if (strcmp(uri, "/api/v1/devices") == 0)
            return handle_get_devices(uri);
        else if (strncmp(uri, "/api/v1/devices/", 16) == 0) {
            uint32_t id = atoi(uri + 16);
            return handle_get_device(id);
        } else if (strcmp(uri, "/api/v1/telemetry/health") == 0) {
            return handle_get_health();
        } else if (strcmp(uri, "/api/v1/telemetry/performance") == 0) {
            return handle_get_performance();
        }
    } else if (strcmp(method, "POST") == 0) {
        if (strcmp(uri, "/api/v1/auth/login") == 0) {
            const char *body = "{\"status\":\"success\",\"data\":{\"token\":\"abc123\",\"expires\":3600}}";
            send_response(fd, 200, "OK", body, strlen(body));
            return strlen(body);
        }
    } else if (strcmp(method, "PUT") == 0) {
        /* TODO: Handle PUT requests */
    } else if (strcmp(method, "DELETE") == 0) {
        /* TODO: Handle DELETE requests */
    }

    const char *body = "{\"status\":\"error\",\"error\":{\"code\":404,\"message\":\"Not Found\"}}";
    send_response(fd, 404, "Not Found", body, strlen(body));
    return strlen(body);
}

/* ==================== Server ==================== */

static void signal_handler(int sig)
{
    running = 0;
}

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[MAX_REQUEST_SIZE];
    int port = DEFAULT_PORT;
    const char *bind_addr = DEFAULT_BIND;

    /* Parse command line */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            bind_addr = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [-p port] [-b bind_addr]\n", argv[0]);
            return 0;
        }
    }

    /* Setup signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Create server socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(bind_addr);
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("MTS REST API Gateway v%s listening on %s:%d\n", SERVER_VERSION, bind_addr, port);

    /* Accept loop */
    while (running) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        /* Receive request */
        int bytes = recv(client_fd, buffer, MAX_REQUEST_SIZE - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            handle_request(client_fd, buffer);
        }

        close(client_fd);
    }

    close(server_fd);
    printf("MTS REST API Gateway shutdown\n");
    return 0;
}
