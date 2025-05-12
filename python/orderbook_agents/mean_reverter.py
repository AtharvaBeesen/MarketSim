import orderbook
from collections import deque
import config

#------------- One of 6 main agents
#Essentially only trades when |mid − avg| > threshold*avg + total cost
#Uses market orders -> wants immediacy


class MeanReverterAgent:
    def __init__(self, manager, symbol,
                 lookback: int = 10,
                 threshold: float = 0.005,
                 size: int = 1,
                 client_id: str = "mr"):

        self.mgr        = manager
        self.symbol     = symbol
        self.lookback   = lookback
        self.threshold  = threshold
        self.size       = size
        self.client_id  = client_id
        self.counter    = 0
        self.history    = deque(maxlen=lookback+1)

        self.fpo = config.FEE_PER_ORDER
        self.fps = config.FEE_PER_SHARE

    def step(self):
        #first check mid‐price
        bid = self.mgr.get_best_bid(self.symbol)
        ask = self.mgr.get_best_ask(self.symbol)
        mid = (bid + ask) / 2
        self.history.append(mid)


        if len(self.history) < self.lookback + 1: #wait until we have enough history
            return []

        past = list(self.history)[:-1]
        avg  = sum(past) / len(past)  #calc rolling average (excluding newest mid)

        deviation    = abs(mid - avg)
        cost_per_share = self.fps
        cost_per_order = self.fpo / self.size
        total_cost     = cost_per_share + cost_per_order

        #as per above description, agent requires deviation ≥ threshold*avg + total_cost
        if deviation < (self.threshold * avg + total_cost):
            return []

        #choose side by sign of (mid − avg)
        if mid > avg:
            side = orderbook.Side.SELL
        else:
            side = orderbook.Side.BUY

        oid = f"{self.client_id}-{self.counter}"
        self.counter += 1

        order = orderbook.Order(
            oid, self.client_id, self.symbol,
            side,
            0.0,
            self.size,
            orderbook.OrderType.MARKET
        ) #send market order
        self.mgr.place_order(order)
        return [order]
