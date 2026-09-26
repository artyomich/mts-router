cmake_minimum_required(VERSION 3.14)
project(mobile_core_api-tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/../proto
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
)

# HAL tests
add_executable(test_hal
    test_fabric_hal.cpp
    test_line_card_hal.cpp
    test_port_hal.cpp
)
target_link_libraries(test_hal
    GTest::gtest_main
    GTest::gtest
    pthread
)

# Service tests
add_executable(test_service
    test_core_router_service.cpp
)
target_link_libraries(test_service
    GTest::gtest_main
    GTest::gtest
    pthread
)

# P4Runtime tests
add_executable(test_p4runtime
    test_p4_manager.cpp
)
target_link_libraries(test_p4runtime
    GTest::gtest_main
    GTest::gtest
    pthread
)

# MPLS tests
add_executable(test_mpls
    test_lsp_manager.cpp
)
target_link_libraries(test_mpls
    GTest::gtest_main
    GTest::gtest
    pthread
)

# SRv6 tests
add_executable(test_srv6
    test_srv6_manager.cpp
)
target_link_libraries(test_srv6
    GTest::gtest_main
    GTest::gtest
    pthread
)

# BGP tests
add_executable(test_bgp
    test_bgp_monitor.cpp
)
target_link_libraries(test_bgp
    GTest::gtest_main
    GTest::gtest
    pthread
)

# Enable test discovery
include(GoogleTest)
gtest_discover_tests(test_hal)
gtest_discover_tests(test_service)
gtest_discover_tests(test_p4runtime)
gtest_discover_tests(test_mpls)
gtest_discover_tests(test_srv6)
gtest_discover_tests(test_bgp)
