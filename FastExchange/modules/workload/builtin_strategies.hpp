#pragma once

#include "workload/base_strategy.hpp"

namespace fastexchange {

class BalancedStrategy : public BaseWorkloadStrategy {
public:
    std::optional<InboundEvent> next(uint64_t sim_time) override;
};

class BullStrategy : public BaseWorkloadStrategy {
public:
    std::optional<InboundEvent> next(uint64_t sim_time) override;
};

class BearStrategy : public BaseWorkloadStrategy {
public:
    std::optional<InboundEvent> next(uint64_t sim_time) override;
};

class FlashCrashStrategy : public BaseWorkloadStrategy {
public:
    std::optional<InboundEvent> next(uint64_t sim_time) override;
};

class HftStrategy : public BaseWorkloadStrategy {
public:
    std::optional<InboundEvent> next(uint64_t sim_time) override;
};

class MarketMakerStrategy : public BaseWorkloadStrategy {
public:
    std::optional<InboundEvent> next(uint64_t sim_time) override;
};

class StressStrategy : public BaseWorkloadStrategy {
public:
    std::optional<InboundEvent> next(uint64_t sim_time) override;
};

}  // namespace fastexchange
