/*
 * main.cpp — Main entry point for MTS-MB-3000 gRPC API service
 *
 * MTS Mobile Backhaul — Main program
 */

#include <iostream>
#include <fstream>
#include <string>
#include <csignal>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "service/mbs_service.h"

/* Global service instance */
static mbs_service_t g_service;
static volatile int g_running = 1;

/* Signal handler */
static void signal_handler(int sig)
{
    (void)sig;
    g_running = 0;
}

/* Parse configuration file */
static int parse_config(const char *config_path, mbs_service_config_t *config)
{
    FILE *fp = fopen(config_path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open config file: %s\n", config_path);
        return -1;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;

        /* Skip comments and empty lines */
        if (line[0] == '#' || line[0] == ';' || line[0] == '\0')
            continue;

        /* Parse key=value */
        char *eq = strchr(line, '=');
        if (!eq)
            continue;
        *eq = '\0';
        char *key = line;
        char *value = eq + 1;

        /* Trim whitespace */
        while (*key == ' ' || *key == '\t') key++;
        char *kend = key + strlen(key) - 1;
        while (kend > key && (*kend == ' ' || *kend == '\t')) *kend-- = '\0';

        while (*value == ' ' || *value == '\t') value++;
        char *vend = value + strlen(value) - 1;
        while (vend > value && (*vend == ' ' || *vend == '\t')) *vend-- = '\0';

        if (strcmp(key, "port") == 0)
            config->port = atoi(value);
        else if (strcmp(key, "tls_enabled") == 0)
            config->tls_enabled = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        else if (strcmp(key, "tls_cert") == 0)
            strncpy(config->tls_cert_path, value, sizeof(config->tls_cert_path) - 1);
        else if (strcmp(key, "tls_key") == 0)
            strncpy(config->tls_key_path, value, sizeof(config->tls_key_path) - 1);
        else if (strcmp(key, "tls_ca") == 0)
            strncpy(config->tls_ca_path, value, sizeof(config->tls_ca_path) - 1);
        else if (strcmp(key, "level") == 0)
            strncpy(config->log_level, value, sizeof(config->log_level) - 1);
        else if (strcmp(key, "file") == 0)
            strncpy(config->log_file, value, sizeof(config->log_file) - 1);
        else if (strcmp(key, "stream_interval_ms") == 0)
            config->telemetry_interval_ms = atoi(value);
        else if (strcmp(key, "delta_encoding") == 0)
            config->delta_encoding = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
    }

    fclose(fp);
    return 0;
}

/* Print usage */
static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -p, --port PORT        gRPC server port (default: 50051)\n");
    fprintf(stderr, "  -c, --config FILE      Configuration file\n");
    fprintf(stderr, "  -t, --tls              Enable TLS\n");
    fprintf(stderr, "  -l, --log-level LEVEL  Log level (debug, info, warn, error)\n");
    fprintf(stderr, "  -h, --help             Show this help\n");
}

int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"port",    required_argument, 0, 'p'},
        {"config",  required_argument, 0, 'c'},
        {"tls",     no_argument,       0, 't'},
        {"log-level", required_argument, 0, 'l'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int config_file_index = -1;
    int port = 50051;
    bool tls_enabled = false;
    const char *log_level = "info";

    while ((opt = getopt_long(argc, argv, "p:c:tl:h", long_options, NULL)) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            if (port <= 0 || port > 65535) {
                fprintf(stderr, "Invalid port number: %s\n", optarg);
                return 1;
            }
            break;
        case 'c':
            config_file_index = optind - 1;
            break;
        case 't':
            tls_enabled = true;
            break;
        case 'l':
            log_level = optarg;
            break;
        case 'h':
            usage(argv[0]);
            return 0;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    /* Initialize config with defaults */
    memset(&g_service, 0, sizeof(g_service));
    g_service.config.port = port;
    g_service.config.tls_enabled = tls_enabled;
    strncpy(g_service.config.log_level, log_level, sizeof(g_service.config.log_level) - 1);
    g_service.config.telemetry_interval_ms = 1000;
    g_service.config.delta_encoding = true;

    /* Load configuration file if provided */
    if (config_file_index >= 0 && config_file_index < argc) {
        const char *config_path = argv[config_file_index];
        struct stat st;
        if (stat(config_path, &st) == 0) {
            if (parse_config(config_path, &g_service.config) != 0) {
                fprintf(stderr, "Warning: Failed to parse config file: %s\n", config_path);
            } else {
                std::cout << "Configuration loaded from: " << config_path << std::endl;
            }
        } else {
            fprintf(stderr, "Warning: Config file not found: %s\n", config_path);
        }
    }

    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "========================================" << std::endl;
    std::cout << "  MTS Mobile Backhaul API (MTS-MB-3000)" << std::endl;
    std::cout << "========================================" << std::endl;

    /* Initialize service */
    if (mbs_service_init(&g_service, &g_service.config) != 0) {
        std::cerr << "Failed to initialize service" << std::endl;
        return 1;
    }

    /* Start service */
    if (mbs_service_start(&g_service) != 0) {
        std::cerr << "Failed to start service" << std::endl;
        mbs_service_destroy(&g_service);
        return 1;
    }

    std::cout << "Service is running. Press Ctrl+C to stop." << std::endl;

    /* Main loop */
    while (g_running) {
        sleep(1);
    }

    std::cout << "Shutting down..." << std::endl;

    /* Stop service */
    mbs_service_stop(&g_service);

    /* Destroy service */
    mbs_service_destroy(&g_service);

    std::cout << "MTS-MB-3000 API stopped" << std::endl;
    return 0;
}
