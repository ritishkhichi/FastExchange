#include "metrics/system_metrics.hpp"
#include <chrono>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#else
#include <chrono>
#include <fstream>
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace fastexchange {

namespace {

#if defined(_WIN32)
ULARGE_INTEGER file_time_to_large_integer(const FILETIME& ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli;
}
#endif

struct SampleState {
    bool initialized{false};
    std::chrono::steady_clock::time_point last_time;
#if defined(_WIN32)
    ULONGLONG last_kernel{0};
    ULONGLONG last_user{0};
#else
    clock_t last_cpu_ticks{0};
#endif
};

SampleState& state() {
    static SampleState s;
    return s;
}

}  // namespace

SystemMetrics sample_system_metrics() {
    SystemMetrics m;
    auto& s = state();
    auto now = std::chrono::steady_clock::now();

#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        m.rss_mb = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }

    FILETIME create_time, exit_time, kernel_time, user_time;
    if (GetProcessTimes(GetCurrentProcess(), &create_time, &exit_time, &kernel_time, &user_time)) {
        auto kernel = file_time_to_large_integer(kernel_time).QuadPart;
        auto user = file_time_to_large_integer(user_time).QuadPart;
        auto total = kernel + user;

        if (s.initialized) {
            auto wall_us = std::chrono::duration_cast<std::chrono::microseconds>(now - s.last_time).count();
            auto cpu_delta = total - (s.last_kernel + s.last_user);
            if (wall_us > 0) {
                // FILETIME is in 100-ns units
                m.cpu_percent = (static_cast<double>(cpu_delta) / static_cast<double>(wall_us * 10)) * 100.0;
                if (m.cpu_percent > 100.0) m.cpu_percent = 100.0;
            }
        }

        s.last_kernel = kernel;
        s.last_user = user;
    }
#else
    std::ifstream statm("/proc/self/statm");
    long pages = 0;
    long rss_pages = 0;
    statm >> pages >> rss_pages;
    if (rss_pages > 0) {
        long page_size = sysconf(_SC_PAGESIZE);
        m.rss_mb = static_cast<double>(rss_pages * page_size) / (1024.0 * 1024.0);
    }

    struct rusage usage {};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        clock_t cpu_ticks = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec +
                            usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;
        if (s.initialized) {
            auto wall_us = std::chrono::duration_cast<std::chrono::microseconds>(now - s.last_time).count();
            auto cpu_delta = cpu_ticks - s.last_cpu_ticks;
            if (wall_us > 0) {
                m.cpu_percent = (static_cast<double>(cpu_delta) / static_cast<double>(wall_us)) * 100.0;
                if (m.cpu_percent > 100.0) m.cpu_percent = 100.0;
            }
        }
        s.last_cpu_ticks = cpu_ticks;
    }
#endif

    s.last_time = now;
    s.initialized = true;
    return m;
}

}  // namespace fastexchange
