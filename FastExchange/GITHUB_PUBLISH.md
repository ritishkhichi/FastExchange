# Publishing FastExchange to GitHub

Complete checklist to create the repository and make it look professional.

---

## 1. Files already in the repo (ready to push)

| File / folder | Purpose |
|---------------|---------|
| `README.md` | Main landing page with demo GIF, quick start, CLI reference |
| `LICENSE` | MIT license |
| `docs/demo.gif` | README demo recording (~32 MB — see note below) |
| `docs/*.md` | Architecture and design docs |
| `.github/workflows/ci.yml` | CI on Ubuntu + Windows |
| `.gitignore` | Excludes `build/`, `results/`, `node_modules/`, `*.bin` |
| `CMakeLists.txt` + source | Full C++ project |
| `dashboard/` | React dashboard (commit source; `node_modules` ignored) |
| `scenarios/`, `config/` | Example YAML configs |
| `scripts/benchmark.sh` | Benchmark helper script |

### Demo GIF size

`docs/demo.gif` is ~32 MB. GitHub allows files up to 100 MB, but large GIFs slow README loading. Optional: compress before push:

- [ezgif.com/optimize](https://ezgif.com/optimize) — target 3–8 MB
- Or use a shorter clip / lower FPS

---

## 2. One-time: initialize Git and first commit

Run from the project root (`FastExchange`):

```powershell
cd C:\Users\Ritish\Desktop\FastExchange

git init
git branch -M main

git add .
git status
```

**Verify** `git status` does **not** include:

- `build/`
- `results/`
- `dashboard/node_modules/`
- `*.bin` event logs
- `.env` or secrets (none should exist)

```powershell
git commit -m "$( @'
Initial release: FastExchange exchange simulation framework

Event-sourced C++20 matching engine with synthetic workloads,
benchmarking, replay, REST API, and live React dashboard.
'@ )"
```

---

## 3. Create GitHub repository

1. Go to [github.com/new](https://github.com/new)
2. **Repository name:** `FastExchange` (or `fastexchange`)
3. **Description:** `Event-driven exchange simulation framework for benchmarking matching engines under reproducible synthetic workloads (C++20)`
4. **Public**
5. Do **not** add README, .gitignore, or license (you already have them)
6. Click **Create repository**

---

## 4. Push to GitHub

Replace `YOUR_USERNAME` with your GitHub username:

```powershell
git remote add origin https://github.com/YOUR_USERNAME/FastExchange.git
git push -u origin main
```

If the GIF push is slow or fails, wait and retry, or compress `docs/demo.gif` first.

---

## 5. GitHub repository settings (web UI)

### About (right sidebar on repo home)

- **Description:** same as above
- **Website:** optional — link to live demo if you deploy later
- **Topics** (add all that apply):
  ```
  cpp
  cpp20
  matching-engine
  order-book
  low-latency
  event-sourcing
  benchmarking
  simulation
  cmake
  react
  trading-simulator
  ```

### General

- Default branch: `main`
- Enable **Issues** if you want bug reports
- Enable **Discussions** (optional) for Q&A

### Actions

- After first push, open **Actions** tab and confirm CI passes (Linux + Windows build + tests)

---

## 6. Optional polish

| Item | How |
|------|-----|
| README badges | Add build status after first CI run: `![CI](https://github.com/YOUR_USERNAME/FastExchange/actions/workflows/ci.yml/badge.svg)` |
| Social preview | Repo **Settings → General → Social preview** — upload a static screenshot (GIF won’t work here) |
| Releases | Tag `v0.1.0` with release notes from README features |
| GitHub Pages | Not required; dashboard runs locally via `npm run dev` |
| Profile README | Link to FastExchange from your GitHub profile |

---

## 7. What reviewers / visitors will try

Clone and build:

```bash
git clone https://github.com/YOUR_USERNAME/FastExchange.git
cd FastExchange
cmake -B build -DCMAKE_BUILD_TYPE=Release -DFASTEXCHANGE_BUILD_API=ON
cmake --build build --config Release -j
```

Run tests:

```bash
cd build
ctest --output-on-failure
```

Quick demo:

```bash
./build/cli/Release/fastexchange.exe run scenarios/balanced.yaml   # Windows path
# or
./build/fastexchange run scenarios/balanced.yaml                   # Linux
```

Live dashboard (two terminals):

```bash
./build/api/Release/fastexchange_api.exe --scenario scenarios/balanced.yaml --orders 30000
cd dashboard && npm install && npm run dev
```

---

## 8. Do not commit

- `build/` — CMake output
- `results/` — generated benchmarks and event logs
- `dashboard/node_modules/`
- `*.bin` — event journals
- API keys, `.env`, personal paths
- IDE folders (`.vs/`, etc. — add if needed)

---

## 9. Suggested first release notes (v0.1.0)

```markdown
## FastExchange v0.1.0

First public release.

- C++20 event-sourced matching engine (price-time FIFO)
- 8 synthetic workload strategies + YAML scenarios
- Deterministic replay with SHA256 verification
- Google Benchmark integration + compare command
- Crow REST API + React dashboard with live order book
- CI on Ubuntu and Windows
```

Create release:

```powershell
git tag -a v0.1.0 -m "v0.1.0 - initial release"
git push origin v0.1.0
```

Then on GitHub: **Releases → Draft new release** → choose tag `v0.1.0` → paste notes → Publish.

---

## 10. Quick copy-paste summary

**Repo description:**
```
Event-driven exchange simulation framework for benchmarking matching engines under reproducible synthetic workloads (C++20)
```

**Topics:**
`cpp` `cpp20` `matching-engine` `order-book` `event-sourcing` `benchmarking` `simulation` `cmake` `react`

**Minimum commands to go live:**
```powershell
git init
git add .
git commit -m "Initial release: FastExchange"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/FastExchange.git
git push -u origin main
```

Done — your README GIF, LICENSE, CI, and docs will show on the repo homepage.
