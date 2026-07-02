#pragma once

#include "matching/matching_plugin.hpp"

namespace fastexchange {

class PriceTimeFifoMatcher : public IMatchingPlugin {
public:
    MatchResult match(OrderBook& book, Order incoming) override;
};

}  // namespace fastexchange
