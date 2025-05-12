import orderbook

#-------- Testing agent, ignore ----------
#places single limit order (quantity at price) each step

class BasicAgent:
    def __init__(self, manager, symbol,
                 side=orderbook.Side.BUY,
                 price: float = 100.0,
                 quantity: int = 1,
                 client_id: str = "agent"):
        self.mgr       = manager
        self.symbol    = symbol
        self.side      = side
        self.price     = price
        self.quantity  = quantity
        self.client_id = client_id
        self.counter   = 0

    def step(self):
        oid = f"{self.client_id}-{self.counter}"
        self.counter += 1

        order = orderbook.Order(
            oid,
            self.client_id,
            self.symbol,
            self.side,
            self.price,
            self.quantity,
            orderbook.OrderType.LIMIT
        )
        self.mgr.place_order(order)
        return order
