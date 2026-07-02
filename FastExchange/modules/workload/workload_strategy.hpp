#pragma once

#include "config/config_loader.hpp"
#include "types/inbound_event.hpp"
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace fastexchange {

class IWorkloadStrategy {
public:
    virtual ~IWorkloadStrategy() = default;
    virtual void configure(const WorkloadConfig& workload, uint64_t seed,
                           const std::vector<std::string>& symbols,
                           const std::unordered_map<std::string, double>& mid_prices,
                           double tick_size) = 0;
    virtual std::optional<InboundEvent> next(uint64_t sim_time) = 0;
    virtual bool has_more() const = 0;
    virtual void set_order_limit(int limit) { order_limit_ = limit; }

protected:
    int order_limit_{0};
    int orders_generated_{0};
};

using StrategyCreator = std::unique_ptr<IWorkloadStrategy> (*)();

class WorkloadFactory {
public:
    static WorkloadFactory& instance();
    void register_strategy(const std::string& name, StrategyCreator creator);
    std::unique_ptr<IWorkloadStrategy> create(const std::string& name) const;
    std::vector<std::string> names() const;

private:
    std::unordered_map<std::string, StrategyCreator> creators_;
};

#define REGISTER_STRATEGY(name, Class)                                           \
    static std::unique_ptr<IWorkloadStrategy> factory_##Class() {                \
        return std::make_unique<Class>();                                        \
    }                                                                              \
    static bool _reg_##Class = ([]() {                                            \
        ::fastexchange::WorkloadFactory::instance().register_strategy(             \
            name, factory_##Class);                                                \
        return true;                                                               \
    })()

}  // namespace fastexchange
