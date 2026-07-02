#include "orderbook/order_book.hpp"
#include <algorithm>

namespace fastexchange {

std::deque<Order>& OrderBook::level_deque(Side side, Price price) {
    if (side == Side::Buy) {
        return bids_[price];
    }
    return asks_[price];
}

void OrderBook::remove_from_level(Side side, Price price, size_t idx) {
    auto& dq = level_deque(side, price);
    dq.erase(dq.begin() + static_cast<std::ptrdiff_t>(idx));
    if (dq.empty()) {
        if (side == Side::Buy) {
            bids_.erase(price);
        } else {
            asks_.erase(price);
        }
    } else {
        for (size_t i = idx; i < dq.size(); ++i) {
            index_[dq[i].id] = {side, price, i};
        }
    }
}

bool OrderBook::add_order(Order order) {
    order.sequence = ++sequence_;
    order.remaining = order.quantity;
    Side side = order.side;
    Price price = order.price;
    auto& dq = level_deque(side, price);
    dq.push_back(order);
    index_[order.id] = {side, price, dq.size() - 1};
    return true;
}

bool OrderBook::cancel_order(OrderId id) {
    auto it = index_.find(id);
    if (it == index_.end()) return false;
    auto loc = it->second;
    remove_from_level(loc.side, loc.price, loc.index_in_deque);
    index_.erase(it);
    return true;
}

bool OrderBook::modify_order(OrderId id, Price new_price, Quantity new_qty) {
    auto it = index_.find(id);
    if (it == index_.end()) return false;
    auto loc = it->second;
    auto& dq = level_deque(loc.side, loc.price);
    Order order = dq[loc.index_in_deque];
    remove_from_level(loc.side, loc.price, loc.index_in_deque);
    index_.erase(it);
    order.price = new_price;
    order.quantity = new_qty;
    order.remaining = new_qty;
    return add_order(order);
}

std::optional<Order> OrderBook::pop_best_opposite(Side incoming_side) {
    if (incoming_side == Side::Buy) {
        if (asks_.empty()) return std::nullopt;
        auto it = asks_.begin();
        auto& dq = it->second;
        Order o = dq.front();
        dq.pop_front();
        if (dq.empty()) asks_.erase(it);
        else {
            for (size_t i = 0; i < dq.size(); ++i) {
                index_[dq[i].id] = {Side::Sell, o.price, i};
            }
        }
        index_.erase(o.id);
        return o;
    }
    if (bids_.empty()) return std::nullopt;
    auto it = bids_.begin();
    auto& dq = it->second;
    Order o = dq.front();
    dq.pop_front();
    if (dq.empty()) bids_.erase(it);
    else {
        for (size_t i = 0; i < dq.size(); ++i) {
            index_[dq[i].id] = {Side::Buy, o.price, i};
        }
    }
    index_.erase(o.id);
    return o;
}

void OrderBook::requeue_remainder(const Order& order) {
    Order o = order;
    add_order(o);
}

BookSnapshot OrderBook::snapshot() const {
    BookSnapshot snap;
    if (!bids_.empty()) snap.best_bid = bids_.begin()->first;
    if (!asks_.empty()) snap.best_ask = asks_.begin()->first;
    for (const auto& [price, dq] : bids_) {
        for (const auto& o : dq) snap.bid_depth += o.remaining;
    }
    for (const auto& [price, dq] : asks_) {
        for (const auto& o : dq) snap.ask_depth += o.remaining;
    }
    return snap;
}

}  // namespace fastexchange
