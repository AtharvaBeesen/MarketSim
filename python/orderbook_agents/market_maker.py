# python/orderbook_agents/market_maker.py

import orderbook
import config

#---------- One of 6 main agents
#Ensures liquidity in market by cancelling stale quotes and only re-quoting if the net edge is greater than market fees...

class MarketMakerAgent:
    def __init__(self, manager, symbol,
                 spread: float = 1.0,
                 size: int = 1,
                 client_id: str = "mm"):
        self.mgr       = manager
        self.symbol    = symbol
        self.spread    = spread
        self.size      = size
        self.client_id = client_id
        self.fpo = config.FEE_PER_ORDER
        self.fps = config.FEE_PER_SHARE

        self.counter = 0
        self.bid_id  = None
        self.ask_id  = None

    def step(self):
        #a) cancel existing quotes
        if self.bid_id is not None:
            self.mgr.cancel_order(self.symbol, self.bid_id)
        if self.ask_id is not None:
            self.mgr.cancel_order(self.symbol, self.ask_id)

        #b) calc mid and net edge
        best_bid = self.mgr.get_best_bid(self.symbol)
        best_ask = self.mgr.get_best_ask(self.symbol)
        mid      = (best_bid + best_ask) / 2

        half_sp     = self.spread / 2
        gross_edge  = half_sp * self.size
        cost_shares = self.fps * self.size
        net_edge    = gross_edge - cost_shares

        #c) fee-aware, so only quote if we can cover the per-order fee and mid is high enough so bid_price > 0
        if net_edge < self.fpo or mid <= half_sp:#no quotes this round
            self.bid_id = None
            self.ask_id = None
            return []


        #d) new quote ids
        bid_id = f"{self.client_id}-bid-{self.counter}"
        ask_id = f"{self.client_id}-ask-{self.counter}"
        self.counter += 1

        bid_price = mid - half_sp
        ask_price = mid + half_sp

        bid_order = orderbook.Order(
            bid_id, self.client_id, self.symbol,
            orderbook.Side.BUY,
            bid_price, self.size,
            orderbook.OrderType.LIMIT
        ) #building and placing orders
        ask_order = orderbook.Order(
            ask_id, self.client_id, self.symbol,
            orderbook.Side.SELL,
            ask_price, self.size,
            orderbook.OrderType.LIMIT
        )

        self.mgr.place_order(bid_order)
        self.mgr.place_order(ask_order)

        #need to remember IDs for next cancellation
        self.bid_id = bid_id
        self.ask_id = ask_id

        return [bid_order, ask_order]
