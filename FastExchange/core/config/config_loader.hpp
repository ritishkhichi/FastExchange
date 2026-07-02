#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace fastexchange {

struct EngineConfig {
    uint64_t max_order_size{100000};
    bool auto_register_symbols{false};
    std::string risk_plugin{"default"};
    std::string matching_algorithm{"price_time_fifo"};
};

struct WorkloadConfig {
    std::string profile{"balanced"};
    int arrival_rate{50000};
    double buy_probability{0.5};
    double sell_probability{0.5};
    std::string price_distribution{"normal"};
    int price_bias_ticks{0};
    std::string quantity_distribution{"poisson"};
    double quantity_lambda{50};
    int quantity_min{1};
    int quantity_max{100};
    double cancel_probability{0.08};
    double modify_probability{0.03};
};

struct ScenarioConfig {
    std::string name;
    int duration_sec{60};
    uint64_t seed{42};
    int order_limit{0};
    std::vector<std::string> symbols;
    double tick_size{0.0001};
    std::unordered_map<std::string, double> initial_mid_prices;
    std::string strategy;
    WorkloadConfig workload;
    EngineConfig engine;
    std::string event_log_path{"results/output.bin"};
    std::string metrics_path{"results/output-metrics.json"};
};

struct AppConfig {
    std::string exchange_name{"FAST Exchange"};
    std::vector<std::string> default_symbols;
    double tick_size{0.0001};
    std::unordered_map<std::string, double> initial_mid_prices;
    EngineConfig engine;
    WorkloadConfig workload;
    int benchmark_warmup{5};
    int benchmark_iterations{10};
    int dashboard_port{8080};
    std::string api_host{"127.0.0.1"};
    int api_port{8080};
    std::string output_directory{"results"};
};

class ConfigLoader {
public:
    static AppConfig load_default(const std::string& path = "config/default.yaml");
    static ScenarioConfig load_scenario(const std::string& scenario_path,
                                          const std::string& default_path = "config/default.yaml");
    static uint64_t hash_config(const ScenarioConfig& config);
};

}  // namespace fastexchange
