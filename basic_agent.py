import os
import glob
import importlib.util

# -------------- Testing file, ignore ----------------

# Load Orderbook extension, required for all agents
# -----------------------------------------------
here    = os.path.dirname(os.path.abspath(__file__))
build   = os.path.join(here, "build")
release = os.path.join(build, "Release")

for ext_dir in (release, build):
    if os.path.isdir(ext_dir):
        break
else:
    raise RuntimeError(f"No build folder found in {here!r}")

candidates = glob.glob(os.path.join(ext_dir, "orderbook*.pyd"))
if not candidates:
    raise FileNotFoundError(f"No orderbook*.pyd in {ext_dir!r}")
pyd_path = candidates[0]

spec = importlib.util.spec_from_file_location("orderbook", pyd_path)
orderbook = importlib.util.module_from_spec(spec)
spec.loader.exec_module(orderbook)


#Basic Agent ------------------------------------------
class SimpleAgent:
    def __init__(self, manager, symbol, client_id, side, price):
        """
        Places a 1‐share limit order at a fixed price each step.
        """
        self.mgr       = manager
        self.symbol    = symbol
        self.client_id = client_id
        self.side      = side
        self.price     = price
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
            1,
            orderbook.OrderType.LIMIT
        )
        self.mgr.place_order(order)


def main(): #Quick baseline check to ensure pybind11 functionality...
    mgr    = orderbook.OrderBookManager()
    symbol = "FOO"
    mgr.add_order_book(symbol)

    buyer  = SimpleAgent(mgr, symbol, "buyer", orderbook.Side.BUY,  100.0)
    seller = SimpleAgent(mgr, symbol, "seller", orderbook.Side.SELL, 100.0)

    for step in range(5):
        buyer.step()
        seller.step()
        trades = mgr.process_orders()
        if trades:
            for t in trades:
                print(f"[step {step}] TRADE {t.quantity}@{t.price}  "
                      f"(buy:{t.buy_order_id}, sell:{t.sell_order_id})")
        else:
            print(f"[step {step}] No trades this round")


if __name__ == "__main__":
    main()
