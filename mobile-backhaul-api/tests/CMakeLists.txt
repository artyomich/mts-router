# Mobile Backhaul API Tests

# Service tests
add_executable(test_mbs_service
    test_mbs_service.cpp
)

target_link_libraries(test_mbs_service
    mts-mb3000-api
    ${PROTOBUF_LIBRARIES}
    ${gRPC_LIBRARIES}
)

add_test(NAME test_mbs_service COMMAND test_mbs_service)

# HAL tests
add_executable(test_mbs_hal
    test_mbs_hal.cpp
)

target_link_libraries(test_mbs_hal
    ${PROTOBUF_LIBRARIES}
    ${gRPC_LIBRARIES}
)

add_test(NAME test_mbs_hal COMMAND test_mbs_hal)
