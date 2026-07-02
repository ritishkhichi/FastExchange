# Benchmark Environment

## Published Results Environment

Official benchmark numbers are produced on:

| Setting | Value |
|---------|-------|
| OS | Ubuntu 24.04 LTS |
| Compiler | g++-14 |
| Flags | `-O3 -march=native -flto -DNDEBUG` |
| CMake | Release build |
| CPU | Document model (e.g. AMD Ryzen 7 5800X) |
| RAM | 32 GB |

## Reproduction Steps

```bash
# Install dependencies (Ubuntu)
sudo apt install build-essential cmake g++-14 libssl-dev

# Clone and build
git clone <repo-url> FastExchange && cd FastExchange
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-14
cmake --build build -j$(nproc)

# Run standardized benchmark
./scripts/benchmark.sh

# Or manually
./build/fastexchange benchmark --scenario scenarios/balanced.yaml --orders 10000000
```

## Development Platforms

FastExchange builds and runs on **Windows**, **Linux**, and **macOS** for development and testing.

**Only Linux numbers are published** in the README benchmark table. Windows/macOS latency will differ due to scheduler and allocator behavior.

## What We Measure

| Metric | Unit |
|--------|------|
| Throughput | orders/sec |
| Latency P50/P95/P99 | microseconds |
| Memory | RSS MB |
| CPU | percent |
| Rejects | count |
| Trades | count |

## Determinism Note

Benchmarks use simulated timestamps and seeded RNG. Same seed + config on same OS/compiler should produce identical `SHA256(eventlog.bin)`. Minor floating-point differences in metrics aggregation are acceptable; event log hash is the correctness oracle.
