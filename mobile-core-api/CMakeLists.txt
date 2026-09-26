cmake_minimum_required(VERSION 3.14)
project(mts-mc5000-api VERSION 1.0.0 LANGUAGES CXX)

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

set(PROTO_SRC proto/mts_mobile_core.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})
grpc_cpp_plugin_location(${CMAKE_CURRENT_BINARY_DIR}/grpc_cpp_plugin)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC})

set(SOURCES
    src/hal/upf_hal.cpp
    src/hal/sm_hal.cpp
    src/hal/pfcp_hal.cpp
    src/hal/gtp_hal.cpp
    src/service/mobile_core_service.cpp
    src/upf/session_manager.cpp
    src/smf/session_controller.cpp
    src/pfcp/pfcp_handler.cpp
    src/gtp/gtp_tunnel.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

add_executable(mts-mc5000-server ${SOURCES})

target_link_libraries(mts-mc5000-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
    ${Boost_LIBRARIES}
)

install(TARGETS mts-mc5000-server DESTINATION bin)
