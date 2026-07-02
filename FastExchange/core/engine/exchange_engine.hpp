#pragma once

#include "config/config_loader.hpp"
#include "eventlog/journal.hpp"
#include "matching/matching_plugin.hpp"
#include "metrics/metrics.hpp"
#include "risk/risk_plugin.hpp"
#include "symbols/symbol_registry.hpp"
#include "types/inbound_event.hpp"
#include <chrono>
#include <memory>
#include <vector>

namespace fastexchange {

class ExchangeEngine {
public:
    ExchangeEngine(SymbolRegistry& registry, EventJournal& journal, MetricsEngine& metrics);

    void configure(const EngineConfig& config);
    void process(const InboundEvent& event, Timestamp sim_time);
    void start_scenario(const std::string& name, uint64_t seed, Timestamp sim_time);
    void end_scenario(const std::string& name, uint64_t seed, Timestamp sim_time);

    SymbolRegistry& registry() { return registry_; }
    const std::vector<Trade>& recent_trades() const { return recent_trades_; }
    uint64_t trade_count() const { return trade_count_; }

private:
    SymbolRegistry& registry_;
    EventJournal& journal_;
    MetricsEngine& metrics_;
    std::unique_ptr<IRiskPlugin> risk_;
    std::unique_ptr<IMatchingPlugin> matcher_;
    EngineConfig config_;
    std::vector<Trade> recent_trades_;
    uint64_t trade_count_{0};
    static constexpr size_t kMaxRecentTrades = 100;

    void append_event(const Event& event);
    void emit_book_update(const Symbol& symbol, Timestamp ts);
    double measure_us(const std::chrono::steady_clock::time_point& start) const;
};

}  // namespace fastexchange
