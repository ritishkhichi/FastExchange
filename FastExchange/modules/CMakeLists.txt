add_library(fastexchange_modules
    workload/workload_factory.cpp
    workload/base_strategy.cpp
    workload/builtin_strategies.cpp
    workload/register_strategies.cpp
    workload/replay_strategy.cpp
    scenario/scenario_runner.cpp
    benchmark/benchmark_runner.cpp
    benchmark/compare.cpp
    replay/replay_engine.cpp
    profiler/stage_profiler.cpp
)

target_include_directories(fastexchange_modules
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fastexchange_modules
    PUBLIC
        fastexchange_core
)

target_compile_features(fastexchange_modules PUBLIC cxx_std_20)
