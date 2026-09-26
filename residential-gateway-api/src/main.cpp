/**
 * MTS-RG-500 Residential Gateway — gRPC Server Entry Point
 */

#include <iostream>
#include <string>
#include <csignal>
#include <chrono>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_service.h>

#include "service/residential_service.h"

using grpc::Server;
using grpc::ServerBuilder;

namespace mts::rg500 {

static Server* g_server = nullptr;
static mts::rg500::service::ResidentialService* g_service = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    if (g_server) g_server->Shutdown();
    if (g_service) g_service->stopHealthMonitor();
}

} // namespace mts::rg500

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50055");
    std::string config_file = "/etc/mts-rg500.conf";
    
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--address" && i + 1 < argc) server_address = argv[++i];
        else if (arg == "--config" && i + 1 < argc) config_file = argv[++i];
        else if (arg == "--help") {
            std::cout << "Usage: mts-rg500-server [--address <addr>] [--config <path>]" << std::endl;
            return 0;
        }
    }
    
    signal(SIGINT, mts::rg500::signalHandler);
    signal(SIGTERM, mts::rg500::signalHandler);
    
    std::cout << "MTS-RG-500 Residential Gateway API Server" << std::endl;
    std::cout << "Starting on " << server_address << std::endl;
    
    mts::rg500::service::ResidentialService service;
    mts::rg500::g_service = &service;
    service.startHealthMonitor();
    
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    mts::rg500::g_server = server.get();
    
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
    
    std::cout << "Server shut down" << std::endl;
    return 0;
}
