/**
 * MTS-MC-5000 Mobile Core — gRPC Server Entry Point
 * Starts the gRPC server with MtsMobileCoreService
 */

#include <iostream>
#include <string>
#include <csignal>
#include <chrono>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_service.h>

#include "service/mobile_core_service.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;

namespace mts::mc5000 {

// Global server pointer for graceful shutdown
static Server* g_server = nullptr;
static mts::mc5000::service::MobileCoreService* g_service = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    if (g_server) {
        g_server->Shutdown();
    }
    if (g_service) {
        g_service->stopHealthMonitor();
    }
}

} // namespace mts::mc5000

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50052");
    std::string config_file = "/etc/mts-mc5000.conf";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--address" && i + 1 < argc) {
            server_address = argv[++i];
        } else if (arg == "--config" && i + 1 < argc) {
            config_file = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: mts-mc5000-server [--address <addr>] [--config <path>]" << std::endl;
            return 0;
        }
    }
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, mts::mc5000::signalHandler);
    signal(SIGTERM, mts::mc5000::signalHandler);
    
    std::cout << "MTS-MC-5000 Mobile Core API Server" << std::endl;
    std::cout << "Starting on " << server_address << std::endl;
    std::cout << "Config: " << config_file << std::endl;
    
    // Create service instance
    mts::mc5000::service::MobileCoreService service;
    mts::mc5000::g_service = &service;
    
    // Start health monitoring
    service.startHealthMonitor();
    
    // Build and start gRPC server
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    mts::mc5000::g_server = server.get();
    
    std::cout << "Server listening on " << server_address << std::endl;
    std::cout << "Press Ctrl+C to shutdown" << std::endl;
    
    server->Wait();
    
    std::cout << "Server shut down" << std::endl;
    return 0;
}
