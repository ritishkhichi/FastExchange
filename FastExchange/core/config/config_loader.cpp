#include "config/config_loader.hpp"
#include <filesystem>
#include <functional>
#include <stdexcept>

namespace fastexchange {

namespace {

std::vector<std::string> get_string_list(const YAML::Node& node) {
    std::vector<std::string> result;
    if (!node || !node.IsSequence()) return result;
    for (const auto& item : node) {
        result.push_back(item.as<std::string>());
    }
    return result;
}

std::unordered_map<std::string, double> get_price_map(const YAML::Node& node) {
    std::unordered_map<std::string, double> result;
    if (!node || !node.IsMap()) return result;
    for (const auto& item : node) {
        result[item.first.as<std::string>()] = item.second.as<double>();
    }
    return result;
}

WorkloadConfig parse_workload(const YAML::Node& node) {
    WorkloadConfig w;
    if (!node) return w;
    if (node["profile"]) w.profile = node["profile"].as<std::string>();
    if (node["arrival_rate"]) w.arrival_rate = node["arrival_rate"].as<int>();
    if (node["buy_probability"]) w.buy_probability = node["buy_probability"].as<double>();
    if (node["sell_probability"]) w.sell_probability = node["sell_probability"].as<double>();
    if (node["price_distribution"]) w.price_distribution = node["price_distribution"].as<std::string>();
    if (node["price_bias_ticks"]) w.price_bias_ticks = node["price_bias_ticks"].as<int>();
    if (node["quantity_distribution"])
        w.quantity_distribution = node["quantity_distribution"].as<std::string>();
    if (node["quantity_lambda"]) w.quantity_lambda = node["quantity_lambda"].as<double>();
    if (node["quantity_min"]) w.quantity_min = node["quantity_min"].as<int>();
    if (node["quantity_max"]) w.quantity_max = node["quantity_max"].as<int>();
    if (node["cancel_probability"]) w.cancel_probability = node["cancel_probability"].as<double>();
    if (node["modify_probability"]) w.modify_probability = node["modify_probability"].as<double>();
    return w;
}

EngineConfig parse_engine(const YAML::Node& node) {
    EngineConfig e;
    if (!node) return e;
    if (node["max_order_size"]) e.max_order_size = node["max_order_size"].as<uint64_t>();
    if (node["auto_register_symbols"])
        e.auto_register_symbols = node["auto_register_symbols"].as<bool>();
    if (node["risk_plugin"]) e.risk_plugin = node["risk_plugin"].as<std::string>();
    if (node["matching_algorithm"])
        e.matching_algorithm = node["matching_algorithm"].as<std::string>();
    return e;
}

YAML::Node load_yaml(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("config not found: " + path);
    }
    return YAML::LoadFile(path);
}

}  // namespace

AppConfig ConfigLoader::load_default(const std::string& path) {
    auto root = load_yaml(path);
    AppConfig cfg;

    if (root["exchange"] && root["exchange"]["name"]) {
        cfg.exchange_name = root["exchange"]["name"].as<std::string>();
    }
    cfg.default_symbols = get_string_list(root["symbols"]);
    if (root["tick_size"]) cfg.tick_size = root["tick_size"].as<double>();
    cfg.initial_mid_prices = get_price_map(root["initial_mid_prices"]);

    if (root["risk"]) {
        auto risk = root["risk"];
        if (risk["max_order_size"]) cfg.engine.max_order_size = risk["max_order_size"].as<uint64_t>();
        if (risk["auto_register_symbols"])
            cfg.engine.auto_register_symbols = risk["auto_register_symbols"].as<bool>();
    }
    if (root["matching"] && root["matching"]["algorithm"]) {
        cfg.engine.matching_algorithm = root["matching"]["algorithm"].as<std::string>();
    }
    cfg.workload = parse_workload(root["workload"]);
    if (root["benchmark"]) {
        if (root["benchmark"]["warmup"]) cfg.benchmark_warmup = root["benchmark"]["warmup"].as<int>();
        if (root["benchmark"]["iterations"])
            cfg.benchmark_iterations = root["benchmark"]["iterations"].as<int>();
    }
    if (root["dashboard"] && root["dashboard"]["port"]) {
        cfg.dashboard_port = root["dashboard"]["port"].as<int>();
    }
    if (root["api"]) {
        if (root["api"]["host"]) cfg.api_host = root["api"]["host"].as<std::string>();
        if (root["api"]["port"]) cfg.api_port = root["api"]["port"].as<int>();
    }
    if (root["output"] && root["output"]["directory"]) {
        cfg.output_directory = root["output"]["directory"].as<std::string>();
    }
    return cfg;
}

ScenarioConfig ConfigLoader::load_scenario(const std::string& scenario_path,
                                           const std::string& default_path) {
    auto defaults = load_default(default_path);
    auto root = load_yaml(scenario_path);

    ScenarioConfig cfg;
    cfg.name = root["name"] ? root["name"].as<std::string>() : "unnamed";
    cfg.duration_sec = root["duration_sec"] ? root["duration_sec"].as<int>() : 60;
    cfg.seed = root["seed"] ? root["seed"].as<uint64_t>() : 42;
    if (root["order_limit"]) cfg.order_limit = root["order_limit"].as<int>();

    cfg.symbols = get_string_list(root["symbols"]);
    if (cfg.symbols.empty()) cfg.symbols = defaults.default_symbols;

    cfg.tick_size = root["tick_size"] ? root["tick_size"].as<double>() : defaults.tick_size;
    cfg.initial_mid_prices = get_price_map(root["initial_mid_prices"]);
    if (cfg.initial_mid_prices.empty()) cfg.initial_mid_prices = defaults.initial_mid_prices;

    cfg.strategy = root["strategy"] ? root["strategy"].as<std::string>() : defaults.workload.profile;
    cfg.workload = parse_workload(root["workload"]);
    if (!root["workload"]) cfg.workload = defaults.workload;

    cfg.engine = parse_engine(root["engine"]);
    if (!root["engine"]) cfg.engine = defaults.engine;
    if (root["risk"] && root["risk"]["max_order_size"]) {
        cfg.engine.max_order_size = root["risk"]["max_order_size"].as<uint64_t>();
    }

    if (root["output"]) {
        if (root["output"]["event_log"])
            cfg.event_log_path = root["output"]["event_log"].as<std::string>();
        if (root["output"]["metrics"])
            cfg.metrics_path = root["output"]["metrics"].as<std::string>();
    }

    return cfg;
}

uint64_t ConfigLoader::hash_config(const ScenarioConfig& config) {
    std::hash<std::string> hasher;
    uint64_t h = hasher(config.name);
    h ^= hasher(config.strategy) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= config.seed + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    for (const auto& s : config.symbols) {
        h ^= hasher(s) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    }
    return h;
}

}  // namespace fastexchange
