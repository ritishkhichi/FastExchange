#pragma once

#include "types/types.hpp"

namespace fastexchange {

struct Order {
    OrderId id{0};
    Symbol symbol;
    Side side{Side::Buy};
    OrderType type{OrderType::Limit};
    Price price{0};
    Quantity quantity{0};
    Quantity remaining{0};
    Timestamp timestamp{0};
    uint64_t sequence{0};
};

}  // namespace fastexchange
