#pragma once

#include "config/config_loader.hpp"
#include <functional>
#include <string>

namespace fastexchange {

class ExchangeEngine;
class MetricsEngine;
class SymbolRegistry;

struct ScenarioHooks {
    SymbolRegistry* registry{nullptr};
    MetricsEngine* metrics{nullptr};
    std::function<void(ExchangeEngine& engine, int orders_processed, double elapsed_sec)> on_progress;
    int progress_interval{500};
};

struct ScenarioResult {
    std::string event_log_path;
    std::string metrics_path;
    std::string sha256;
    uint64_t event_count{0};
    uint64_t trade_count{0};
    double duration_sec{0};
};

class ScenarioRunner {
public:
    ScenarioResult run(const ScenarioConfig& config, ScenarioHooks* hooks = nullptr);
    ScenarioResult run_file(const std::string& scenario_path,
                            const std::string& default_config = "config/default.yaml",
                            ScenarioHooks* hooks = nullptr);
};

}  // namespace fastexchange
