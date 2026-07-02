# Scenario YAML Schema

## Schema

```yaml
name: string                    # required
duration_sec: int             # required (or order_limit)
seed: uint64                  # required for determinism
order_limit: int              # optional, alternative stop condition

symbols: [string, ...]        # required
tick_size: float              # e.g. 0.0001

initial_mid_prices:           # required per symbol
  SYMBOL: float

strategy: string              # workload strategy name

workload:
  arrival_rate: int           # target orders/sec
  buy_probability: float      # 0.0 - 1.0
  sell_probability: float
  price_distribution: string  # normal | uniform
  price_bias_ticks: int       # directional bias
  quantity_distribution: string  # poisson | uniform
  quantity_lambda: float      # for poisson
  quantity_min: int
  quantity_max: int
  cancel_probability: float
  modify_probability: float
  max_quantity: int

engine:
  max_order_size: int
  risk_plugin: string         # default
  matching_algorithm: string  # price_time_fifo
  auto_register_symbols: bool # default false

output:
  event_log: string           # path to .bin
  metrics: string             # path to metrics JSON
```

## Config Merge Order

1. `config/default.yaml`
2. Scenario YAML (overrides)
3. CLI `--output` flags (paths only)

## Example Scenarios

See `scenarios/balanced.yaml`, `scenarios/flash-crash.yaml`, `scenarios/market-maker.yaml`.

## CLI Usage

```bash
fastexchange run scenarios/flash-crash.yaml
fastexchange benchmark --scenario scenarios/balanced.yaml --orders 1000000
fastexchange compare results/balanced-metrics.json results/flash-crash-metrics.json
```
