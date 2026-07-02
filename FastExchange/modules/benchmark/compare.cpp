#include "benchmark/benchmark_runner.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

namespace fastexchange {

void compare_metrics(const std::string& path_a, const std::string& path_b) {
    std::ifstream a(path_a), b(path_b);
    nlohmann::json ja, jb;
    a >> ja;
    b >> jb;

    auto get_throughput = [](const nlohmann::json& j) {
        return j["throughput"]["orders_per_sec"].get<double>();
    };
    auto get_p99 = [](const nlohmann::json& j) {
        if (j.contains("histograms") && j["histograms"].contains("latency.order.total")) {
            return j["histograms"]["latency.order.total"].value("p99", 0.0);
        }
        return 0.0;
    };

    std::cout << std::left << std::setw(20) << "Metric" << std::setw(20) << ja.value("scenario", "A")
              << std::setw(20) << jb.value("scenario", "B") << "\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::setw(20) << "Throughput" << std::setw(20) << std::fixed
              << std::setprecision(0) << get_throughput(ja) << std::setw(20) << get_throughput(jb)
              << "\n";
    std::cout << std::setw(20) << "P99 Latency (us)" << std::setw(20) << std::setprecision(2)
              << get_p99(ja) << std::setw(20) << get_p99(jb) << "\n";
    std::cout << std::setw(20) << "Trades"
              << std::setw(20) << ja["counters"].value("trades.total", 0ULL) << std::setw(20)
              << jb["counters"].value("trades.total", 0ULL) << "\n";
    std::cout << std::setw(20) << "Rejected"
              << std::setw(20) << ja["counters"].value("orders.rejected", 0ULL) << std::setw(20)
              << jb["counters"].value("orders.rejected", 0ULL) << "\n";
}

}  // namespace fastexchange
