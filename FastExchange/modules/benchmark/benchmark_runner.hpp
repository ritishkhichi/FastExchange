#pragma once

#include "config/config_loader.hpp"
#include <string>

namespace fastexchange {

struct BenchmarkResult {
    uint64_t orders{0};
    uint64_t trades{0};
    uint64_t rejected{0};
    double throughput{0};
    double p50{0};
    double p95{0};
    double p99{0};
    double memory_mb{0};
    std::string sha256;
};

class BenchmarkRunner {
public:
    BenchmarkResult run(const std::string& scenario_path, int order_count,
                        const std::string& default_config = "config/default.yaml");
};

void compare_metrics(const std::string& path_a, const std::string& path_b);

}  // namespace fastexchange
