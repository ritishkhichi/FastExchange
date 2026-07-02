#include <algorithm>
#include "matching/price_time_fifo.hpp"

namespace fastexchange {

MatchResult PriceTimeFifoMatcher::match(OrderBook& book, Order incoming) {
    MatchResult result;
    incoming.remaining = incoming.quantity;

    while (incoming.remaining > 0) {
        Price best_opp = 0;
        bool can_match = false;

        if (incoming.side == Side::Buy) {
            if (!book.asks().empty()) {
                best_opp = book.asks().begin()->first;
                can_match = best_opp <= incoming.price;
            }
        } else {
            if (!book.bids().empty()) {
                best_opp = book.bids().begin()->first;
                can_match = best_opp >= incoming.price;
            }
        }

        if (!can_match) break;

        auto resting = book.pop_best_opposite(incoming.side);
        if (!resting) break;

        Quantity fill_qty = std::min(incoming.remaining, resting->remaining);
        Price trade_price = resting->price;

        Trade trade;
        if (incoming.side == Side::Buy) {
            trade.buy_order_id = incoming.id;
            trade.sell_order_id = resting->id;
        } else {
            trade.buy_order_id = resting->id;
            trade.sell_order_id = incoming.id;
        }
        trade.symbol = incoming.symbol;
        trade.price = trade_price;
        trade.quantity = fill_qty;
        trade.timestamp = incoming.timestamp;
        result.trades.push_back(trade);
        book.add_trade(trade);

        incoming.remaining -= fill_qty;
        resting->remaining -= fill_qty;

        if (resting->remaining > 0) {
            book.requeue_remainder(*resting);
        }
    }

    if (incoming.remaining > 0) {
        incoming.quantity = incoming.remaining;
        result.remainder = incoming;
    }

    return result;
}

}  // namespace fastexchange
