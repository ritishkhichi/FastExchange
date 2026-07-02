#include "metrics/metrics.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fastexchange {

const std::vector<double> MetricsEngine::kDefaultBuckets = {
    1, 2, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000};

void MetricsEngine::counter(const std::string& name, uint64_t delta) {
    std::lock_guard lock(mutex_);
    counters_[name] += delta;
}

void MetricsEngine::gauge(const std::string& name, double value) {
    std::lock_guard lock(mutex_);
    gauges_[name] = value;
}

void MetricsEngine::histogram(const std::string& name, double value_us) {
    std::lock_guard lock(mutex_);
    histograms_[name].push_back(value_us);
}

void MetricsEngine::set_scenario(const std::string& name) {
    std::lock_guard lock(mutex_);
    scenario_ = name;
}

void MetricsEngine::set_throughput(double orders_per_sec, double trades_per_sec) {
    std::lock_guard lock(mutex_);
    orders_per_sec_ = orders_per_sec;
    trades_per_sec_ = trades_per_sec;
}

void MetricsEngine::reset() {
    std::lock_guard lock(mutex_);
    counters_.clear();
    gauges_.clear();
    histograms_.clear();
    orders_per_sec_ = 0;
    trades_per_sec_ = 0;
}

double MetricsEngine::percentile(const std::vector<double>& sorted, double p) {
    if (sorted.empty()) return 0;
    if (sorted.size() == 1) return sorted[0];
    double idx = p * (sorted.size() - 1);
    size_t lo = static_cast<size_t>(idx);
    size_t hi = std::min(lo + 1, sorted.size() - 1);
    double frac = idx - lo;
    return sorted[lo] * (1 - frac) + sorted[hi] * frac;
}

nlohmann::json MetricsEngine::snapshot() const {
    std::lock_guard lock(mutex_);
    nlohmann::json j;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    j["scenario"] = scenario_;
    j["counters"] = counters_;
    j["gauges"] = gauges_;

    nlohmann::json hist;
    for (const auto& [name, values] : histograms_) {
        if (values.empty()) continue;
        auto sorted = values;
        std::sort(sorted.begin(), sorted.end());
        double sum = 0;
        for (double v : sorted) sum += v;
        hist[name] = {{"count", sorted.size()},
                      {"sum", sum},
                      {"p50", percentile(sorted, 0.50)},
                      {"p95", percentile(sorted, 0.95)},
                      {"p99", percentile(sorted, 0.99)},
                      {"max", sorted.back()}};
    }
    j["histograms"] = hist;
    j["throughput"] = {{"orders_per_sec", orders_per_sec_}, {"trades_per_sec", trades_per_sec_}};
    return j;
}

void MetricsEngine::save_to_file(const std::string& path) const {
    std::ofstream out(path);
    out << snapshot().dump(2);
}

}  // namespace fastexchange
