/**
 * MTS-MB-3000 Mobile Backhaul - gRPC Server
 * 
 * Главный файл приложения - запуск gRPC сервера
 * для управления Mobile Backhaul устройством.
 * 
 * Уровень реализации:
 * - gRPC server initialization
 * - Thread management
 * - Signal handling
 * - Graceful shutdown
 */

#include <iostream>
#include <string>
#include <csignal>
#include <chrono>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "service/backhaul_service.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::Status;

namespace mts {
namespace backhaul {

// Global server pointer for signal handling
Server* g_server = nullptr;

/**
 * Signal handler для graceful shutdown
 */
void signalHandler(int signum) {
    std::cout << "\n[Server] Received signal " << signum 
              << ", shutting down..." << std::endl;
    
    if (g_server) {
        g_server->Shutdown();
    }
    
    std::cout << "[Server] Shutdown complete" << std::endl;
    exit(signum);
}

/**
 * Запуск gRPC сервера
 */
class MtsBackhaulServer {
public:
    MtsBackhaulServer(const std::string& address)
        : address_(address) {
        std::cout << "[Server] Initializing MTS-MB-3000 gRPC server..." << std::endl;
        std::cout << "[Server] Address: " << address_ << std::endl;
    }
    
    ~MtsBackhaulServer() {
        std::cout << "[Server] Destroying..." << std::endl;
    }
    
    /**
     * Запуск сервера
     */
    void run() {
        std::cout << "[Server] Starting gRPC server..." << std::endl;
        
        // Создание service implementation
        mts::service::MtsBackhaulServiceImpl service;
        
        // Создание сервера
        ServerBuilder builder;
        builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        
        server_ = builder.BuildAndStart();
        
        if (server_) {
            std::cout << "[Server] gRPC server listening on " << address_ << std::endl;
            std::cout << "[Server] Available services:" << std::endl;
            std::cout << "  - GetPtpStatus" << std::endl;
            std::cout << "  - GetSyncEStatus" << std::endl;
            std::cout << "  - GetMplsTpPwStatus" << std::endl;
            std::cout << "  - GetPortStatus" << std::endl;
            std::cout << "  - GetDeviceHealth" << std::endl;
            std::cout << "  - SetGrandmaster" << std::endl;
            std::cout << "  - CreatePw" << std::endl;
            std::cout << "  - DeletePw" << std::endl;
            std::cout << "  - SubscribeTelemetry" << std::endl;
        } else {
            std::cerr << "[Server] Failed to start gRPC server" << std::endl;
        }
        
        // Ожидание shutdown
        server_->Wait();
        
        std::cout << "[Server] gRPC server stopped" << std::endl;
    }
    
    /**
     * Остановка сервера
     */
    void stop() {
        if (server_) {
            server_->Shutdown();
        }
    }

private:
    std::string address_;
    std::unique_ptr<Server> server_;
};

} // namespace backhaul
} // namespace mts

/**
 * Main function
 */
int main(int argc, char* argv[]) {
    std::string address = "0.0.0.0:50051";
    
    // Парсинг командной строки
    if (argc > 1) {
        address = argv[1];
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "  MTS-MB-3000 Mobile Backhaul API" << std::endl;
    std::cout << "  gRPC Server v1.0.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Установка signal handlers
    signal(SIGINT, mts::backhaul::signalHandler);
    signal(SIGTERM, mts::backhaul::signalHandler);
    
    // Создание и запуск сервера
    mts::backhaul::MtsBackhaulServer server(address);
    server.run();
    
    return 0;
}
