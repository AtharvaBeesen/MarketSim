# python/orderbook_agents/trend_follower.py

import orderbook
import config

#------------ One of 6 main agents
#Essentially only trades when recent return x mid x size > total cost...


class TrendFollowerAgent:
    def __init__(self, manager, symbol,
                 lookback: int = 3,
                 threshold: float = 0.002,
                 size: int = 1,
                 client_id: str = "tf"):
        self.mgr        = manager
        self.symbol     = symbol
        self.lookback   = lookback
        self.threshold  = threshold
        self.size       = size
        self.client_id  = client_id

        self.counter    = 0
        self.history    = []  #historical mids

        self.fpo = config.FEE_PER_ORDER
        self.fps = config.FEE_PER_SHARE

    def step(self):
        b = self.mgr.get_best_bid(self.symbol)
        a = self.mgr.get_best_ask(self.symbol)
        mid = (b + a) / 2
        self.history.append(mid)

        if len(self.history) <= self.lookback:
            return []

        prev = self.history[-(self.lookback+1)]
        if prev <= 0.0: #prevent against zero or missing previous mid
            return []
        ret  = (mid - prev) / prev

        #remember, only act if return exceeds both your threshold and fees...
        gross_edge = abs(ret) * mid * self.size
        total_cost = self.fps * self.size + self.fpo

        gross_edge = abs(ret) * mid * self.size
        total_cost = self.fps * self.size + self.fpo
        if abs(ret) < self.threshold or gross_edge < total_cost:
            return []

        side = orderbook.Side.BUY if ret > 0 else orderbook.Side.SELL # decide side by the sign of return

        oid = f"{self.client_id}-{self.counter}"
        self.counter += 1

        order = orderbook.Order(
            oid, self.client_id, self.symbol,
            side, 0.0, self.size,
            orderbook.OrderType.MARKET
        )
        self.mgr.place_order(order)
        return [order]
