cmake_minimum_required(VERSION 3.10)

project(NiveshSetu LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header files
include_directories(${PROJECT_SOURCE_DIR}/include)

# Core library: Risk Engine and Member 2 module
add_library(tradeguard_core
    src/RiskEngine.cpp
    src/Member2.cpp
)

# Main application
add_executable(tradeguard
    src/main.cpp
)

target_link_libraries(tradeguard
    PRIVATE tradeguard_core
)

# Automated test application
add_executable(tradeguard_tests
    risk_engine_tests.cpp
)

target_link_libraries(tradeguard_tests
    PRIVATE tradeguard_core
)

# Enable CTest
enable_testing()

add_test(
    NAME RiskEngineTests
    COMMAND tradeguard_tests
)