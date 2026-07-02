add_library(fastexchange_core
    event/event_codec.cpp
    eventlog/sha256.cpp
    eventlog/journal.cpp
    metrics/metrics.cpp
    metrics/system_metrics.cpp
    config/config_loader.cpp
    symbols/symbol_registry.cpp
    orderbook/order_book.cpp
    matching/price_time_fifo.cpp
    risk/default_risk.cpp
    engine/exchange_engine.cpp
)

target_include_directories(fastexchange_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(fastexchange_core
    PUBLIC
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
        spdlog::spdlog
)

if(WIN32)
    target_link_libraries(fastexchange_core PUBLIC psapi)
endif()

target_compile_features(fastexchange_core PUBLIC cxx_std_20)
