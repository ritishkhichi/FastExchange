# FastExchange — Vision

## Product Statement

> **FastExchange is an event-driven exchange simulation framework for benchmarking, replaying, and profiling matching engine architectures under reproducible synthetic workloads.**

This sentence is frozen. All design and implementation decisions must support it.

---

## What FastExchange Is

A **systems benchmark platform** for evaluating how efficiently an exchange processes millions of orders while maintaining correctness.

FastExchange answers one question:

> **How efficiently can an exchange process millions of orders while maintaining correctness?**

---

## What FastExchange Is Not

| Not this | Why |
|----------|-----|
| Trading platform | No real money, no broker integration |
| Stock simulator | Assets are fictional; focus is systems engineering |
| Crypto exchange | Same engine could match anything — we model events |
| Another 200-line order book | Matching is ~15–20% of the project |

---

## Guiding Principles

Every feature in FastExchange must satisfy **at least one** of these goals:

1. **Correctness** — deterministic replay, event sourcing, reproducible workloads
2. **Performance** — benchmarking, latency metrics, memory layout
3. **Observability** — metrics pipeline, profiler, dashboard
4. **Extensibility** — workload strategies, YAML configuration, plugin architecture

If a proposed feature does not clearly fit one of these four goals, it does not belong in v1.

---

## Headline Features

- Event-sourced architecture
- Deterministic replay (SHA256 verification)
- Reproducible benchmarks
- Scenario runner (YAML-driven)
- Plugin architecture (compile-time)
- Metrics pipeline
- Multi-asset matching
- REST API (Crow)
- Web dashboard (minimal, Grafana-style)
- Benchmark suite

---

## Target Audience

Engineers and recruiters at firms where systems design matters:

- Tower Research, HRT, Jane Street (low-latency systems)
- Databricks, Rubrik, Rippling (performance + observability + API design)

The interview story is **engineering**, not finance.
