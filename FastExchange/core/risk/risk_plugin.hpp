#pragma once

#include "symbols/symbol_registry.hpp"
#include "types/inbound_event.hpp"
#include <string>

namespace fastexchange {

struct RiskResult {
    bool passed{true};
    std::string reason;
};

class IRiskPlugin {
public:
    virtual ~IRiskPlugin() = default;
    virtual RiskResult check(const InboundEvent& event, const SymbolRegistry& registry,
                             uint64_t max_order_size, bool auto_register) = 0;
};

}  // namespace fastexchange
