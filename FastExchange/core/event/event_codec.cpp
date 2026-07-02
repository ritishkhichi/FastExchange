#include "event/event_codec.hpp"
#include <cstring>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace fastexchange {

namespace {

void write_u16(std::vector<uint8_t>& buf, uint16_t v) {
    buf.push_back(static_cast<uint8_t>(v & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}

void write_u64(std::vector<uint8_t>& buf, uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        buf.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
    }
}

void write_i64(std::vector<uint8_t>& buf, int64_t v) {
    write_u64(buf, static_cast<uint64_t>(v));
}

void write_string(std::vector<uint8_t>& buf, const std::string& s) {
    write_u16(buf, static_cast<uint16_t>(s.size()));
    buf.insert(buf.end(), s.begin(), s.end());
}

uint16_t read_u16(const uint8_t*& p) {
    uint16_t v = static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
    p += 2;
    return v;
}

uint64_t read_u64(const uint8_t*& p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v |= static_cast<uint64_t>(p[i]) << (i * 8);
    }
    p += 8;
    return v;
}

int64_t read_i64(const uint8_t*& p) {
    return static_cast<int64_t>(read_u64(p));
}

std::string read_string(const uint8_t*& p) {
    auto len = read_u16(p);
    std::string s(reinterpret_cast<const char*>(p), len);
    p += len;
    return s;
}

void encode_order(std::vector<uint8_t>& buf, const Order& o) {
    write_u64(buf, o.id);
    write_string(buf, o.symbol);
    buf.push_back(static_cast<uint8_t>(o.side));
    write_i64(buf, o.price);
    write_u64(buf, o.quantity);
    write_u64(buf, o.remaining);
    write_u64(buf, o.timestamp);
    write_u64(buf, o.sequence);
}

Order decode_order(const uint8_t*& p) {
    Order o;
    o.id = read_u64(p);
    o.symbol = read_string(p);
    o.side = static_cast<Side>(p[0]);
    p += 1;
    o.price = read_i64(p);
    o.quantity = read_u64(p);
    o.remaining = read_u64(p);
    o.timestamp = read_u64(p);
    o.sequence = read_u64(p);
    o.type = OrderType::Limit;
    return o;
}

}  // namespace

const char* event_type_name(EventType type) {
    switch (type) {
        case EventType::OrderSubmitRequested: return "OrderSubmitRequested";
        case EventType::OrderCancelRequested: return "OrderCancelRequested";
        case EventType::OrderModifyRequested: return "OrderModifyRequested";
        case EventType::RiskChecked: return "RiskChecked";
        case EventType::OrderAccepted: return "OrderAccepted";
        case EventType::OrderRejected: return "OrderRejected";
        case EventType::TradeExecuted: return "TradeExecuted";
        case EventType::OrderPartiallyFilled: return "OrderPartiallyFilled";
        case EventType::OrderFullyFilled: return "OrderFullyFilled";
        case EventType::OrderCancelled: return "OrderCancelled";
        case EventType::OrderModified: return "OrderModified";
        case EventType::OrderBookUpdated: return "OrderBookUpdated";
        case EventType::ScenarioStarted: return "ScenarioStarted";
        case EventType::ScenarioEnded: return "ScenarioEnded";
        default: return "Unknown";
    }
}

std::vector<uint8_t> EventCodec::encode_payload(const Event& event) {
    std::vector<uint8_t> buf;
    switch (event.type) {
        case EventType::OrderSubmitRequested:
        case EventType::OrderAccepted:
        case EventType::OrderPartiallyFilled:
        case EventType::OrderFullyFilled: {
            const auto& p = std::get<OrderPayload>(event.payload);
            encode_order(buf, p.order);
            break;
        }
        case EventType::OrderCancelRequested:
        case EventType::OrderCancelled: {
            const auto& p = std::get<CancelPayload>(event.payload);
            write_u64(buf, p.order_id);
            write_string(buf, p.symbol);
            break;
        }
        case EventType::OrderModifyRequested:
        case EventType::OrderModified: {
            const auto& p = std::get<ModifyPayload>(event.payload);
            write_u64(buf, p.order_id);
            write_string(buf, p.symbol);
            write_i64(buf, p.new_price);
            write_u64(buf, p.new_quantity);
            break;
        }
        case EventType::RiskChecked: {
            const auto& p = std::get<RiskCheckedPayload>(event.payload);
            write_u64(buf, p.order_id);
            buf.push_back(p.passed ? 1 : 0);
            write_string(buf, p.reason);
            break;
        }
        case EventType::OrderRejected: {
            const auto& p = std::get<RejectPayload>(event.payload);
            write_u64(buf, p.order_id);
            write_string(buf, p.reason);
            break;
        }
        case EventType::TradeExecuted: {
            const auto& p = std::get<TradePayload>(event.payload);
            write_u64(buf, p.trade.buy_order_id);
            write_u64(buf, p.trade.sell_order_id);
            write_string(buf, p.trade.symbol);
            write_i64(buf, p.trade.price);
            write_u64(buf, p.trade.quantity);
            write_u64(buf, p.trade.timestamp);
            break;
        }
        case EventType::OrderBookUpdated: {
            const auto& p = std::get<BookUpdatePayload>(event.payload);
            write_string(buf, p.symbol);
            write_i64(buf, p.best_bid);
            write_i64(buf, p.best_ask);
            write_u64(buf, p.bid_depth);
            write_u64(buf, p.ask_depth);
            break;
        }
        case EventType::ScenarioStarted:
        case EventType::ScenarioEnded: {
            const auto& p = std::get<ScenarioPayload>(event.payload);
            write_string(buf, p.name);
            write_u64(buf, p.seed);
            break;
        }
        default:
            break;
    }
    return buf;
}

Event EventCodec::decode_payload(EventType type, Timestamp ts, const std::vector<uint8_t>& data) {
    Event event;
    event.type = type;
    event.timestamp = ts;
    const uint8_t* p = data.data();
    const uint8_t* end = data.data() + data.size();

    switch (type) {
        case EventType::OrderSubmitRequested:
        case EventType::OrderAccepted:
        case EventType::OrderPartiallyFilled:
        case EventType::OrderFullyFilled: {
            if (p >= end) throw std::runtime_error("truncated order payload");
            event.payload = OrderPayload{decode_order(p)};
            break;
        }
        case EventType::OrderCancelRequested:
        case EventType::OrderCancelled: {
            CancelPayload cp;
            cp.order_id = read_u64(p);
            cp.symbol = read_string(p);
            event.payload = cp;
            break;
        }
        case EventType::OrderModifyRequested:
        case EventType::OrderModified: {
            ModifyPayload mp;
            mp.order_id = read_u64(p);
            mp.symbol = read_string(p);
            mp.new_price = read_i64(p);
            mp.new_quantity = read_u64(p);
            event.payload = mp;
            break;
        }
        case EventType::RiskChecked: {
            RiskCheckedPayload rp;
            rp.order_id = read_u64(p);
            rp.passed = (*p++) != 0;
            rp.reason = read_string(p);
            event.payload = rp;
            break;
        }
        case EventType::OrderRejected: {
            RejectPayload rp;
            rp.order_id = read_u64(p);
            rp.reason = read_string(p);
            event.payload = rp;
            break;
        }
        case EventType::TradeExecuted: {
            TradePayload tp;
            tp.trade.buy_order_id = read_u64(p);
            tp.trade.sell_order_id = read_u64(p);
            tp.trade.symbol = read_string(p);
            tp.trade.price = read_i64(p);
            tp.trade.quantity = read_u64(p);
            tp.trade.timestamp = read_u64(p);
            event.payload = tp;
            break;
        }
        case EventType::OrderBookUpdated: {
            BookUpdatePayload bp;
            bp.symbol = read_string(p);
            bp.best_bid = read_i64(p);
            bp.best_ask = read_i64(p);
            bp.bid_depth = read_u64(p);
            bp.ask_depth = read_u64(p);
            event.payload = bp;
            break;
        }
        case EventType::ScenarioStarted:
        case EventType::ScenarioEnded: {
            ScenarioPayload sp;
            sp.name = read_string(p);
            sp.seed = read_u64(p);
            event.payload = sp;
            break;
        }
        default:
            break;
    }
    return event;
}

std::string EventCodec::export_event_json(const Event& event) {
    nlohmann::json j;
    j["type"] = event_type_name(event.type);
    j["timestamp"] = event.timestamp;
    // Simplified export - key fields per type
    switch (event.type) {
        case EventType::TradeExecuted: {
            const auto& t = std::get<TradePayload>(event.payload).trade;
            j["buy_order_id"] = t.buy_order_id;
            j["sell_order_id"] = t.sell_order_id;
            j["symbol"] = t.symbol;
            j["price"] = t.price;
            j["quantity"] = t.quantity;
            break;
        }
        case EventType::OrderRejected: {
            const auto& r = std::get<RejectPayload>(event.payload);
            j["order_id"] = r.order_id;
            j["reason"] = r.reason;
            break;
        }
        case EventType::ScenarioStarted:
        case EventType::ScenarioEnded: {
            const auto& s = std::get<ScenarioPayload>(event.payload);
            j["name"] = s.name;
            j["seed"] = s.seed;
            break;
        }
        default:
            break;
    }
    return j.dump();
}

}  // namespace fastexchange
