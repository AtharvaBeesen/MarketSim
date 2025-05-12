import os


SYMBOLS = ["AAPL", "MSFT", "GOOGL"]  #Add symbols as required

NUM_STEPS = 1000     #No. of time slices in simulation
DT        = 0.1      #seconds per step (i.e 100 ms ticks)

#Transaction costs (lowered)
FEE_PER_ORDER = 0.01
FEE_PER_SHARE = 0.002

#Latency model (how much jitter)
LATENCY_MEAN = 0.02
LATENCY_STD  = 0.01     #10 ms jitter

#Fundamental price drift volatility/step ...
FUND_VOLATILITY = 0.1