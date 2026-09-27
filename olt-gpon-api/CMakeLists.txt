cmake_minimum_required(VERSION 3.14)
project(mts-olt2000-api VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(gRPC REQUIRED)

include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

set(PROTO_SRC proto/mts_olt_gpon.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin PATHS /usr/bin /usr/local/bin)
if(NOT GRPC_CPP_PLUGIN)
  message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_olt_gpon.grpc.pb.cc
         ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_olt_gpon.grpc.pb.h
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
  ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
       --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
       --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
       mts_olt_gpon.proto
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_SRC}
  COMMENT "Generating gRPC C++ code"
)

set(GRPC_SRCS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_olt_gpon.grpc.pb.cc)
set(GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_olt_gpon.grpc.pb.h)

set(SOURCES
    src/hal/gpon_hal.cpp
    src/hal/onu_hal.cpp
    src/hal/omci_hal.cpp
    src/hal/tr069_hal.cpp
    src/hal/gpon_hal_cpp.cpp
    src/hal/onu_hal_cpp.cpp
    src/hal/omci_hal_cpp.cpp
    src/hal/tr069_hal_cpp.cpp
    src/service/olt_gpon_service.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

add_executable(mts-olt2000-server ${SOURCES})

target_link_libraries(mts-olt2000-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
    ${Boost_LIBRARIES}
)

install(TARGETS mts-olt2000-server DESTINATION bin)
