# FastExchange

> **FastExchange is an event-driven exchange simulation framework for benchmarking, replaying, and profiling matching engine architectures under reproducible synthetic workloads.**

A high-performance **local exchange simulator** in C++20 — not another 200-line order book. Uses **synthetic fictional assets** (ABC, XYZ, TECH) only. No real market data, no broker APIs, no real money.

## Demo

![FastExchange live dashboard](docs/demo.gif)

*Live dashboard during a balanced workload scenario — synthetic orders, real-time order book, trades, and latency metrics (local simulation).*

## Features

- Event-sourced architecture (append-only `eventlog.bin`)
- Deterministic replay with SHA256 verification
- Reproducible benchmarks (P50 / P95 / P99 latency)
- YAML-driven scenario runner
- Compile-time plugin architecture (matching, risk, workload strategies)
- Metrics pipeline (CLI, API, dashboard)
- Multi-asset matching engine
- REST API (Crow) + Grafana-style web dashboard
- Workload strategies: balanced, bull, bear, flash-crash, hft, market-maker, stress, replay

## Tech Stack

| Layer | Technology |
|-------|------------|
| Core engine | C++20, CMake |
| Matching | Price-time FIFO, `std::map` + `std::deque` |
| Config | YAML (yaml-cpp) |
| CLI | CLI11 |
| API | Crow + ASIO |
| Dashboard | React, TypeScript, Vite, Recharts |
| Testing | Google Test |
| Benchmarking | Google Benchmark |

## Architecture

```
CLI / REST API / Dashboard
         |
  Scenario Runner (YAML)
         |
Workload Strategy (plugin)
         |
  Exchange Engine (single-threaded)
         |
  Matching | Risk | Event Journal
         |
    Metrics Engine
         |
 Benchmark | Replay | Dashboard
```

## Quick Start

### Prerequisites

- CMake 3.20+
- C++20 compiler (MSVC 2022, g++-14, or Clang)
- Node.js 18+ (dashboard only)

### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DFASTEXCHANGE_BUILD_API=ON
cmake --build build --config Release -j
```

### Run a scenario

```bash
# Windows
.\build\cli\Release\fastexchange.exe run scenarios\balanced.yaml
.\build\cli\Release\fastexchange.exe verify results\balanced.bin
.\build\cli\Release\fastexchange.exe stats results\balanced-metrics.json

# Linux / macOS
./build/fastexchange run scenarios/balanced.yaml
./build/fastexchange verify results/balanced.bin
```

### Benchmark

```bash
.\build\cli\Release\fastexchange.exe benchmark --scenario scenarios\balanced.yaml --orders 50000
.\build\cli\Release\fastexchange.exe benchmark --scenario scenarios\flash-crash.yaml --orders 50000
.\build\cli\Release\fastexchange.exe compare results\benchmark-balanced-metrics.json results\benchmark-flash-crash-metrics.json
```

**Sample results (Windows, Release build):**

| Metric | balanced | flash-crash |
|--------|----------|-------------|
| Throughput | ~89k orders/sec | ~85k orders/sec |
| P99 Latency | ~28 μs | ~32 μs |
| Trades | higher | lower (sell-heavy) |
| Rejected | 0 | 0 |

### Replay

```bash
.\build\cli\Release\fastexchange.exe replay results\balanced.bin --speed 10
.\build\cli\Release\fastexchange.exe replay results\balanced.bin --verify
```

### Live dashboard

**Terminal 1 — API (live simulation):**
```bash
.\build\api\Release\fastexchange_api.exe --scenario scenarios\balanced.yaml --orders 30000
```

**Terminal 2 — Dashboard:**
```bash
cd dashboard
npm install
npm run dev
```

Open **http://localhost:3000**

## CLI Reference

| Command | Description |
|---------|-------------|
| `run <scenario.yaml>` | Run a workload scenario |
| `benchmark --scenario <yaml> --orders N` | Throughput + latency benchmark |
| `compare <metrics-a.json> <metrics-b.json>` | Side-by-side comparison |
| `verify <eventlog.bin>` | SHA256 journal verification |
| `export <eventlog.bin> -o out.json` | Binary journal → JSON |
| `replay <eventlog.bin> [--verify]` | Replay event journal |
| `stats <metrics.json>` | Print metrics snapshot |

## Workload Strategies

| Strategy | Description |
|----------|-------------|
| `balanced` | Normal two-sided flow |
| `bull` / `bear` | Directional buy/sell bias |
| `flash-crash` | High-rate sell-heavy stress |
| `hft` | Tiny orders, high cancel rate |
| `market-maker` | Spread maintenance |
| `stress` | Boundary quantities |
| `replay` | Replay from event journal |

## Design Decisions

- **Event sourcing:** Every state change is an append-only event in `eventlog.bin`
- **Determinism:** Same seed + config → identical `SHA256(eventlog.bin)`
- **Fixed-point prices:** `int64_t` ticks — no floating point in matching
- **Single-threaded engine:** Deterministic and cache-friendly; per-symbol threading planned for v2
- **Synthetic workloads:** Probability distributions, not simulated human traders

## Project Structure

```
FastExchange/
├── core/           # Engine, matching, journal, metrics
├── modules/        # Workload, scenario, benchmark, replay
├── api/            # Crow REST server
├── cli/            # fastexchange binary
├── dashboard/      # React UI
├── scenarios/      # YAML workload configs
├── config/         # default.yaml
├── tests/          # Google Test
└── docs/           # Design documentation
```

## Documentation

- [Vision](docs/vision.md)
- [Architecture](docs/architecture.md)
- [Events](docs/events.md)
- [Plugins](docs/plugins.md)
- [Metrics](docs/metrics.md)
- [Scenarios](docs/scenarios.md)
- [Benchmark environment](docs/benchmark-env.md)

## Roadmap (v2)

- Memory pool allocator
- Per-symbol threading
- Lock-free ingress queue
- Market / IOC / FOK order types
- Replay time-travel UI

## License

MIT — see [LICENSE](LICENSE).
