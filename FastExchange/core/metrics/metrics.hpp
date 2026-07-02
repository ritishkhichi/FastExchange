#pragma once

#include <nlohmann/json_fwd.hpp>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace fastexchange {

class MetricsEngine {
public:
    static const std::vector<double> kDefaultBuckets;

    void counter(const std::string& name, uint64_t delta = 1);
    void gauge(const std::string& name, double value);
    void histogram(const std::string& name, double value_us);

    void set_scenario(const std::string& name);
    void set_throughput(double orders_per_sec, double trades_per_sec);
    void reset();

    nlohmann::json snapshot() const;
    void save_to_file(const std::string& path) const;

private:
    mutable std::mutex mutex_;
    std::map<std::string, uint64_t> counters_;
    std::map<std::string, double> gauges_;
    std::map<std::string, std::vector<double>> histograms_;
    std::string scenario_;
    double orders_per_sec_{0};
    double trades_per_sec_{0};

    static double percentile(const std::vector<double>& sorted, double p);
};

}  // namespace fastexchange
