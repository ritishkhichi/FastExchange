#include "replay/replay_engine.hpp"
#include "config/config_loader.hpp"
#include "engine/exchange_engine.hpp"
#include "eventlog/journal.hpp"
#include "metrics/metrics.hpp"
#include "symbols/symbol_registry.hpp"
#include <iostream>
#include <thread>

namespace fastexchange {

ReplayEngine::ReplayResult ReplayEngine::verify(const std::string& journal_path,
                                                  const std::string& /*default_config*/) {
    ReplayResult result;
    result.original_hash = EventJournal::compute_file_hash(journal_path);

    auto events = EventJournal::read_from_file(journal_path);
    if (events.empty()) {
        result.replay_hash = result.original_hash;
        result.hash_match = true;
        return result;
    }

    // Extract scenario info from first events
    SymbolRegistry registry;
    registry.register_symbol("ABC");
    registry.register_symbol("XYZ");
    registry.register_symbol("TECH");

    std::string replay_path = journal_path + ".replay.bin";
    MetricsEngine metrics;
    EventJournal journal;
    journal.open(replay_path, 0, 0);

    ExchangeEngine engine(registry, journal, metrics);
    EngineConfig ec;
    engine.configure(ec);

    uint64_t sim_time = 0;
    for (const auto& e : events) {
        if (e.type == EventType::OrderSubmitRequested) {
            const auto& o = std::get<OrderPayload>(e.payload).order;
            if (!registry.has_symbol(o.symbol)) registry.register_symbol(o.symbol);
            engine.process(OrderSubmit{o}, e.timestamp);
            sim_time = e.timestamp;
        } else if (e.type == EventType::OrderCancelRequested) {
            const auto& c = std::get<CancelPayload>(e.payload);
            engine.process(OrderCancel{c.order_id, c.symbol}, e.timestamp);
        } else if (e.type == EventType::OrderModifyRequested) {
            const auto& m = std::get<ModifyPayload>(e.payload);
            OrderModify om{m.order_id, m.symbol, m.new_price, m.new_quantity};
            engine.process(om, e.timestamp);
        } else if (e.type == EventType::ScenarioStarted) {
            const auto& s = std::get<ScenarioPayload>(e.payload);
            engine.start_scenario(s.name, s.seed, e.timestamp);
        } else if (e.type == EventType::ScenarioEnded) {
            const auto& s = std::get<ScenarioPayload>(e.payload);
            engine.end_scenario(s.name, s.seed, e.timestamp);
        }
        ++result.events_replayed;
    }

    journal.close();
    result.replay_hash = EventJournal::compute_file_hash(replay_path);
    result.hash_match = (result.original_hash == result.replay_hash);

    std::cout << "Replay verification: " << (result.hash_match ? "PASS" : "FAIL") << "\n";
    std::cout << "Original: " << result.original_hash << "\n";
    std::cout << "Replay:   " << result.replay_hash << "\n";

    return result;
}

void ReplayEngine::replay(const std::string& journal_path, double speed_multiplier) {
    auto events = EventJournal::read_from_file(journal_path);
    std::cout << "Replaying " << events.size() << " events at " << speed_multiplier << "x\n";

    uint64_t prev_ts = 0;
    for (const auto& e : events) {
        if (prev_ts > 0 && e.timestamp > prev_ts && speed_multiplier > 0) {
            auto delay_ns = static_cast<uint64_t>((e.timestamp - prev_ts) / speed_multiplier);
            std::this_thread::sleep_for(std::chrono::nanoseconds(delay_ns));
        }
        prev_ts = e.timestamp;
        std::cout << event_type_name(e.type) << " @ " << e.timestamp << "\n";
    }
}

}  // namespace fastexchange
