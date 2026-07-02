#include "metrics/metrics.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

namespace fastexchange {

void print_stage_profiler(const MetricsEngine& metrics) {
    auto snap = metrics.snapshot();
    std::cout << "=== Stage Profiler ===\n";
    if (!snap.contains("histograms")) return;
    for (const auto& [name, data] : snap["histograms"].items()) {
        if (name.find("latency.stage") != std::string::npos) {
            std::cout << name << ": p50=" << data.value("p50", 0.0)
                      << "us p99=" << data.value("p99", 0.0) << "us\n";
        }
    }
}

}  // namespace fastexchange
