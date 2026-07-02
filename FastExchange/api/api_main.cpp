#include "server.hpp"
#include "engine/exchange_engine.hpp"
#include "scenario/scenario_runner.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    CLI::App app{"FastExchange API Server"};
    std::string host = "127.0.0.1";
    int port = 8080;
    std::string scenario_path;
    std::string metrics_path;
    int order_limit = 0;

    app.add_option("--host", host, "Bind host")->default_val("127.0.0.1");
    app.add_option("--port", port, "Bind port")->default_val(8080);
    app.add_option("--scenario", scenario_path, "Run live scenario (synthetic orders, updates dashboard)");
    app.add_option("--metrics", metrics_path,
                   "Serve static metrics JSON (post-run snapshot only)");
    app.add_option("--orders", order_limit, "Order limit for live scenario (0 = use scenario YAML)");

    CLI11_PARSE(app, argc, argv);

    fastexchange::ApiState state;
    state.registry = std::make_unique<fastexchange::SymbolRegistry>();

    const bool live_mode = !scenario_path.empty();

    if (!live_mode && !metrics_path.empty()) {
        state.load_metrics_file(metrics_path);
        state.register_symbols({"ABC", "XYZ", "TECH", "COIN", "FOOD"});
        std::cout << "Serving static metrics from " << metrics_path << "\n";
    } else if (!live_mode && std::filesystem::exists("results/balanced-metrics.json")) {
        state.load_metrics_file("results/balanced-metrics.json");
        state.register_symbols({"ABC", "XYZ", "TECH"});
        std::cout << "Serving static metrics from results/balanced-metrics.json\n";
    }

    if (live_mode) {
        std::thread bg([&state, scenario_path, order_limit]() {
            try {
                fastexchange::ScenarioRunner runner;
                auto config = fastexchange::ConfigLoader::load_scenario(scenario_path);
                if (order_limit > 0) {
                    config.order_limit = order_limit;
                    config.duration_sec = 0;
                }
                config.event_log_path = "results/live-" + config.name + ".bin";
                config.metrics_path = "results/live-" + config.name + "-metrics.json";

                {
                    std::lock_guard lock(state.mutex);
                    state.scenario_name = config.name;
                    state.running = true;
                    state.has_metrics_cache = false;
                    state.metrics.set_scenario(config.name);
                    state.metrics.reset();
                    state.register_symbols(config.symbols);
                }

                fastexchange::ScenarioHooks hooks;
                hooks.registry = state.registry.get();
                hooks.metrics = &state.metrics;
                hooks.progress_interval = 200;
                hooks.on_progress = [&state](fastexchange::ExchangeEngine& engine, int /*orders*/,
                                               double /*elapsed*/) {
                    state.sync_live_snapshot(state.metrics.snapshot(), engine.recent_trades());
                };

                std::cout << "Live scenario started: " << config.name << "\n";
                auto result = runner.run(config, &hooks);

                {
                    std::lock_guard lock(state.mutex);
                    state.running = false;
                    state.load_metrics_file(result.metrics_path);
                }
                std::cout << "Live scenario complete. Metrics at " << result.metrics_path << "\n";
            } catch (const std::exception& ex) {
                std::cerr << "Scenario error: " << ex.what() << "\n";
                state.running = false;
            }
        });
        bg.detach();
    }

    fastexchange::ApiServer server(state);
    server.run(host, port);
    return 0;
}
