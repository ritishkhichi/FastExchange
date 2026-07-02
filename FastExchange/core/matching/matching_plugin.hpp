#pragma once

#include "orderbook/order_book.hpp"
#include <vector>

namespace fastexchange {

struct MatchResult {
    std::vector<Trade> trades;
    std::optional<Order> remainder;
};

class IMatchingPlugin {
public:
    virtual ~IMatchingPlugin() = default;
    virtual MatchResult match(OrderBook& book, Order incoming) = 0;
};

}  // namespace fastexchange
