import orderbook
import random
import config


#--------One of 6 main agents----------
#Essentially, with probability = order_prob, send a MARKET order only if half-spread can cover fees...

class LiquidityTakerAgent:
    def __init__(self, manager, symbol,
                 order_prob: float = 0.1,
                 size_dist: tuple[int,int] = (1,5),
                 client_id: str = "lt"):
        self.mgr        = manager
        self.symbol     = symbol
        self.order_prob = order_prob
        self.size_dist  = size_dist
        self.client_id  = client_id
        self.counter    = 0

        #Initialize fees (all agents are fee-aware)
        self.fpo = config.FEE_PER_ORDER
        self.fps = config.FEE_PER_SHARE

    def step(self):
        #a) randomization: coin‐flip for willingness to trade
        if random.random() > self.order_prob:
            return []

        #b) peek at spread to compute half‐spread...
        best_bid = self.mgr.get_best_bid(self.symbol)
        best_ask = self.mgr.get_best_ask(self.symbol)
        half_spread = (best_ask - best_bid) / 2

        #c) pick size *before cost, bcos fee_per_order is per‐order
        size = random.randint(self.size_dist[0], self.size_dist[1])

        #d) calc total cost per share + per‐order (amortized)
        cost_per_share = self.fps
        cost_per_order = self.fpo / size
        total_cost     = cost_per_share + cost_per_order

        #e) only trade if half_spread ≥ total_cost (i.e. agent expects to clear fees)
        if half_spread < total_cost:
            return []

        #d) choose side & create order...
        side = random.choice([orderbook.Side.BUY, orderbook.Side.SELL])
        oid  = f"{self.client_id}-{self.counter}"
        self.counter += 1

        order = orderbook.Order(
            oid, self.client_id, self.symbol,
            side,
            0.0,               #remember, price ignored for MARKET
            size,
            orderbook.OrderType.MARKET
        )
        self.mgr.place_order(order)
        return [order]
