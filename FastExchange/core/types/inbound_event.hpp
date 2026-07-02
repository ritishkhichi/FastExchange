#pragma once

#include "types/order.hpp"
#include <optional>
#include <string>
#include <variant>

namespace fastexchange {

enum class InboundType : uint8_t {
    Submit = 0,
    Cancel = 1,
    Modify = 2,
};

struct OrderSubmit {
    Order order;
};

struct OrderCancel {
    OrderId order_id;
    Symbol symbol;
};

struct OrderModify {
    OrderId order_id;
    Symbol symbol;
    std::optional<Price> new_price;
    std::optional<Quantity> new_quantity;
};

using InboundEvent = std::variant<OrderSubmit, OrderCancel, OrderModify>;

}  // namespace fastexchange
