#pragma once

#include "types/order.hpp"

namespace fastexchange {

struct Trade {
    OrderId buy_order_id{0};
    OrderId sell_order_id{0};
    Symbol symbol;
    Price price{0};
    Quantity quantity{0};
    Timestamp timestamp{0};
};

}  // namespace fastexchange
