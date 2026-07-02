#pragma once

namespace fastexchange {

struct SystemMetrics {
    double rss_mb{0};
    double cpu_percent{0};
};

// Samples process RSS and CPU usage since the previous call.
SystemMetrics sample_system_metrics();

}  // namespace fastexchange
