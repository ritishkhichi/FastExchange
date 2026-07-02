#include "engine/exchange_engine.hpp"
#include "matching/price_time_fifo.hpp"
#include "risk/default_risk.hpp"
#include <algorithm>

namespace fastexchange {

ExchangeEngine::ExchangeEngine(SymbolRegistry& registry, EventJournal& journal,
                                 MetricsEngine& metrics)
    : registry_(registry), journal_(journal), metrics_(metrics) {
    risk_ = std::make_unique<DefaultRiskPlugin>();
    matcher_ = std::make_unique<PriceTimeFifoMatcher>();
}

void ExchangeEngine::configure(const EngineConfig& config) {
    config_ = config;
    if (auto* dr = dynamic_cast<DefaultRiskPlugin*>(risk_.get())) {
        dr->reset();
    }
}

void ExchangeEngine::append_event(const Event& event) {
    journal_.append(event);
    metrics_.counter("events.total");
}

double ExchangeEngine::measure_us(
    const std::chrono::steady_clock::time_point& start) const {
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::micro>(end - start).count();
}

void ExchangeEngine::emit_book_update(const Symbol& symbol, Timestamp ts) {
    auto snap = registry_.get_book(symbol).snapshot();
    Event e;
    e.type = EventType::OrderBookUpdated;
    e.timestamp = ts;
    e.payload = BookUpdatePayload{symbol, snap.best_bid, snap.best_ask, snap.bid_depth,
                                  snap.ask_depth};
    append_event(e);
}

void ExchangeEngine::start_scenario(const std::string& name, uint64_t seed, Timestamp sim_time) {
    Event e;
    e.type = EventType::ScenarioStarted;
    e.timestamp = sim_time;
    e.payload = ScenarioPayload{name, seed};
    append_event(e);
}

void ExchangeEngine::end_scenario(const std::string& name, uint64_t seed, Timestamp sim_time) {
    Event e;
    e.type = EventType::ScenarioEnded;
    e.timestamp = sim_time;
    e.payload = ScenarioPayload{name, seed};
    append_event(e);
}

void ExchangeEngine::process(const InboundEvent& event, Timestamp sim_time) {
    auto total_start = std::chrono::steady_clock::now();

    if (std::holds_alternative<OrderSubmit>(event)) {
        const auto& submit = std::get<OrderSubmit>(event);
        Order order = submit.order;
        order.timestamp = sim_time;

        Event req;
        req.type = EventType::OrderSubmitRequested;
        req.timestamp = sim_time;
        req.payload = OrderPayload{order};
        append_event(req);
        metrics_.counter("orders.submitted");

        auto risk_start = std::chrono::steady_clock::now();
        auto risk_result =
            risk_->check(event, registry_, config_.max_order_size, config_.auto_register_symbols);
        metrics_.histogram("latency.stage.risk", measure_us(risk_start));

        Event risk_evt;
        risk_evt.type = EventType::RiskChecked;
        risk_evt.timestamp = sim_time;
        risk_evt.payload =
            RiskCheckedPayload{order.id, risk_result.passed, risk_result.reason};
        append_event(risk_evt);

        if (!risk_result.passed) {
            Event rej;
            rej.type = EventType::OrderRejected;
            rej.timestamp = sim_time;
            rej.payload = RejectPayload{order.id, risk_result.reason};
            append_event(rej);
            metrics_.counter("orders.rejected");
            metrics_.histogram("latency.order.total", measure_us(total_start));
            return;
        }

        if (!registry_.has_symbol(order.symbol)) {
            registry_.register_symbol(order.symbol);
        }

        Event acc;
        acc.type = EventType::OrderAccepted;
        acc.timestamp = sim_time;
        acc.payload = OrderPayload{order};
        append_event(acc);
        metrics_.counter("orders.accepted");

        auto match_start = std::chrono::steady_clock::now();
        auto& book = registry_.get_book(order.symbol);
        auto match_result = matcher_->match(book, order);
        metrics_.histogram("latency.stage.match", measure_us(match_start));

        for (const auto& trade : match_result.trades) {
            Event te;
            te.type = EventType::TradeExecuted;
            te.timestamp = sim_time;
            te.payload = TradePayload{trade};
            append_event(te);
            metrics_.counter("trades.total");
            ++trade_count_;

            recent_trades_.push_back(trade);
            if (recent_trades_.size() > kMaxRecentTrades) {
                recent_trades_.erase(recent_trades_.begin());
            }
        }

        if (match_result.remainder) {
            book.add_order(*match_result.remainder);
            Event pf;
            pf.type = (match_result.trades.empty() ? EventType::OrderAccepted
                                                   : EventType::OrderPartiallyFilled);
            if (!match_result.trades.empty() && match_result.remainder->remaining > 0) {
                pf.type = EventType::OrderPartiallyFilled;
            }
            pf.timestamp = sim_time;
            pf.payload = OrderPayload{*match_result.remainder};
            append_event(pf);
        } else if (!match_result.trades.empty()) {
            Event ff;
            ff.type = EventType::OrderFullyFilled;
            ff.timestamp = sim_time;
            order.remaining = 0;
            ff.payload = OrderPayload{order};
            append_event(ff);
        }

        emit_book_update(order.symbol, sim_time);
    } else if (std::holds_alternative<OrderCancel>(event)) {
        const auto& cancel = std::get<OrderCancel>(event);

        Event req;
        req.type = EventType::OrderCancelRequested;
        req.timestamp = sim_time;
        req.payload = CancelPayload{cancel.order_id, cancel.symbol};
        append_event(req);

        auto risk_result =
            risk_->check(event, registry_, config_.max_order_size, config_.auto_register_symbols);
        if (!risk_result.passed) {
            metrics_.counter("orders.rejected");
            return;
        }

        auto& book = registry_.get_book(cancel.symbol);
        if (book.cancel_order(cancel.order_id)) {
            Event ce;
            ce.type = EventType::OrderCancelled;
            ce.timestamp = sim_time;
            ce.payload = CancelPayload{cancel.order_id, cancel.symbol};
            append_event(ce);
            metrics_.counter("orders.cancelled");
            emit_book_update(cancel.symbol, sim_time);
        }
    } else if (std::holds_alternative<OrderModify>(event)) {
        const auto& modify = std::get<OrderModify>(event);

        Event req;
        req.type = EventType::OrderModifyRequested;
        req.timestamp = sim_time;
        ModifyPayload mp{modify.order_id, modify.symbol, modify.new_price.value_or(0),
                         modify.new_quantity.value_or(0)};
        req.payload = mp;
        append_event(req);

        auto risk_result =
            risk_->check(event, registry_, config_.max_order_size, config_.auto_register_symbols);
        if (!risk_result.passed) {
            metrics_.counter("orders.rejected");
            return;
        }

        auto& book = registry_.get_book(modify.symbol);
        Price np = modify.new_price.value_or(0);
        Quantity nq = modify.new_quantity.value_or(0);
        if (book.modify_order(modify.order_id, np, nq)) {
            Event me;
            me.type = EventType::OrderModified;
            me.timestamp = sim_time;
            me.payload = mp;
            append_event(me);
            emit_book_update(modify.symbol, sim_time);
        }
    }

    metrics_.histogram("latency.order.total", measure_us(total_start));
}

}  // namespace fastexchange
