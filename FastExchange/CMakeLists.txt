cmake_minimum_required(VERSION 3.20)
project(FastExchange VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FASTEXCHANGE_BUILD_TESTS "Build tests" ON)
option(FASTEXCHANGE_BUILD_API "Build REST API" ON)
option(FASTEXCHANGE_BUILD_BENCHMARKS "Build Google Benchmark targets" ON)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.2
)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest spdlog json CLI11)

# yaml-cpp 0.8.0 needs cmake_minimum_required bump for CMake 4.x
FetchContent_GetProperties(yaml-cpp)
if(NOT yaml-cpp_POPULATED)
    FetchContent_Populate(yaml-cpp)
    file(READ "${yaml-cpp_SOURCE_DIR}/CMakeLists.txt" YAML_CPP_CMAKE)
    string(REGEX REPLACE "cmake_minimum_required\\(VERSION [0-9.]+\\)"
           "cmake_minimum_required(VERSION 3.10)" YAML_CPP_CMAKE "${YAML_CPP_CMAKE}")
    file(WRITE "${yaml-cpp_SOURCE_DIR}/CMakeLists.txt" "${YAML_CPP_CMAKE}")
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()

if(FASTEXCHANGE_BUILD_BENCHMARKS)
    FetchContent_MakeAvailable(benchmark)
endif()

add_subdirectory(core)
add_subdirectory(modules)
add_subdirectory(cli)

if(FASTEXCHANGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(FASTEXCHANGE_BUILD_API)
    add_subdirectory(api)
endif()
