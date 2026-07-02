#include "workload/base_strategy.hpp"
#include <cmath>

namespace fastexchange {

void BaseWorkloadStrategy::configure(const WorkloadConfig& workload, uint64_t seed,
                                     const std::vector<std::string>& symbols,
                                     const std::unordered_map<std::string, double>& mid_prices,
                                     double tick_size) {
    workload_ = workload;
    symbols_ = symbols;
    tick_size_ = tick_size;
    rng_.seed(seed);
    next_order_id_ = 1;
    live_orders_.clear();
    orders_generated_ = 0;
    mids_.clear();
    for (const auto& sym : symbols_) {
        double mid = 100.0;
        if (mid_prices.contains(sym)) mid = mid_prices.at(sym);
        mids_[sym] = to_ticks(mid, tick_size_);
    }
}

bool BaseWorkloadStrategy::has_more() const {
    if (order_limit_ > 0 && orders_generated_ >= order_limit_) return false;
    return true;
}

Symbol BaseWorkloadStrategy::pick_symbol() {
    std::uniform_int_distribution<size_t> dist(0, symbols_.size() - 1);
    return symbols_[dist(rng_)];
}

Side BaseWorkloadStrategy::pick_side(double buy_prob) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng_) < buy_prob ? Side::Buy : Side::Sell;
}

Price BaseWorkloadStrategy::pick_price(const Symbol& sym, int bias_ticks) {
    std::normal_distribution<double> dist(0.0, 5.0);
    Price mid = mids_[sym];
    Price offset = static_cast<Price>(dist(rng_)) + bias_ticks;
    if (offset == 0) offset = 1;
    return mid + offset;
}

Quantity BaseWorkloadStrategy::pick_quantity() {
    if (workload_.quantity_distribution == "uniform") {
        std::uniform_int_distribution<int> dist(workload_.quantity_min, workload_.quantity_max);
        return static_cast<Quantity>(dist(rng_));
    }
    std::poisson_distribution<int> dist(workload_.quantity_lambda);
    int q = dist(rng_);
    q = std::max(workload_.quantity_min, std::min(workload_.quantity_max, q));
    return static_cast<Quantity>(q);
}

InboundEvent BaseWorkloadStrategy::make_submit(uint64_t sim_time, Side side, const Symbol& sym,
                                               Price price, Quantity qty) {
    Order o;
    o.id = next_order_id_++;
    o.symbol = sym;
    o.side = side;
    o.price = price;
    o.quantity = qty;
    o.remaining = qty;
    o.timestamp = sim_time;
    live_orders_.push_back(o.id);
    ++orders_generated_;
    return OrderSubmit{o};
}

InboundEvent BaseWorkloadStrategy::maybe_cancel_or_modify(uint64_t sim_time) {
    if (live_orders_.empty()) {
        auto sym = pick_symbol();
        auto side = pick_side(workload_.buy_probability);
        return make_submit(sim_time, side, sym, pick_price(sym, workload_.price_bias_ticks),
                           pick_quantity());
    }

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r = dist(rng_);

    if (r < workload_.cancel_probability) {
        std::uniform_int_distribution<size_t> idx_dist(0, live_orders_.size() - 1);
        size_t idx = idx_dist(rng_);
        OrderId id = live_orders_[idx];
        live_orders_.erase(live_orders_.begin() + static_cast<std::ptrdiff_t>(idx));
        Symbol sym = pick_symbol();
        return OrderCancel{id, sym};
    }

    if (r < workload_.cancel_probability + workload_.modify_probability) {
        std::uniform_int_distribution<size_t> idx_dist(0, live_orders_.size() - 1);
        size_t idx = idx_dist(rng_);
        OrderId id = live_orders_[idx];
        Symbol sym = pick_symbol();
        OrderModify m{id, sym, pick_price(sym, 0), pick_quantity()};
        return m;
    }

    auto sym = pick_symbol();
    auto side = pick_side(workload_.buy_probability);
    return make_submit(sim_time, side, sym, pick_price(sym, workload_.price_bias_ticks),
                       pick_quantity());
}

}  // namespace fastexchange
