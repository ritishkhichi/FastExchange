# Event Catalog and Binary Journal Format

## Event Types (v1)

### Inbound
| Type | Description |
|------|-------------|
| `OrderSubmitRequested` | New limit order submitted |
| `OrderCancelRequested` | Cancel existing order |
| `OrderModifyRequested` | Modify price/qty of existing order |

### Pipeline
| Type | Description |
|------|-------------|
| `RiskChecked` | Risk validation result |
| `OrderAccepted` | Order passed risk, entering matching |
| `OrderRejected` | Order failed risk |
| `TradeExecuted` | Match occurred |
| `OrderPartiallyFilled` | Order partially filled |
| `OrderFullyFilled` | Order completely filled |
| `OrderCancelled` | Order removed from book |
| `OrderModified` | Order updated in book |
| `OrderBookUpdated` | Book state changed (compact snapshot) |

### System
| Type | Description |
|------|-------------|
| `ScenarioStarted` | Scenario run began |
| `ScenarioEnded` | Scenario run finished |

## Binary File Layout

```
FileHeader (64 bytes, fixed)
  magic:      char[8]   "FXEVENT\0"
  version:    uint32    1
  seed:       uint64
  config_hash:uint64   hash of merged config YAML
  reserved:   bytes

EventRecord (variable, repeated)
  type:        uint16
  timestamp:   uint64   simulated nanoseconds
  payload_len: uint32
  payload:     bytes[payload_len]

FileFooter (40 bytes, fixed)
  event_count: uint64
  sha256:      uint8[32]   hash of FileHeader + all EventRecords
```

## Payload Encoding

Payloads use a compact binary layout per event type. Strings (symbol) are length-prefixed UTF-8.

Common fields:
- `order_id`: uint64
- `symbol`: uint16 len + bytes
- `side`: uint8 (0=buy, 1=sell)
- `price`: int64 (ticks)
- `quantity`: uint64
- `reason`: uint16 len + bytes (for rejections)

## JSON Export

`fastexchange export eventlog.bin -o eventlog.json` decodes the binary journal to:

```json
{
  "header": { "version": 1, "seed": 42, "config_hash": "..." },
  "events": [
    { "type": "OrderSubmitRequested", "timestamp": 1000, "order_id": 1, ... }
  ],
  "footer": { "event_count": 1000, "sha256": "..." }
}
```

## SHA256 Verification

Hash is computed over: `FileHeader || EventRecord[0] || ... || EventRecord[n-1]`

Footer `sha256` field stores the digest. `fastexchange verify` recomputes and compares.
