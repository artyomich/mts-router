cmake_minimum_required(VERSION 3.14)
project(mts-er1000-api VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/proto
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

set(PROTO_SRC proto/mts_enterprise.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})
grpc_cpp_plugin_location(${CMAKE_CURRENT_BINARY_DIR}/grpc_cpp_plugin)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC})

set(SOURCES
    src/hal/sdwan_hal.cpp
    src/hal/mpls_hal.cpp
    src/hal/ipsec_hal.cpp
    src/hal/vrrp_hal.cpp
    src/service/enterprise_service.cpp
    src/sdwan/path_manager.cpp
    src/mpls/lsp_manager.cpp
    src/ipsec/tunnel_manager.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

add_executable(mts-er1000-server ${SOURCES})

target_link_libraries(mts-er1000-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
    ${Boost_LIBRARIES}
)

install(TARGETS mts-er1000-server DESTINATION bin)
