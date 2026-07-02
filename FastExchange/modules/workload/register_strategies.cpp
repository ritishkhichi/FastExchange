#include "workload/builtin_strategies.hpp"
#include "workload/register_strategies.hpp"
#include "workload/replay_strategy.hpp"
#include "workload/workload_strategy.hpp"

namespace fastexchange {

namespace {

std::unique_ptr<IWorkloadStrategy> make_balanced() { return std::make_unique<BalancedStrategy>(); }
std::unique_ptr<IWorkloadStrategy> make_bull() { return std::make_unique<BullStrategy>(); }
std::unique_ptr<IWorkloadStrategy> make_bear() { return std::make_unique<BearStrategy>(); }
std::unique_ptr<IWorkloadStrategy> make_flash_crash() { return std::make_unique<FlashCrashStrategy>(); }
std::unique_ptr<IWorkloadStrategy> make_hft() { return std::make_unique<HftStrategy>(); }
std::unique_ptr<IWorkloadStrategy> make_market_maker() { return std::make_unique<MarketMakerStrategy>(); }
std::unique_ptr<IWorkloadStrategy> make_stress() { return std::make_unique<StressStrategy>(); }
std::unique_ptr<IWorkloadStrategy> make_replay() { return std::make_unique<ReplayStrategy>(); }

}  // namespace

void register_workload_strategies() {
    static bool registered = false;
    if (registered) return;
    registered = true;

    auto& factory = WorkloadFactory::instance();
    factory.register_strategy("balanced", make_balanced);
    factory.register_strategy("bull", make_bull);
    factory.register_strategy("bear", make_bear);
    factory.register_strategy("flash-crash", make_flash_crash);
    factory.register_strategy("hft", make_hft);
    factory.register_strategy("market-maker", make_market_maker);
    factory.register_strategy("stress", make_stress);
    factory.register_strategy("replay", make_replay);
}

}  // namespace fastexchange
