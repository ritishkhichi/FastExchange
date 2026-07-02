#include "benchmark/benchmark_runner.hpp"
#include "eventlog/journal.hpp"
#include "metrics/metrics.hpp"
#include "replay/replay_engine.hpp"
#include "scenario/scenario_runner.hpp"
#include <CLI/CLI.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
    CLI::App app{"FastExchange - High-Performance Exchange Simulator"};
    app.require_subcommand(1);

    std::string scenario_path;
    std::string export_input;
    std::string export_output;
    std::string verify_path;
    uint64_t verify_seed = 0;
    std::string stats_path;
    std::string compare_a;
    std::string compare_b;
    std::string replay_path;
    double replay_speed = 1.0;
    bool replay_verify = false;
    int benchmark_orders = 100000;
    std::string default_config = "config/default.yaml";

    auto* run_cmd = app.add_subcommand("run", "Run a scenario from YAML");
    run_cmd->add_option("scenario", scenario_path, "Scenario YAML file")->required();

    auto* export_cmd = app.add_subcommand("export", "Export binary journal to JSON");
    export_cmd->add_option("input", export_input, "Input .bin file")->required();
    export_cmd->add_option("-o,--output", export_output, "Output JSON file")->required();

    auto* verify_cmd = app.add_subcommand("verify", "Verify journal SHA256");
    verify_cmd->add_option("journal", verify_path, "Journal .bin file")->required();
    verify_cmd->add_option("--seed", verify_seed, "Expected seed (informational)");

    auto* stats_cmd = app.add_subcommand("stats", "Print metrics summary");
    stats_cmd->add_option("metrics", stats_path, "Metrics JSON file")->required();

    auto* compare_cmd = app.add_subcommand("compare", "Compare two metrics files");
    compare_cmd->add_option("file_a", compare_a, "First metrics JSON")->required();
    compare_cmd->add_option("file_b", compare_b, "Second metrics JSON")->required();

    auto* benchmark_cmd = app.add_subcommand("benchmark", "Run benchmark");
    benchmark_cmd->add_option("--scenario", scenario_path, "Scenario YAML")->required();
    benchmark_cmd->add_option("--orders", benchmark_orders, "Order count")->default_val(100000);

    auto* replay_cmd = app.add_subcommand("replay", "Replay event journal");
    replay_cmd->add_option("journal", replay_path, "Journal .bin file")->required();
    replay_cmd->add_option("--speed", replay_speed, "Replay speed multiplier")->default_val(1.0);
    replay_cmd->add_flag("--verify", replay_verify, "Verify determinism by reprocessing");

    CLI11_PARSE(app, argc, argv);

    try {
        if (run_cmd->parsed()) {
            fastexchange::ScenarioRunner runner;
            auto result = runner.run_file(scenario_path, default_config);
            std::cout << "Event log: " << result.event_log_path << "\n";
            std::cout << "Metrics:   " << result.metrics_path << "\n";
            std::cout << "SHA256:    " << result.sha256 << "\n";
            std::cout << "Trades:    " << result.trade_count << "\n";
        } else if (export_cmd->parsed()) {
            fastexchange::EventJournal::export_to_json(export_input, export_output);
            std::cout << "Exported to " << export_output << "\n";
        } else if (verify_cmd->parsed()) {
            bool ok = fastexchange::EventJournal::verify_file(verify_path);
            auto hash = fastexchange::EventJournal::compute_file_hash(verify_path);
            std::cout << "SHA256: " << hash << "\n";
            std::cout << (ok ? "VERIFY: PASS" : "VERIFY: FAIL") << "\n";
            if (verify_seed) std::cout << "Seed (info): " << verify_seed << "\n";
            return ok ? 0 : 1;
        } else if (stats_cmd->parsed()) {
            std::ifstream in(stats_path);
            nlohmann::json j;
            in >> j;
            std::cout << j.dump(2) << "\n";
        } else if (compare_cmd->parsed()) {
            fastexchange::compare_metrics(compare_a, compare_b);
        } else if (benchmark_cmd->parsed()) {
            fastexchange::BenchmarkRunner bench;
            bench.run(scenario_path, benchmark_orders, default_config);
        } else if (replay_cmd->parsed()) {
            if (replay_verify) {
                auto result = fastexchange::ReplayEngine::verify(replay_path, default_config);
                return result.hash_match ? 0 : 1;
            }
            fastexchange::ReplayEngine::replay(replay_path, replay_speed);
        }
    } catch (const std::exception& ex) {
        spdlog::error("{}", ex.what());
        return 1;
    }

    return 0;
}
