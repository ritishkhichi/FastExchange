# Plugin Architecture

## Design

FastExchange uses compile-time plugin registration. Users extend behavior by implementing interfaces and registering with macros — no `.so`/`.dll` loading in v1.

## IMatchingPlugin

```cpp
struct MatchResult {
    std::vector<Trade> trades;
    std::optional<Order> remainder;  // unfilled portion to rest on book
};

class IMatchingPlugin {
public:
    virtual ~IMatchingPlugin() = default;
    virtual MatchResult match(OrderBook& book, Order& incoming) = 0;
};
```

**v1 default:** `PriceTimeFifoMatcher` — price-time priority, partial fills, FIFO at each level.

## IRiskPlugin

```cpp
struct RiskResult {
    bool passed;
    std::string reason;
};

class IRiskPlugin {
public:
    virtual ~IRiskPlugin() = default;
    virtual RiskResult check(const InboundEvent& event,
                             const SymbolRegistry& registry) = 0;
};
```

**v1 default:** `DefaultRiskPlugin`
- Reject zero/negative quantity
- Reject duplicate order ID
- Reject quantity > `max_order_size`
- Reject unknown symbol (unless auto-register enabled)

## IWorkloadStrategy

```cpp
class IWorkloadStrategy {
public:
    virtual ~IWorkloadStrategy() = default;
    virtual void configure(const YAML::Node& workload,
                           uint64_t seed,
                           const std::vector<Symbol>& symbols) = 0;
    virtual std::optional<InboundEvent> next(uint64_t sim_time) = 0;
    virtual bool has_more() const = 0;
};
```

## WorkloadFactory

```cpp
class WorkloadFactory {
public:
    static WorkloadFactory& instance();
    void register_strategy(const std::string& name, StrategyCreator creator);
    std::unique_ptr<IWorkloadStrategy> create(const std::string& name) const;
};

#define REGISTER_STRATEGY(name, Class) \
    static bool _reg_##Class = (WorkloadFactory::instance().register_strategy( \
        name, []() { return std::make_unique<Class>(); }), true)
```

## Built-in Strategies (v1)

| Name | Description |
|------|-------------|
| `balanced` | 50/50 buy/sell, normal arrival |
| `bull` | Buy-heavy, positive price bias |
| `bear` | Sell-heavy, negative price bias |
| `flash-crash` | High rate, 90%+ sells |
| `hft` | Tiny orders, high rate, high cancel/modify |
| `market-maker` | Spread maintenance, frequent replace |
| `stress` | Maximum rate and boundary quantities |
| `replay` | Reads events from existing journal |

## v2 Roadmap

Dynamic plugin loading via shared libraries and a stable C ABI for risk/matching strategies.
