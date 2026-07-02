#include "engine/exchange_engine.hpp"
#include "eventlog/journal.hpp"
#include "metrics/metrics.hpp"
#include "symbols/symbol_registry.hpp"
#include <filesystem>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

TEST(EngineTest, Integration10k) {
    std::filesystem::create_directories("test_output");

    fastexchange::SymbolRegistry registry;
    registry.register_symbol("ABC", fastexchange::to_ticks(105.0, 0.0001));

    fastexchange::MetricsEngine metrics;
    fastexchange::EventJournal journal;
    journal.open("test_output/engine_test.bin", 42, 0);

    fastexchange::ExchangeEngine engine(registry, journal, metrics);
    fastexchange::EngineConfig config;
    config.max_order_size = 10000;
    engine.configure(config);

    engine.start_scenario("integration", 42, 0);

    for (int i = 0; i < 10000; ++i) {
        fastexchange::Order o;
        o.id = static_cast<fastexchange::OrderId>(i + 1);
        o.symbol = "ABC";
        o.side = (i % 2 == 0) ? fastexchange::Side::Buy : fastexchange::Side::Sell;
        o.price = fastexchange::to_ticks(105.0, 0.0001) + (i % 5);
        o.quantity = 10;
        engine.process(fastexchange::OrderSubmit{o}, static_cast<fastexchange::Timestamp>(i * 1000));
    }

    engine.end_scenario("integration", 42, 10000000);
    journal.close();

    EXPECT_GT(metrics.snapshot()["counters"]["orders.submitted"], 0);
    EXPECT_TRUE(fastexchange::EventJournal::verify_file("test_output/engine_test.bin"));
}
