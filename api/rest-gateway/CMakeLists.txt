cmake_minimum_required(VERSION 3.16)
project(mts-rest-gateway C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -O2")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Sources
file(GLOB SOURCS "src/*.c")

# Executable
add_executable(mts-rest-gateway ${SOURCES})

# Libraries
target_link_libraries(mts-rest-gateway
    pthread
    m
)

# Install
install(TARGETS mts-rest-gateway DESTINATION bin)

# Tests
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
