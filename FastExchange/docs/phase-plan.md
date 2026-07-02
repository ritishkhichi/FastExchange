# Phase Plan

## Phase 0 — Design Docs
**Deliverable:** Architecture, events, plugins, metrics, scenarios, data structures, benchmark env docs + config templates.
**Acceptance:** All docs reviewed; no C++ engine code.

## Phase 1 — Foundation
**Deliverable:** CMake project, types, Event/EventCodec, binary EventJournal, MetricsEngine, ConfigLoader.
**Acceptance:** Unit tests for journal round-trip and metrics snapshot JSON.

## Phase 2 — Matching
**Deliverable:** SymbolRegistry, OrderBook, PriceTimeFifoMatcher.
**Acceptance:** Partial fill, multi-symbol isolation, cancel/modify tests pass.

## Phase 3 — Engine
**Deliverable:** ExchangeEngine pipeline, DefaultRiskPlugin, full event journaling.
**Acceptance:** 10k order integration test; determinism on event/trade counts.

## Phase 4 — Workload
**Deliverable:** IWorkloadStrategy, WorkloadFactory, 8 built-in strategies.
**Acceptance:** Fixed seed produces identical first N events per strategy.

## Phase 5 — Scenario + CLI
**Deliverable:** ScenarioRunner, `run`, `export`, `verify`, `stats` commands.
**Acceptance:** `run` produces `.bin` + metrics JSON; `verify` checks SHA256.

## Phase 6 — Benchmark
**Deliverable:** Google Benchmark integration, `compare`, results persistence.
**Acceptance:** 1M order benchmark completes; benchmark-env documented.

## Phase 7 — Dashboard + API
**Deliverable:** Crow REST/WebSocket, minimal React dashboard (6 panels).
**Acceptance:** Dashboard shows live metrics during scenario run.

## Phase 8 — Replay
**Deliverable:** ReplayStrategy, determinism verification.
**Acceptance:** Golden test: run → replay → verify → identical SHA256.

## Phase 9 — Polish
**Deliverable:** README, CI, example scenarios, benchmark script.
**Acceptance:** Builds on Windows + Ubuntu; tests pass in CI.
