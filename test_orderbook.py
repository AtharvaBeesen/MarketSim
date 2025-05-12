import os
import glob
import importlib.util

#---------- Testing file, ignore --------

#Finding extension...
here      = os.path.dirname(os.path.abspath(__file__))
build     = os.path.join(here, "build")
release   = os.path.join(build, "Release")

for ext_dir in (release, build):
    if os.path.isdir(ext_dir):
        break
else:
    raise RuntimeError(f"No build folder found in {here!r}")

candidates = glob.glob(os.path.join(ext_dir, "orderbook*.pyd"))
if not candidates:
    raise FileNotFoundError(f"No orderbook*.pyd in {ext_dir!r}")
pyd_path = candidates[0]

#Dynamically load orderbook module....
spec = importlib.util.spec_from_file_location("orderbook", pyd_path)
orderbook = importlib.util.module_from_spec(spec)
spec.loader.exec_module(orderbook) #Working --> this is method we will replicate for the agents...

#Quick test
mgr = orderbook.OrderBookManager()
mgr.add_order_book("FOO")

b = orderbook.Order("b1", "you", "FOO",
                    orderbook.Side.BUY,
                    1.23, 2, orderbook.OrderType.LIMIT)
s = orderbook.Order("s1", "them", "FOO",
                    orderbook.Side.SELL,
                    1.23, 2, orderbook.OrderType.LIMIT)

mgr.place_order(b)
mgr.place_order(s)

trades = mgr.process_orders()
for t in trades:
    print("TRADE", t.trade_id, t.quantity, "@", t.price)
