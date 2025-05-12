# python/simulation.py

import os
import sys
import glob
import importlib.util
import random
import pandas as pd

import config

#LOAD THE CPP ORDERBOOK AS A MODULE...
here         = os.path.dirname(os.path.abspath(__file__))     # python/
project_root = os.path.abspath(os.path.join(here, os.pardir))  # AI_OrderBook_Proj
build_dir    = os.path.join(project_root, "build")
release_dir  = os.path.join(build_dir, "Release")

for ext_dir in (release_dir, build_dir):
    if os.path.isdir(ext_dir):
        break
else:
    raise RuntimeError(f"No build folder found at {build_dir!r}")

pyds = glob.glob(os.path.join(ext_dir, "orderbook*.pyd"))
if not pyds:
    raise FileNotFoundError(f"No orderbook*.pyd in {ext_dir!r}")
pyd_path = pyds[0]

spec = importlib.util.spec_from_file_location("orderbook", pyd_path)
_orderbook = importlib.util.module_from_spec(spec)
spec.loader.exec_module(_orderbook)
sys.modules["orderbook"] = _orderbook

from orderbook import OrderBookManager, Order, Side, OrderType

#INITIALLY SEED THE ORDERBOOK WITH DUMMY DATA TO ENSURE LIQUIDITY
def seed_order_book(mgr, symbol, levels=5, size=10, tick=1.0):
    mid = 100.0
    for i in range(1, levels + 1):
        mgr.place_order(Order(f"seed-b-{symbol}-{i}", "seeder",
                              symbol, Side.BUY,  mid - i*tick, size, OrderType.LIMIT))
        mgr.place_order(Order(f"seed-s-{symbol}-{i}", "seeder",
                              symbol, Side.SELL, mid + i*tick, size, OrderType.LIMIT))


#Manager proxy for latency & per-order fees...
CURRENT_TIME = 0.0

class ManagerProxy:
    def __init__(self, real_mgr, agent_wrapper):
        self._mgr   = real_mgr
        self._agent = agent_wrapper

    def get_best_bid(self, sym):
        return self._mgr.get_best_bid(sym)
    def get_best_ask(self, sym):
        return self._mgr.get_best_ask(sym)

    def cancel_order(self, sym, oid):
        try:
            return self._mgr.cancel_order(sym, oid)
        except RuntimeError:
            return

    def place_order(self, order):
        latency   = max(0.0, random.gauss(config.LATENCY_MEAN, config.LATENCY_STD))
        exec_time = CURRENT_TIME + latency
        delayed_orders.append((exec_time, order, self._agent))
        self._agent.pnl -= config.FEE_PER_ORDER



class AgentWrapper: #USED TO HOLD STATE FOR EACH AGENT
    def __init__(self, cls, symbol, client_id):
        self.real_mgr   = real_mgr
        self.pnl         = 0.0
        self.inventory   = 0
        self.symbol      = symbol
        self.client_id   = client_id

        self.proxy      = ManagerProxy(real_mgr, self)
        self.agent      = cls(self.proxy, symbol, client_id=client_id)


# Initialize everything...
real_mgr        = OrderBookManager()
delayed_orders  = []      # (exec_time, order, wrapper)
order_map       = {}      # order_id -> wrapper

config._base_mid = {sym: 100.0 for sym in config.SYMBOLS}

for sym in config.SYMBOLS:
    real_mgr.add_order_book(sym)
    seed_order_book(real_mgr, sym)

agent_plan = [
    ("MM", 2, "market_maker",    "MarketMakerAgent"),
    ("TF", 2, "trend_follower",  "TrendFollowerAgent"),
    ("MR", 2, "mean_reverter",   "MeanReverterAgent"),
    ("LT", 2, "liquidity_taker", "LiquidityTakerAgent"),
    ("SP", 1, "supervised_predictor", "SupervisedPredictorAgent"),
    ("RL", 1, "rl_agent",        "RLAgent"),
]

agents = []
for tag, count, module_name, class_name in agent_plan:
    m = importlib.import_module(f"orderbook_agents.{module_name}")
    AgentClass = getattr(m, class_name)
    for i in range(count):
        sym = config.SYMBOLS[len(agents) % len(config.SYMBOLS)]
        cid = f"{tag}-{i}"
        wrapper = AgentWrapper(AgentClass, sym, client_id=cid)
        agents.append(wrapper)

# Prepare history buffers and trade counters for the agents...
steps          = []
pnl_history    = {ag.client_id: [] for ag in agents}
inv_history    = {ag.client_id: [] for ag in agents}
nav_history    = {ag.client_id: [] for ag in agents}
trade_counts   = {ag.client_id: 0  for ag in agents}


# MAIN SIMULATION LOOP!
for step in range(config.NUM_STEPS):
    CURRENT_TIME = step * config.DT

    #a) First, fundamental mid‐price drift + light reseed
    for sym in config.SYMBOLS:
        config._base_mid[sym] += random.gauss(0, config.FUND_VOLATILITY)
        m = config._base_mid[sym]
        real_mgr.place_order(Order(f"fund-b-{sym}-{step}", "fund",
                                   sym, Side.BUY,  m - 0.05, 1, OrderType.LIMIT))
        real_mgr.place_order(Order(f"fund-s-{sym}-{step}", "fund",
                                   sym, Side.SELL, m + 0.05, 1, OrderType.LIMIT))

    #b) Now release delayed orders
    for exec_t, order, agent in list(delayed_orders):
        if exec_t <= CURRENT_TIME:
            try:
                real_mgr.place_order(order)
                order_map[order.order_id] = agent
            except RuntimeError as e:
                if "liquidity" in str(e).lower():
                    pass
                else:
                    raise
            finally:
                delayed_orders.remove((exec_t, order, agent))

    #c) step agents
    for agent in agents:
        agent.agent.step() # allows the agents to do what they want as per orderbook state

    #d) match and collect trades from this step...
    trades = real_mgr.process_orders()
    for tr in trades: #process trades individually
        buyer  = order_map.get(tr.buy_order_id)
        seller = order_map.get(tr.sell_order_id)

        #update statistics for buyer and seller to ensure everything outputted in metrics.csv
        if buyer:
            buyer.pnl         -= tr.quantity * tr.price
            buyer.pnl         -= config.FEE_PER_SHARE * tr.quantity
            buyer.inventory   += tr.quantity
            trade_counts[buyer.client_id] += 1

        if seller:
            seller.pnl        += tr.quantity * tr.price
            seller.pnl        -= config.FEE_PER_SHARE * tr.quantity
            seller.inventory  -= tr.quantity
            trade_counts[seller.client_id] += 1

        print(f"[step {step}] TRADE {tr.quantity}@{tr.price} "
              f"(buy:{tr.buy_order_id}, sell:{tr.sell_order_id})")

    #record aforementioned metrics
    steps.append(step)
    for ag in agents:
        pnl_history[ag.client_id].append(ag.pnl)
        inv_history[ag.client_id].append(ag.inventory)
        bid = real_mgr.get_best_bid(ag.symbol)
        ask = real_mgr.get_best_ask(ag.symbol)
        mid = (bid + ask) / 2
        nav_history[ag.client_id].append(ag.pnl + ag.inventory * mid)

#Summaries...
print("\n--- Final P&L, Inventory, Trades, Return/Trade ---")
for ag in agents:
    cnt   = trade_counts[ag.client_id]
    pnl   = ag.pnl
    rpt   = (pnl / cnt) if cnt > 0 else float("nan")
    print(f"{ag.client_id:<8}  P&L={pnl:8.2f}  Inv={ag.inventory:3d}  "
          f"Trades={cnt:3d}  Return/Trade={rpt:8.2f}")

#Summary pt2 for graphs
df = pd.DataFrame({"step": steps})
for cid in pnl_history:
    df[f"{cid}_pnl"]  = pnl_history[cid]
    df[f"{cid}_inv"]  = inv_history[cid]
    df[f"{cid}_nav"]  = nav_history[cid]

out_path = os.path.join(here, "metrics.csv")
df.to_csv(out_path, index=False)
print(f"All P&L, inventory & NAV history to {out_path}")
