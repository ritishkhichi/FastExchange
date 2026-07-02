#include "benchmark/benchmark_runner.hpp"
#include "scenario/scenario_runner.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

namespace fastexchange {

BenchmarkResult BenchmarkRunner::run(const std::string& scenario_path, int order_count,
                                     const std::string& default_config) {
    auto config = ConfigLoader::load_scenario(scenario_path, default_config);
    config.order_limit = order_count;
    config.duration_sec = 0;
    config.event_log_path = "results/benchmark-" + config.name + ".bin";
    config.metrics_path = "results/benchmark-" + config.name + "-metrics.json";

    ScenarioRunner runner;
    auto result = runner.run(config);

    std::ifstream in(result.metrics_path);
    nlohmann::json j;
    in >> j;

    BenchmarkResult br;
    br.orders = order_count;
    br.trades = result.trade_count;
    br.throughput = j["throughput"]["orders_per_sec"].get<double>();
    br.sha256 = result.sha256;

    if (j.contains("histograms") && j["histograms"].contains("latency.order.total")) {
        const auto& h = j["histograms"]["latency.order.total"];
        br.p50 = h.value("p50", 0.0);
        br.p95 = h.value("p95", 0.0);
        br.p99 = h.value("p99", 0.0);
    }
    if (j.contains("counters")) {
        br.rejected = j["counters"].value("orders.rejected", 0ULL);
    }

    std::cout << "========== FastExchange Benchmark ==========\n";
    std::cout << "Scenario:      " << config.name << "\n";
    std::cout << "Orders:        " << br.orders << "\n";
    std::cout << "Trades:        " << br.trades << "\n";
    std::cout << "Rejected:      " << br.rejected << "\n";
    std::cout << "Throughput:    " << std::fixed << std::setprecision(0) << br.throughput
              << " orders/sec\n";
    std::cout << "Latency P50:   " << std::setprecision(2) << br.p50 << " us\n";
    std::cout << "Latency P95:   " << br.p95 << " us\n";
    std::cout << "Latency P99:   " << br.p99 << " us\n";
    std::cout << "SHA256:        " << br.sha256 << "\n";
    std::cout << "============================================\n";

    return br;
}

}  // namespace fastexchange
