#include "risk/default_risk.hpp"
#include <sstream>

namespace fastexchange {

RiskResult DefaultRiskPlugin::check(const InboundEvent& event, const SymbolRegistry& registry,
                                    uint64_t max_order_size, bool auto_register) {
    RiskResult result;

    if (std::holds_alternative<OrderSubmit>(event)) {
        const auto& submit = std::get<OrderSubmit>(event);
        const auto& o = submit.order;

        if (o.quantity == 0) {
            result.passed = false;
            result.reason = "zero quantity";
            return result;
        }
        if (o.quantity > max_order_size) {
            result.passed = false;
            result.reason = "exceeds max order size";
            return result;
        }
        if (!registry.has_symbol(o.symbol) && !auto_register) {
            result.passed = false;
            result.reason = "unknown symbol";
            return result;
        }
        if (seen_order_ids_.contains(o.id)) {
            result.passed = false;
            result.reason = "duplicate order id";
            return result;
        }
        seen_order_ids_.insert(o.id);
    } else if (std::holds_alternative<OrderCancel>(event)) {
        const auto& c = std::get<OrderCancel>(event);
        if (!registry.has_symbol(c.symbol)) {
            result.passed = false;
            result.reason = "unknown symbol";
            return result;
        }
    } else if (std::holds_alternative<OrderModify>(event)) {
        const auto& m = std::get<OrderModify>(event);
        if (!registry.has_symbol(m.symbol)) {
            result.passed = false;
            result.reason = "unknown symbol";
            return result;
        }
        if (m.new_quantity && *m.new_quantity == 0) {
            result.passed = false;
            result.reason = "zero quantity";
            return result;
        }
    }

    return result;
}

}  // namespace fastexchange
