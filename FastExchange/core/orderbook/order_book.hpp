#pragma once

#include "types/order.hpp"
#include "types/trade.hpp"
#include <deque>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

namespace fastexchange {

struct OrderLocation {
    Side side;
    Price price;
    size_t index_in_deque;
};

struct BookSnapshot {
    Price best_bid{0};
    Price best_ask{0};
    Quantity bid_depth{0};
    Quantity ask_depth{0};
};

class OrderBook {
public:
    bool add_order(Order order);
    bool cancel_order(OrderId id);
    bool modify_order(OrderId id, Price new_price, Quantity new_qty);

    std::optional<Order> pop_best_opposite(Side incoming_side);
    void requeue_remainder(const Order& order);

    BookSnapshot snapshot() const;
    const std::vector<Trade>& trades() const { return trades_; }
    void add_trade(const Trade& trade) { trades_.push_back(trade); }

    using BidLevels = std::map<Price, std::deque<Order>, std::greater<Price>>;
    using AskLevels = std::map<Price, std::deque<Order>>;

    const BidLevels& bids() const { return bids_; }
    const AskLevels& asks() const { return asks_; }

private:
    BidLevels bids_;
    AskLevels asks_;
    std::unordered_map<OrderId, OrderLocation> index_;
    std::vector<Trade> trades_;
    uint64_t sequence_{0};

    void remove_from_level(Side side, Price price, size_t idx);
    std::deque<Order>& level_deque(Side side, Price price);
};

}  // namespace fastexchange
