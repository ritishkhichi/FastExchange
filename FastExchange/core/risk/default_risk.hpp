#pragma once

#include "risk/risk_plugin.hpp"
#include <unordered_set>

namespace fastexchange {

class DefaultRiskPlugin : public IRiskPlugin {
public:
    RiskResult check(const InboundEvent& event, const SymbolRegistry& registry,
                     uint64_t max_order_size, bool auto_register) override;
    void reset() { seen_order_ids_.clear(); }

private:
    std::unordered_set<OrderId> seen_order_ids_;
};

}  // namespace fastexchange
