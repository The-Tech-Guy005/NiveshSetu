cmake_minimum_required(VERSION 3.16)
-> This tells CMake that the project requires version 3.16 or later.cmake_minimum_required is a built-in CMake command.VERSION 3.16 sets the minimum supported version.It is necessary so that project should not silently depend on an older CMake version that lacks a feature it needs.

project(TradeGuard360 VERSION 1.0 LANGUAGES CXX)
->  This declares the project. (project) initializes the project configuration.(TradeGuard360) is the project name.(VERSION 1.0) identifies your project's version.(LANGUAGES CXX) tells CMake that project is using C++.
-> (CXX) is CMake's language identifier for C++, not the filename extension.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
-> (set) assigns a value to a CMake variable.Here, the project requests C++17.
-> (CMAKE_CXX_STANDARD_REQUIRED ON) means CMake must use the requested standard instead of sliently choosing an older one.
-> (CMAKE_CXX_EXTENSIONS OFF) requests standard C++17 rather than  compiler-specific  extensions where suppported.

enable_testing()
-> This enable CMake's testing infrastructure.It allows you to register test commands and run them through CTest, which is the test runner distributed with CMake.

add_library(
    tradeguard_core
    src/risk_engine.cpp 
    src/data_store.cpp 
)
-> This creates a library target called tradeguard_core.

target_include_directories(tradeguard_core
     PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
-> This tells CMake where the library's header files are located.
->(target_include_directories) configures header search paths for a  particular target.
->(tradeguard_core) is the target  being configured.
->(PUBLIC) means the include directory applies to the library itself and is also passed to targets that link against it.
->(${CMAKE_CURRENT_SOURCE_DIR}) is a CMake variable representing the directory containing CMakeLists.txt.
->(/include) is appended to that directory

add_executable(tradeguard src/main.cpp)
-> (add_executable) tells CMake to produce a runnable program.
-> (tradeguard) is the target name.
-> (src/main.cpp) contains the program's main() function,which is where execution starts.

target_link_libraries(tradeguard PRIVATE tradeguard_core)
-> (target_link_libraries) declares a dependency between targets.
-> (tradeguard) is the executable.
-> (PRIVATE) means the dependency is used by this target and is not propagated as a public requirement to its consumers.
-> tradeguard_core provides the risk and dat-handling implementations.
