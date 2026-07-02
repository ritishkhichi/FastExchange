#include "workload/builtin_strategies.hpp"

namespace fastexchange {

std::optional<InboundEvent> BalancedStrategy::next(uint64_t sim_time) {
    if (!has_more()) return std::nullopt;
    return maybe_cancel_or_modify(sim_time);
}

std::optional<InboundEvent> BullStrategy::next(uint64_t sim_time) {
    if (!has_more()) return std::nullopt;
    workload_.buy_probability = 0.75;
    workload_.sell_probability = 0.25;
    workload_.price_bias_ticks = 3;
    return maybe_cancel_or_modify(sim_time);
}

std::optional<InboundEvent> BearStrategy::next(uint64_t sim_time) {
    if (!has_more()) return std::nullopt;
    workload_.buy_probability = 0.25;
    workload_.sell_probability = 0.75;
    workload_.price_bias_ticks = -3;
    return maybe_cancel_or_modify(sim_time);
}

std::optional<InboundEvent> FlashCrashStrategy::next(uint64_t sim_time) {
    if (!has_more()) return std::nullopt;
    workload_.buy_probability = 0.10;
    workload_.sell_probability = 0.90;
    workload_.price_bias_ticks = -25;
    workload_.cancel_probability = 0.30;
    return maybe_cancel_or_modify(sim_time);
}

std::optional<InboundEvent> HftStrategy::next(uint64_t sim_time) {
    if (!has_more()) return std::nullopt;
    workload_.quantity_min = 1;
    workload_.quantity_max = 5;
    workload_.cancel_probability = 0.20;
    workload_.modify_probability = 0.15;
    return maybe_cancel_or_modify(sim_time);
}

std::optional<InboundEvent> MarketMakerStrategy::next(uint64_t sim_time) {
    if (!has_more()) return std::nullopt;
    auto sym = pick_symbol();
    Price mid = mids_[sym];
    std::uniform_int_distribution<int> spread_dist(1, 3);
    int spread = spread_dist(rng_);
    std::uniform_int_distribution<int> coin(0, 1);

    if (coin(rng_) == 0) {
        return make_submit(sim_time, Side::Buy, sym, mid - spread, 5 + (next_order_id_ % 10));
    }
    return make_submit(sim_time, Side::Sell, sym, mid + spread, 5 + (next_order_id_ % 10));
}

std::optional<InboundEvent> StressStrategy::next(uint64_t sim_time) {
    if (!has_more()) return std::nullopt;
    workload_.quantity_max = workload_.quantity_max > 0 ? workload_.quantity_max : 100;
    auto sym = pick_symbol();
    auto side = pick_side(0.5);
    return make_submit(sim_time, side, sym, pick_price(sym, 0), workload_.quantity_max);
}

}  // namespace fastexchange
