/**
 * MTS-ER-1000 Enterprise Router — gRPC Server Entry Point
 */

#include <iostream>
#include <string>
#include <csignal>
#include <chrono>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_service.h>

#include "service/enterprise_service.h"

using grpc::Server;
using grpc::ServerBuilder;

namespace mts::er1000 {

static Server* g_server = nullptr;
static mts::er1000::service::EnterpriseService* g_service = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    if (g_server) g_server->Shutdown();
    if (g_service) g_service->stopHealthMonitor();
}

} // namespace mts::er1000

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50054");
    std::string config_file = "/etc/mts-er1000.conf";
    
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--address" && i + 1 < argc) server_address = argv[++i];
        else if (arg == "--config" && i + 1 < argc) config_file = argv[++i];
        else if (arg == "--help") {
            std::cout << "Usage: mts-er1000-server [--address <addr>] [--config <path>]" << std::endl;
            return 0;
        }
    }
    
    signal(SIGINT, mts::er1000::signalHandler);
    signal(SIGTERM, mts::er1000::signalHandler);
    
    std::cout << "MTS-ER-1000 Enterprise Router API Server" << std::endl;
    std::cout << "Starting on " << server_address << std::endl;
    
    mts::er1000::service::EnterpriseService service;
    mts::er1000::g_service = &service;
    service.startHealthMonitor();
    
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    mts::er1000::g_server = server.get();
    
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
    
    std::cout << "Server shut down" << std::endl;
    return 0;
}
