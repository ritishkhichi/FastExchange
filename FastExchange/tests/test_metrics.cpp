#include "metrics/metrics.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

TEST(MetricsTest, SnapshotContainsExpectedKeys) {
    fastexchange::MetricsEngine metrics;
    metrics.set_scenario("test");
    metrics.counter("orders.submitted", 100);
    metrics.counter("trades.total", 50);
    metrics.histogram("latency.order.total", 4.5);
    metrics.histogram("latency.order.total", 10.2);
    metrics.gauge("memory.rss_mb", 91.0);
    metrics.set_throughput(1500000, 900000);

    auto snap = metrics.snapshot();
    EXPECT_EQ(snap["scenario"], "test");
    EXPECT_EQ(snap["counters"]["orders.submitted"], 100);
    EXPECT_EQ(snap["counters"]["trades.total"], 50);
    EXPECT_TRUE(snap.contains("histograms"));
    EXPECT_TRUE(snap["histograms"].contains("latency.order.total"));
    EXPECT_DOUBLE_EQ(snap["throughput"]["orders_per_sec"], 1500000);
}
