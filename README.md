# Market Simulator (C++ Orderbook with Python Trading Agents)

A high-performance limit order book engine implemented in C++ with Python bindings, featuring a comprehensive library of trading agents for algorithmic trading research and backtesting.

## Architecture

### Core Engine (C++)
- **OrderBook**: O(log n) price/time-priority matching via balanced BSTs (`std::map`)
- **OrderBookManager**: Multi-symbol order book management with symbol isolation
- **Python Bindings**: pybind11 integration for sub-millisecond cross-language calls

### Data Structures
- **Order**: Limit/Market orders with client tracking and timestamp validation
- **Trade**: Executed trade records with price, quantity, and counterparty mapping
- **OrderBook**: Bid/Ask price levels with FIFO time priority within levels

## Trading Agents

### Rule-Based Agents
- **MarketMaker**: Bid-ask spread management with fee-aware quoting
- **TrendFollower**: Momentum-based trading using return thresholds
- **MeanReverter**: Statistical arbitrage via rolling mean deviation
- **LiquidityTaker**: Probabilistic market order execution

### ML Agents
- **SupervisedPredictor**: Logistic regression on mid-price returns (scikit-learn)
- **RLAgent**: Tabular Q-learning with discrete state/action spaces (PyTorch)

## Simulation Framework

### Market Dynamics
- Random walk price drift with configurable volatility
- Dynamic liquidity reseeding to maintain market depth
- Transaction costs: per-order and per-share fees
- Latency constraints with Gaussian jitter modeling

### Backtesting Engine
- 999-step simulation with 100ms time slices
- Multi-symbol order book exposure (AAPL, MSFT, GOOGL)
- Real-time P&L, inventory, and NAV tracking
- Comprehensive trade execution logging

## Performance Results

The supervised learning agent achieved optimal performance:
- **832 trades** executed over 3.2 seconds
- **$40.3k P&L** with $48.4 average return per trade
- **2.5k total trades** across all agents in the simulation

## Build System

### C++ Core
```bash
mkdir build && cd build
cmake ..
make
```

### Python Dependencies
```bash
pip install -r python/requirements.txt
```

### Simulation Execution
```bash
cd python
python simulation.py
```

## Key Features

- **High Performance**: C++ engine with O(log n) matching complexity
- **Multi-Agent**: 6 distinct trading strategies with fee-aware execution
- **Realistic Market**: Price drift, latency, and transaction cost modeling
- **Extensible**: Modular agent architecture for strategy development
- **Research Ready**: Comprehensive metrics and visualization outputs

## Dependencies

- **C++**: C++17, CMake 3.10+, pybind11
- **Python**: numpy, pandas, scikit-learn, PyTorch, stable-baselines3
- **Build**: Boost (system, json)

The system demonstrates an implementation of algorithmic trading infrastructure with market simulation and diverse agent strategies.
