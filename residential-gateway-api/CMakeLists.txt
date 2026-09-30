cmake_minimum_required(VERSION 3.16)
project(mts-rg500-api VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O2")

# Dependencies
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/hal)
include_directories(${CMAKE_SOURCE_DIR}/include/service)
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${gRPC_INCLUDE_DIR})

# Generated proto files
set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")
set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")

# Generate protobuf and gRPC code
protobuf_generate_cpp(GENERATED_SRCS GENERATED_HDRS ${PROTO_DIR}/mts_residential.proto)
grpc_generate_cpp(GENERATED_GRPC_SRCS GENERATED_GRPC_HDRS ${PROTO_DIR}/mts_residential.proto)

# Source files
set(SOURCES
    src/main.cpp
    src/service/rg_service.cpp
    hal/rg_hal.cpp
    hal/rg_wifi.cpp
    hal/rg_voip.cpp
    hal/rg_iptv.cpp
    hal/rg_tr069.cpp
    hal/rg_gpon.cpp
    ${GENERATED_SRCS}
    ${GENERATED_GRPC_SRCS}
)

# Headers
set(HEADERS
    include/hal/rg_hal.h
    include/hal/rg_wifi.h
    include/hal/rg_voip.h
    include/hal/rg_iptv.h
    include/hal/rg_tr069.h
    include/hal/rg_gpon.h
    include/service/rg_service.h
)

# Build executable
add_executable(mts-rg500-api ${SOURCES})

# Link libraries
target_link_libraries(mts-rg500-api
    ${PROTOBUF_LIBRARIES}
    ${gRPC_LIBRARIES}
    pthread
)

# Install
install(TARGETS mts-rg500-api DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include/mts-rg500)

# Tests
enable_testing()
add_subdirectory(tests)

# Configuration
configure_file(config/mts-rg500.conf.in config/mts-rg500.conf @ONLY)

message(STATUS "MTS Residential Gateway API configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Protobuf: ${PROTOBUF_VERSION}")
message(STATUS "  gRPC: ${gRPC_VERSION}")
