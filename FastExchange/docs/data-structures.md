# Data Structures

## Type Aliases

```cpp
using Price = int64_t;      // price in ticks (no floating point)
using Quantity = uint64_t;
using OrderId = uint64_t;
using Timestamp = uint64_t; // simulated nanoseconds
using Symbol = std::string;
```

### Tick Conversion

```
display_price = ticks * tick_size
ticks = round(display_price / tick_size)

Example: ₹105.1234 with tick_size 0.0001 → 1051234 ticks
```

## Order

```cpp
struct Order {
    OrderId id;
    Symbol symbol;
    Side side;           // Buy | Sell
    Price price;
    Quantity quantity;
    Quantity remaining;
    Timestamp timestamp;
    uint64_t sequence;   // FIFO tie-breaker within price level
};
```

## OrderBook (per symbol)

```cpp
class OrderBook {
    // Bids: highest price first (best bid = begin())
    std::map<Price, std::deque<Order>, std::greater<Price>> bids_;
    // Asks: lowest price first (best ask = begin())
    std::map<Price, std::deque<Order>> asks_;
    std::unordered_map<OrderId, OrderLocation> index_;  // O(1) cancel/modify
};
```

### Why `std::map` + `std::deque`?

| Choice | Rationale |
|--------|-----------|
| `std::map<Price, ...>` | O(log n) insert/erase per price level; sorted iteration for best bid/ask |
| `std::deque<Order>` | O(1) push/pop at ends; FIFO within price level |
| `unordered_map<OrderId, ...>` | O(1) cancel/modify lookup |

Alternative considered: flat sorted vector — faster cache but expensive inserts. Map+deque is the standard exchange textbook approach and sufficient for v1 benchmarks.

## SymbolRegistry

```cpp
class SymbolRegistry {
    std::unordered_map<Symbol, std::unique_ptr<OrderBook>> books_;
    std::vector<Symbol> symbols_;  // registration order
public:
    void register_symbol(const Symbol& sym, Price initial_mid = 0);
    OrderBook& get_book(const Symbol& sym);
    bool has_symbol(const Symbol& sym) const;
    const std::vector<Symbol>& symbols() const;
};
```

Internally `unordered_map`, but exposed via registry API so YAML-defined symbols require no code changes.

## Price-Time Priority

1. **Price:** Buy matches lowest ask ≤ buy price; sell matches highest bid ≥ sell price
2. **Time:** Within same price level, FIFO via `deque` front-to-back
3. **Partial fill:** Reduce `remaining`; re-queue remainder at same price if limit not fully filled

## Multi-Asset Isolation

Each symbol has independent `OrderBook`. Matching never crosses symbols. `SymbolRegistry` routes events to the correct book.
