#include "symbols/symbol_registry.hpp"
#include <stdexcept>

namespace fastexchange {

void SymbolRegistry::register_symbol(const Symbol& symbol, Price /*initial_mid*/) {
    if (books_.contains(symbol)) return;
    symbols_.push_back(symbol);
    books_[symbol] = std::make_unique<OrderBook>();
}

bool SymbolRegistry::has_symbol(const Symbol& symbol) const {
    return books_.contains(symbol);
}

OrderBook& SymbolRegistry::get_book(const Symbol& symbol) {
    auto it = books_.find(symbol);
    if (it == books_.end()) {
        throw std::runtime_error("unknown symbol: " + symbol);
    }
    return *it->second;
}

const OrderBook& SymbolRegistry::get_book(const Symbol& symbol) const {
    return const_cast<SymbolRegistry*>(this)->get_book(symbol);
}

}  // namespace fastexchange
