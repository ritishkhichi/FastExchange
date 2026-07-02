#pragma once

#include "orderbook/order_book.hpp"
#include "types/types.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace fastexchange {

class SymbolRegistry {
public:
    void register_symbol(const Symbol& symbol, Price initial_mid = 0);
    bool has_symbol(const Symbol& symbol) const;
    OrderBook& get_book(const Symbol& symbol);
    const OrderBook& get_book(const Symbol& symbol) const;
    const std::vector<Symbol>& symbols() const { return symbols_; }

private:
    std::vector<Symbol> symbols_;
    std::unordered_map<Symbol, std::unique_ptr<OrderBook>> books_;
};

}  // namespace fastexchange
