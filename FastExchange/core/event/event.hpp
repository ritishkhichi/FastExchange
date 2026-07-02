#pragma once

#include "types/inbound_event.hpp"
#include "types/trade.hpp"
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace fastexchange {

enum class EventType : uint16_t {
    OrderSubmitRequested = 1,
    OrderCancelRequested = 2,
    OrderModifyRequested = 3,
    RiskChecked = 10,
    OrderAccepted = 11,
    OrderRejected = 12,
    TradeExecuted = 20,
    OrderPartiallyFilled = 21,
    OrderFullyFilled = 22,
    OrderCancelled = 30,
    OrderModified = 31,
    OrderBookUpdated = 40,
    ScenarioStarted = 100,
    ScenarioEnded = 101,
};

struct RiskCheckedPayload {
    OrderId order_id;
    bool passed;
    std::string reason;
};

struct OrderPayload {
    Order order;
};

struct TradePayload {
    Trade trade;
};

struct CancelPayload {
    OrderId order_id;
    Symbol symbol;
};

struct ModifyPayload {
    OrderId order_id;
    Symbol symbol;
    Price new_price;
    Quantity new_quantity;
};

struct RejectPayload {
    OrderId order_id;
    std::string reason;
};

struct BookUpdatePayload {
    Symbol symbol;
    Price best_bid;
    Price best_ask;
    Quantity bid_depth;
    Quantity ask_depth;
};

struct ScenarioPayload {
    std::string name;
    uint64_t seed;
};

using EventPayload = std::variant<std::monostate, OrderPayload, TradePayload, CancelPayload,
                                  ModifyPayload, RejectPayload, RiskCheckedPayload,
                                  BookUpdatePayload, ScenarioPayload>;

struct Event {
    EventType type{EventType::OrderSubmitRequested};
    Timestamp timestamp{0};
    EventPayload payload;
};

const char* event_type_name(EventType type);

}  // namespace fastexchange
