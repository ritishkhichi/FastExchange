#pragma once

#include <cstdint>
#include <string>

namespace fastexchange {

using Price = int64_t;
using Quantity = uint64_t;
using OrderId = uint64_t;
using Timestamp = uint64_t;
using Symbol = std::string;

enum class Side : uint8_t { Buy = 0, Sell = 1 };

enum class OrderType : uint8_t { Limit = 0 };

inline Price to_ticks(double display_price, double tick_size) {
    return static_cast<Price>(display_price / tick_size + 0.5);
}

inline double from_ticks(Price ticks, double tick_size) {
    return static_cast<double>(ticks) * tick_size;
}

}  // namespace fastexchange
