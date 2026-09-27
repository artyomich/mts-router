cmake_minimum_required(VERSION 3.14)
project(enterprise_router_api-tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/../proto
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# HAL tests
add_executable(test_hal
    test_sdwan_hal.cpp
    test_mpls_hal.cpp
    test_ipsec_hal.cpp
    test_vrrp_hal.cpp
)
target_link_libraries(test_hal
    GTest::gtest_main
    GTest::gtest
    pthread
    ${Boost_LIBRARIES}
)

# Enable test discovery
include(GoogleTest)
gtest_discover_tests(test_hal)
