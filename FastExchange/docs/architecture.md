# FastExchange Architecture

## Overview

FastExchange is an event-driven exchange simulation framework. The matching engine is one module inside a larger platform for benchmarking, replaying, and profiling exchange architectures.

## Component Diagram

```
         CLI / REST API / Dashboard
                  |
           Scenario Runner
                  |
         Workload Generator (Strategy)
                  |
           Exchange Engine
                  |
    +-------------+-------------+
    |             |             |
 Matching     Risk Plugin   Event Journal
    |             |             |
    +-------------+-------------+
                  |
            Metrics Engine
    +-------------+-------------+
    |             |             |
 Benchmark      Replay      Dashboard Feed
```

## Core Components

### ExchangeEngine
Single-threaded pipeline processing inbound events:
1. Record ingress timestamp
2. Append `OrderSubmitRequested` (or cancel/modify) to journal
3. Risk validation
4. Matching against `SymbolRegistry` order books
5. Append pipeline events (accepted, rejected, trades, book updates)
6. Record metrics

### SymbolRegistry
Maps `Symbol` → `OrderBook`. Symbols are registered from YAML config at scenario start. No hardcoded symbol list in engine code.

### EventJournal
Append-only binary log (`eventlog.bin`). All state changes are events. JSON export is a separate command, not dual-write at runtime.

### MetricsEngine
First-class observability. Counters, histograms, gauges consumed by benchmark, CLI, profiler, and dashboard.

### WorkloadFactory
Compile-time strategy registration via `REGISTER_STRATEGY`. Strategies produce deterministic inbound events from seeded RNG + YAML config.

## Threading Model

v1: **Fully single-threaded.** Determinism and cache locality take priority.

v2 roadmap: Per-symbol matching threads (ABC on thread 1, XYZ on thread 2).

## Plugin Boundaries

| Plugin | Interface | v1 Default |
|--------|-----------|------------|
| Matching | `IMatchingPlugin` | `PriceTimeFifoMatcher` |
| Risk | `IRiskPlugin` | `DefaultRiskPlugin` |
| Workload | `IWorkloadStrategy` | YAML-selected strategy |

Plugins are compile-time registered. No dynamic `.so` loading in v1.

## Config Layering

1. `config/default.yaml` — global defaults
2. `scenarios/*.yaml` — per-run overrides
3. CLI flags — output paths only

## Determinism Contract

```
same seed + same workload config + same engine config
  → identical SHA256(eventlog.bin)
```

Simulated clock (`Timestamp`) is used during benchmarks, not wall-clock time.
