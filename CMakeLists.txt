cmake_minimum_required(VERSION 3.16)

project(TradeGuard360 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(
    tradeguard_core
    src/risk_engine.cpp 
    src/data_store.cpp 
)

target_include_directories(tradeguard_core
     PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(tradeguard src/main.cpp)
target_link_libraries(tradeguard PRIVATE tradeguard_core)



