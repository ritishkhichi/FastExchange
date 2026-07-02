#include "workload/workload_strategy.hpp"
#include <stdexcept>
#include <unordered_map>

namespace fastexchange {

WorkloadFactory& WorkloadFactory::instance() {
    static WorkloadFactory factory;
    return factory;
}

void WorkloadFactory::register_strategy(const std::string& name, StrategyCreator creator) {
    creators_[name] = creator;
}

std::unique_ptr<IWorkloadStrategy> WorkloadFactory::create(const std::string& name) const {
    auto it = creators_.find(name);
    if (it == creators_.end()) {
        throw std::runtime_error("unknown workload strategy: " + name);
    }
    return it->second();
}

std::vector<std::string> WorkloadFactory::names() const {
    std::vector<std::string> result;
    for (const auto& [name, _] : creators_) {
        result.push_back(name);
    }
    return result;
}

}  // namespace fastexchange
