#include "workload/register_strategies.hpp"
#include "workload/replay_strategy.hpp"
#include "event/event_codec.hpp"
#include "eventlog/journal.hpp"

namespace fastexchange {

void ReplayStrategy::configure(const WorkloadConfig& /*workload*/, uint64_t /*seed*/,
                               const std::vector<std::string>& /*symbols*/,
                               const std::unordered_map<std::string, double>& /*mid_prices*/,
                               double /*tick_size*/) {
    events_.clear();
    index_ = 0;
}

void ReplayStrategy::load_journal(const std::string& path) {
    events_.clear();
    index_ = 0;
    auto journal_events = EventJournal::read_from_file(path);
    for (const auto& e : journal_events) {
        if (e.type == EventType::OrderSubmitRequested) {
            const auto& o = std::get<OrderPayload>(e.payload).order;
            events_.push_back(OrderSubmit{o});
        } else if (e.type == EventType::OrderCancelRequested) {
            const auto& c = std::get<CancelPayload>(e.payload);
            events_.push_back(OrderCancel{c.order_id, c.symbol});
        } else if (e.type == EventType::OrderModifyRequested) {
            const auto& m = std::get<ModifyPayload>(e.payload);
            OrderModify om{m.order_id, m.symbol, m.new_price, m.new_quantity};
            events_.push_back(om);
        }
    }
}

std::optional<InboundEvent> ReplayStrategy::next(uint64_t /*sim_time*/) {
    if (index_ >= events_.size()) return std::nullopt;
    return events_[index_++];
}

bool ReplayStrategy::has_more() const {
    return index_ < events_.size();
}

}  // namespace fastexchange
