#pragma once

#include "metrics/metrics.hpp"
#include "symbols/symbol_registry.hpp"
#include "types/trade.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace fastexchange {

struct ApiState {
    std::mutex mutex;
    MetricsEngine metrics;
    nlohmann::json metrics_cache;
    bool has_metrics_cache{false};
    std::unique_ptr<SymbolRegistry> registry;
    std::vector<Trade> recent_trades;
    std::string scenario_name;
    std::atomic<bool> running{false};

    void load_metrics_file(const std::string& path);
    void register_symbols(const std::vector<std::string>& symbols);
    void sync_live_snapshot(const nlohmann::json& snapshot, const std::vector<Trade>& trades);
};

class ApiServer {
public:
    explicit ApiServer(ApiState& state);
    void run(const std::string& host, int port);

private:
    ApiState& state_;
};

}  // namespace fastexchange
