add_executable(fastexchange_tests
    test_journal.cpp
    test_metrics.cpp
    test_orderbook.cpp
    test_matching.cpp
    test_engine.cpp
    test_determinism.cpp
)

target_link_libraries(fastexchange_tests
    PRIVATE
        fastexchange_modules
        GTest::gtest_main
        nlohmann_json::nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(fastexchange_tests
    PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
