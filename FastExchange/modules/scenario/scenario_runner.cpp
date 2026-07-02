#include "scenario/scenario_runner.hpp"
#include "engine/exchange_engine.hpp"
#include "eventlog/journal.hpp"
#include "metrics/metrics.hpp"
#include "metrics/system_metrics.hpp"
#include "symbols/symbol_registry.hpp"
#include "workload/register_strategies.hpp"
#include "workload/replay_strategy.hpp"
#include "workload/workload_strategy.hpp"
#include <chrono>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace fastexchange {

ScenarioResult ScenarioRunner::run(const ScenarioConfig& config, ScenarioHooks* hooks) {
    register_workload_strategies();

    SymbolRegistry local_registry;
    SymbolRegistry* registry = hooks && hooks->registry ? hooks->registry : &local_registry;

    for (const auto& sym : config.symbols) {
        if (!registry->has_symbol(sym)) {
            Price mid = 0;
            if (config.initial_mid_prices.contains(sym)) {
                mid = to_ticks(config.initial_mid_prices.at(sym), config.tick_size);
            }
            registry->register_symbol(sym, mid);
        }
    }

    MetricsEngine local_metrics;
    MetricsEngine* metrics = hooks && hooks->metrics ? hooks->metrics : &local_metrics;
    metrics->set_scenario(config.name);
    if (!hooks || !hooks->metrics) {
        metrics->reset();
    }

    std::filesystem::create_directories(
        std::filesystem::path(config.event_log_path).parent_path());

    EventJournal journal;
    journal.open(config.event_log_path, config.seed, ConfigLoader::hash_config(config));

    ExchangeEngine engine(*registry, journal, *metrics);
    engine.configure(config.engine);

    auto strategy = WorkloadFactory::instance().create(config.strategy);
    strategy->configure(config.workload, config.seed, config.symbols, config.initial_mid_prices,
                        config.tick_size);
    if (config.order_limit > 0) {
        strategy->set_order_limit(config.order_limit);
    }

    if (config.strategy == "replay") {
        if (auto* replay = dynamic_cast<ReplayStrategy*>(strategy.get())) {
            replay->load_journal(config.event_log_path);
        }
    }

    uint64_t sim_time = 0;
    const uint64_t ns_per_order =
        config.workload.arrival_rate > 0 ? 1'000'000'000ULL / config.workload.arrival_rate : 1000;

    engine.start_scenario(config.name, config.seed, sim_time);

    auto wall_start = std::chrono::steady_clock::now();
    int orders_processed = 0;
    int max_orders = config.order_limit > 0 ? config.order_limit
                                              : config.duration_sec * config.workload.arrival_rate;
    const int progress_interval = hooks ? hooks->progress_interval : 0;

    while (strategy->has_more()) {
        if (config.order_limit == 0 && config.duration_sec > 0) {
            auto elapsed = std::chrono::steady_clock::now() - wall_start;
            if (std::chrono::duration<double>(elapsed).count() >= config.duration_sec) break;
        }
        if (max_orders > 0 && orders_processed >= max_orders) break;

        auto event = strategy->next(sim_time);
        if (!event) break;

        engine.process(*event, sim_time);
        sim_time += ns_per_order;
        ++orders_processed;

        if (hooks && hooks->on_progress && progress_interval > 0 &&
            orders_processed % progress_interval == 0) {
            auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - wall_start)
                               .count();
            auto sys = sample_system_metrics();
            metrics->gauge("memory.rss_mb", sys.rss_mb);
            metrics->gauge("cpu.percent", sys.cpu_percent);
            metrics->set_throughput(orders_processed / std::max(elapsed, 0.001),
                                    engine.trade_count() / std::max(elapsed, 0.001));
            hooks->on_progress(engine, orders_processed, elapsed);
        }
    }

    engine.end_scenario(config.name, config.seed, sim_time);
    journal.close();

    auto wall_end = std::chrono::steady_clock::now();
    double duration = std::chrono::duration<double>(wall_end - wall_start).count();
    double ops = duration > 0 ? orders_processed / duration : 0;

    auto sys = sample_system_metrics();
    metrics->gauge("memory.rss_mb", sys.rss_mb);
    metrics->gauge("cpu.percent", sys.cpu_percent);
    metrics->set_throughput(ops, engine.trade_count() / std::max(duration, 0.001));

    if (hooks && hooks->on_progress) {
        hooks->on_progress(engine, orders_processed, duration);
    }

    metrics->save_to_file(config.metrics_path);

    ScenarioResult result;
    result.event_log_path = config.event_log_path;
    result.metrics_path = config.metrics_path;
    result.sha256 = EventJournal::compute_file_hash(config.event_log_path);
    result.event_count = journal.event_count();
    result.trade_count = engine.trade_count();
    result.duration_sec = duration;

    spdlog::info("Scenario '{}' complete: {} orders, {} trades, {:.2f}s, sha256={}",
                 config.name, orders_processed, result.trade_count, duration, result.sha256);
    return result;
}

ScenarioResult ScenarioRunner::run_file(const std::string& scenario_path,
                                          const std::string& default_config,
                                          ScenarioHooks* hooks) {
    auto config = ConfigLoader::load_scenario(scenario_path, default_config);
    return run(config, hooks);
}

}  // namespace fastexchange
