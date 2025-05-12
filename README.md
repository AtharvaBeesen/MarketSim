# Market Simulator (C++ Orderbook with Python Trading Agents)

A modular, multi-symbol limit-order-book engine in C++ with Python bindings, plus a library of trading agents and a full-featured backtester.

## Features

- **High-Performance LOB Engine**  
  - Thread-safe C++ implementation using balanced BSTs for \(O(\log n)\) price/time-priority matching  
  - Packaged as a pip-installable Python extension via **pybind11**  
  - Zero-copy data access & sub-millisecond native calls  

- **Multi-Symbol OrderBookManager**  
  - Create/manage separate order books by symbol (e.g. AAPL, MSFT, GOOGL)  
  - Route add/cancel/replace requests and aggregate per-symbol depth & trade history  

- **Python Agent Framework**  
  - **Market-Maker**: fee-aware quoting with stale-quote cancellation  
  - **Trend-Follower**: trades on multi-step mid-price returns  
  - **Mean-Reverter**: reacts to deviations from a rolling mid average  
  - **Liquidity-Taker**: fires market orders when spreads exceed cost thresholds  
  - **Supervised Predictor**: online logistic regression via scikit-learn  
  - **RL Agent**: tabular Q-learning (PyTorch/stable-baselines3)  
  - All agents conform to a common `step()` / `on_trade()` interface  

- **Backtesting & Simulation**  
  - Configurable via `config.py` (tick size, time interval, fees, latency model, liquidity depth)  
  - Random-walk mid-price drift & periodic liquidity reseeding  
  - Transaction-cost & Gaussian latency injection  
  - Per-agent P&L, inventory & NAV tracking with CSV export
 
## Files

 - **Orderbook**
    - hpp located inside root/include
    - cpp located inside root/src

 - **Trading Agents**
    - all agents inside root/python/orderbook_agents

 - **Simulation**
   - simulation.py located inside root/python

 - **Simulation Outputs**
    - graphs located inside root/python
    - metrics file generated post simulation for info on each trade made in the simulation located in src/python
