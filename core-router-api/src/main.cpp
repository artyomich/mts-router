/**
 * MTS-CR-9000 Core Router — gRPC Server Entry Point
 * Starts the gRPC server with MtsCoreRouterService
 */

#include <iostream>
#include <string>
#include <csignal>
#include <chrono>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_service.h>

#include "service/core_router_service.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;

namespace mts::cr9000 {

// Global server pointer for graceful shutdown
static Server* g_server = nullptr;
static mts::cr9000::service::CoreRouterService* g_service = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    if (g_server) {
        g_server->Shutdown();
    }
    if (g_service) {
        g_service->stopHealthMonitor();
    }
}

} // namespace mts::cr9000

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50051");
    std::string config_file = "/etc/mts-cr9000.conf";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--address" && i + 1 < argc) {
            server_address = argv[++i];
        } else if (arg == "--config" && i + 1 < argc) {
            config_file = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: mts-cr9000-server [--address <addr>] [--config <path>]" << std::endl;
            return 0;
        }
    }
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, mts::cr9000::signalHandler);
    signal(SIGTERM, mts::cr9000::signalHandler);
    
    std::cout << "MTS-CR-9000 Core Router API Server" << std::endl;
    std::cout << "Starting on " << server_address << std::endl;
    std::cout << "Config: " << config_file << std::endl;
    
    // Create service instance
    mts::cr9000::service::CoreRouterService service;
    mts::cr9000::g_service = &service;
    
    // Start health monitoring
    service.startHealthMonitor();
    
    // Build and start gRPC server
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    mts::cr9000::g_server = server.get();
    
    std::cout << "Server listening on " << server_address << std::endl;
    std::cout << "Press Ctrl+C to shutdown" << std::endl;
    
    server->Wait();
    
    std::cout << "Server shut down" << std::endl;
    return 0;
}
