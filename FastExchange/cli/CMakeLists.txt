add_executable(fastexchange main.cpp)
target_link_libraries(fastexchange PRIVATE fastexchange_modules CLI11::CLI11 spdlog::spdlog)
target_compile_features(fastexchange PRIVATE cxx_std_20)
