#pragma once

#include "workload/workload_strategy.hpp"
#include <vector>

namespace fastexchange {

class ReplayStrategy : public IWorkloadStrategy {
public:
    void configure(const WorkloadConfig& workload, uint64_t seed,
                   const std::vector<std::string>& symbols,
                   const std::unordered_map<std::string, double>& mid_prices,
                   double tick_size) override;

    void load_journal(const std::string& path);
    std::optional<InboundEvent> next(uint64_t sim_time) override;
    bool has_more() const override;

private:
    std::vector<InboundEvent> events_;
    size_t index_{0};
};

}  // namespace fastexchange
