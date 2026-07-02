#pragma once

#include "workload/workload_strategy.hpp"
#include <random>

namespace fastexchange {

class BaseWorkloadStrategy : public IWorkloadStrategy {
public:
    void configure(const WorkloadConfig& workload, uint64_t seed,
                   const std::vector<std::string>& symbols,
                   const std::unordered_map<std::string, double>& mid_prices,
                   double tick_size) override;

    bool has_more() const override;

protected:
    WorkloadConfig workload_;
    std::vector<std::string> symbols_;
    std::unordered_map<std::string, Price> mids_;
    double tick_size_{0.0001};
    std::mt19937_64 rng_;
    OrderId next_order_id_{1};
    std::vector<OrderId> live_orders_;

    Symbol pick_symbol();
    Side pick_side(double buy_prob);
    Price pick_price(const Symbol& sym, int bias_ticks);
    Quantity pick_quantity();
    InboundEvent make_submit(uint64_t sim_time, Side side, const Symbol& sym, Price price,
                             Quantity qty);
    InboundEvent maybe_cancel_or_modify(uint64_t sim_time);
};

}  // namespace fastexchange
