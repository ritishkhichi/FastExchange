# Metrics Pipeline

## Design

All observability flows through `MetricsEngine`. Benchmark, CLI, profiler, and dashboard are consumers — not separate instrumentation paths.

## API

```cpp
class MetricsEngine {
public:
    void counter(const std::string& name, uint64_t delta = 1);
    void gauge(const std::string& name, double value);
    void histogram(const std::string& name, double value_us);
    nlohmann::json snapshot() const;
    void reset();
};
```

## Standard Metrics (v1)

### Counters
| Name | Description |
|------|-------------|
| `orders.submitted` | Total orders submitted |
| `orders.accepted` | Orders passing risk |
| `orders.rejected` | Orders failing risk |
| `orders.cancelled` | Cancelled orders |
| `trades.total` | Executed trades |
| `events.total` | Journal events written |

### Histograms (microseconds)
| Name | Description |
|------|-------------|
| `latency.order.total` | End-to-end order processing |
| `latency.stage.receive` | Ingress |
| `latency.stage.risk` | Risk check |
| `latency.stage.match` | Matching |
| `latency.stage.journal` | Journal append |

Histogram buckets: `[1, 2, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000]` μs

### Gauges
| Name | Description |
|------|-------------|
| `book.depth.bid` | Total bid depth (all symbols) |
| `book.depth.ask` | Total ask depth |
| `book.spread` | Best bid-ask spread (primary symbol) |
| `queue.length` | Pending inbound queue size |
| `memory.rss_mb` | Resident set size |
| `cpu.percent` | CPU utilization |

## MetricsSnapshot JSON Schema

```json
{
  "timestamp": 1700000000000,
  "scenario": "flash-crash",
  "counters": {
    "orders.submitted": 2000000,
    "trades.total": 1420000
  },
  "histograms": {
    "latency.order.total": {
      "count": 2000000,
      "sum": 8400000.0,
      "p50": 2.9,
      "p95": 5.8,
      "p99": 10.2,
      "max": 45.0
    }
  },
  "gauges": {
    "memory.rss_mb": 91.0,
    "cpu.percent": 84.0
  },
  "throughput": {
    "orders_per_sec": 1820000,
    "trades_per_sec": 1290000
  }
}
```

## Consumers

| Consumer | Reads |
|----------|-------|
| `fastexchange stats` | Metrics JSON file |
| `fastexchange compare` | Two metrics JSON files |
| `fastexchange benchmark` | Live snapshot after run |
| Dashboard WebSocket | `GET /metrics` or WS `/feed` |
| Profiler module | Stage histograms |
