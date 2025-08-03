# Market Simulator (C++ Orderbook with Python Trading Agents)

A high-performance limit order book engine implemented in C++ with Python bindings, featuring a comprehensive library of trading agents for algorithmic trading research and backtesting.

## Architecture

### Core Engine (C++)
- **OrderBook**: O(log n) price/time-priority matching via balanced BSTs (`std::map`)
- **OrderBookManager**: Multi-symbol order book management with fast order lookup
- **Python Bindings**: Sub-millisecond cross-language calls via pybind11

### Trading Agents (Python)
Six distinct algorithmic trading strategies:

1. **Market Maker**: Fee-aware liquidity provision with dynamic spread management
2. **Trend Follower**: Momentum-based trading with configurable thresholds
3. **Mean Reverter**: Statistical arbitrage using rolling price averages
4. **Liquidity Taker**: Opportunistic market order execution
5. **Supervised Predictor**: Logistic regression for price direction prediction
6. **RL Agent**: Tabular Q-learning with discrete state/action spaces

## Key Features

- **High Performance**: C++ core with O(log n) order matching
- **Multi-Symbol Support**: Concurrent order books for multiple instruments
- **Fee-Aware Trading**: All agents incorporate transaction costs
- **Latency Modeling**: Realistic execution delays and market impact
- **Comprehensive Metrics**: P&L, inventory, NAV tracking per agent
- **Extensible Framework**: Modular agent architecture for strategy development

## Simulation Engine Output:

- **999-step backtest** with 100ms time slices
- **Random walk price drift** with configurable volatility
- **Dynamic liquidity reseeding** to maintain market depth
- **2.5k+ trades** executed in 3.2 seconds across multiple symbols

## Build & Usage

```bash
# Build C++ extension
mkdir build && cd build
cmake ..
make

# Run simulation
cd python
python simulation.py
```

## Dependencies

- **C++**: C++17, Boost, pybind11
- **Python**: numpy, pandas, scikit-learn, stable-baselines3, torch

## Performance

- Sub-millisecond order processing
- Real-time market data queries
- Efficient memory management with smart pointers
- Thread-safe order book operations

The system provides a production-ready foundation for algorithmic trading research, combining high-performance order matching with sophisticated trading strategies in a unified backtesting framework.
