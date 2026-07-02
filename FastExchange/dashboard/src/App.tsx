import { useEffect, useState } from 'react'
import {
  LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer,
  BarChart, Bar, Cell,
} from 'recharts'

interface Metrics {
  scenario?: string
  counters?: Record<string, number>
  histograms?: Record<string, { p50: number; p95: number; p99: number }>
  gauges?: Record<string, number>
  throughput?: { orders_per_sec: number; trades_per_sec: number }
}

interface BookLevel {
  price: number
  quantity: number
}

interface Book {
  symbol: string
  bids: BookLevel[]
  asks: BookLevel[]
  best_bid: number
  best_ask: number
}

interface Trade {
  symbol: string
  price: number
  quantity: number
  buy_order_id: number
  sell_order_id: number
}

export default function App() {
  const [metrics, setMetrics] = useState<Metrics>({})
  const [book, setBook] = useState<Book | null>(null)
  const [trades, setTrades] = useState<Trade[]>([])
  const [history, setHistory] = useState<{ t: number; orders: number; p99: number }[]>([])
  const [scenario, setScenario] = useState('')
  const [live, setLive] = useState(false)

  useEffect(() => {
    const fetchData = async () => {
      try {
        const sRes = await fetch('/api/scenario')
        let isLive = false
        if (sRes.ok) {
          const s = await sRes.json()
          setScenario(s.name || '')
          isLive = !!s.running
          setLive(isLive)
        }

        const mRes = await fetch('/api/metrics')
        if (mRes.ok) {
          const m: Metrics = await mRes.json()
          setMetrics(m)
          if (m.scenario) setScenario(m.scenario)
          const ops = m.throughput?.orders_per_sec || 0
          const p99 = m.histograms?.['latency.order.total']?.p99 || 0
          setHistory((h) => [...h.slice(-59), { t: Date.now(), orders: ops, p99 }])
        }

        const bRes = await fetch('/api/book/ABC')
        if (bRes.ok) setBook(await bRes.json())

        const tRes = await fetch('/api/trades')
        if (tRes.ok) setTrades(await tRes.json())
      } catch {
        // API offline
      }
    }
    fetchData()
    const id = setInterval(fetchData, live ? 400 : 1000)
    return () => clearInterval(id)
  }, [live])

  const ordersSubmitted = metrics.counters?.['orders.submitted'] || 0
  const tradesTotal = metrics.counters?.['trades.total'] || 0
  const rejected = metrics.counters?.['orders.rejected'] || 0
  const p50 = metrics.histograms?.['latency.order.total']?.p50 || 0
  const p99 = metrics.histograms?.['latency.order.total']?.p99 || 0
  const mem = metrics.gauges?.['memory.rss_mb'] || 0
  const cpu = metrics.gauges?.['cpu.percent'] || 0

  const depthData = [
    ...(book?.bids?.slice(0, 6).map((b) => ({
      level: `B ${b.price}`,
      qty: b.quantity,
      side: 'bid' as const,
    })) || []),
    ...(book?.asks?.slice(0, 6).map((a) => ({
      level: `A ${a.price}`,
      qty: a.quantity,
      side: 'ask' as const,
    })) || []),
  ]

  const stageData = Object.entries(metrics.histograms || {})
    .filter(([k]) => k.includes('latency.stage'))
    .map(([k, v]) => ({
      stage: k.replace('latency.stage.', ''),
      p50: v.p50,
      p99: v.p99,
    }))

  const spread =
    book && book.best_bid > 0 && book.best_ask > 0 ? book.best_ask - book.best_bid : 0

  return (
    <div className="dashboard">
      <header>
        <div className="header-row">
          <h1>FastExchange</h1>
          {live && <span className="live-badge">LIVE</span>}
        </div>
        <p>
          Scenario: <strong>{scenario || 'idle'}</strong>
          {live ? ' — synthetic orders streaming (local simulation)' : ' — snapshot mode'}
          {spread > 0 && ` | Spread: ${spread} ticks`}
        </p>
      </header>

      <div className="grid">
        <div className="panel">
          <h2>Orders / sec</h2>
          <div className="stat-value">
            {Math.round(metrics.throughput?.orders_per_sec || 0).toLocaleString()}
          </div>
          <ResponsiveContainer width="100%" height={120}>
            <LineChart data={history}>
              <XAxis dataKey="t" hide />
              <YAxis hide />
              <Tooltip />
              <Line type="monotone" dataKey="orders" stroke="#58a6ff" dot={false} strokeWidth={2} />
            </LineChart>
          </ResponsiveContainer>
        </div>

        <div className="panel">
          <h2>Latency (μs)</h2>
          <div className="latency-row">
            <span>P50: <strong>{p50.toFixed(2)}</strong></span>
            <span>P99: <strong>{p99.toFixed(2)}</strong></span>
          </div>
          <ResponsiveContainer width="100%" height={100}>
            <LineChart data={history}>
              <Line type="monotone" dataKey="p99" stroke="#f0883e" dot={false} strokeWidth={2} />
            </LineChart>
          </ResponsiveContainer>
        </div>

        <div className="panel">
          <h2>CPU / Memory</h2>
          <div className="stat-value" style={{ fontSize: '1.25rem' }}>
            {cpu.toFixed(1)}% CPU
          </div>
          <div>{mem.toFixed(1)} MB RSS</div>
          <div className="counters">
            <div>Submitted: {ordersSubmitted.toLocaleString()}</div>
            <div>Trades: {tradesTotal.toLocaleString()}</div>
            <div>Rejected: {rejected.toLocaleString()}</div>
          </div>
        </div>

        <div className="panel">
          <h2>Order Book (ABC)</h2>
          {!book?.bids?.length && !book?.asks?.length ? (
            <p className="empty-hint">No resting orders — start API with <code>--scenario</code> for live book</p>
          ) : (
            <table>
              <thead><tr><th>Side</th><th>Price (ticks)</th><th>Qty</th></tr></thead>
              <tbody>
                {book?.bids?.slice(0, 6).map((b) => (
                  <tr key={`b${b.price}`}><td className="bid">BID</td><td>{b.price}</td><td>{b.quantity}</td></tr>
                ))}
                <tr><td colSpan={3} className="spread-row">— spread —</td></tr>
                {book?.asks?.slice(0, 6).map((a) => (
                  <tr key={`a${a.price}`}><td className="ask">ASK</td><td>{a.price}</td><td>{a.quantity}</td></tr>
                ))}
              </tbody>
            </table>
          )}
        </div>

        <div className="panel full">
          <h2>Market Depth</h2>
          {depthData.length === 0 ? (
            <p className="empty-hint">Depth chart populates during live scenario runs</p>
          ) : (
            <ResponsiveContainer width="100%" height={160}>
              <BarChart data={depthData}>
                <XAxis dataKey="level" tick={{ fontSize: 10 }} />
                <YAxis />
                <Tooltip />
                <Bar dataKey="qty">
                  {depthData.map((d, i) => (
                    <Cell key={i} fill={d.side === 'bid' ? '#3fb950' : '#f85149'} />
                  ))}
                </Bar>
              </BarChart>
            </ResponsiveContainer>
          )}
        </div>

        <div className="panel">
          <h2>Recent Trades</h2>
          {trades.length === 0 ? (
            <p className="empty-hint">No trades yet</p>
          ) : (
            <table>
              <thead><tr><th>Price</th><th>Qty</th><th>Symbol</th></tr></thead>
              <tbody>
                {[...trades].reverse().slice(0, 8).map((t, i) => (
                  <tr key={i}>
                    <td>{t.price}</td>
                    <td>{t.quantity}</td>
                    <td>{t.symbol}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          )}
        </div>

        <div className="panel">
          <h2>Stage Profiler (μs)</h2>
          {stageData.length === 0 ? (
            <p className="empty-hint">Stage histograms appear after orders process</p>
          ) : (
            <ResponsiveContainer width="100%" height={160}>
              <BarChart data={stageData} layout="vertical">
                <XAxis type="number" />
                <YAxis type="category" dataKey="stage" width={60} tick={{ fontSize: 11 }} />
                <Tooltip />
                <Bar dataKey="p99" fill="#a371f7" name="P99" />
                <Bar dataKey="p50" fill="#58a6ff" name="P50" />
              </BarChart>
            </ResponsiveContainer>
          )}
        </div>
      </div>
    </div>
  )
}
