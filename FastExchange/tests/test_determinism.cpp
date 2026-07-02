#include "config/config_loader.hpp"
#include "scenario/scenario_runner.hpp"
#include <filesystem>
#include <gtest/gtest.h>

TEST(DeterminismTest, SameSeedSameHash) {
    if (!std::filesystem::exists("scenarios/balanced.yaml")) {
        GTEST_SKIP() << "scenarios/balanced.yaml not found";
    }

    std::filesystem::create_directories("test_output");

    fastexchange::ScenarioRunner runner;
    auto config = fastexchange::ConfigLoader::load_scenario("scenarios/balanced.yaml");
    config.order_limit = 500;
    config.duration_sec = 0;
    config.event_log_path = "test_output/det1.bin";
    config.metrics_path = "test_output/det1-metrics.json";

    auto r1 = runner.run(config);
    config.event_log_path = "test_output/det2.bin";
    config.metrics_path = "test_output/det2-metrics.json";
    auto r2 = runner.run(config);

    EXPECT_EQ(r1.sha256, r2.sha256);
    EXPECT_EQ(r1.trade_count, r2.trade_count);
}
