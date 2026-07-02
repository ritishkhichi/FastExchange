include(FetchContent)

# Crow requires standalone ASIO headers
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-28-0
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()
set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "ASIO include directory" FORCE)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.0
)
set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Crow)

add_library(fastexchange_api_lib server.cpp)
target_include_directories(fastexchange_api_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fastexchange_api_lib PUBLIC fastexchange_modules Crow::Crow)
target_compile_features(fastexchange_api_lib PUBLIC cxx_std_20)

if(WIN32)
    target_link_libraries(fastexchange_api_lib PUBLIC ws2_32 wsock32 mswsock)
endif()

add_executable(fastexchange_api api_main.cpp)
target_link_libraries(fastexchange_api PRIVATE fastexchange_api_lib CLI11::CLI11)
