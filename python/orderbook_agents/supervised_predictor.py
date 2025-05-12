import orderbook
import numpy as np
from sklearn.linear_model import SGDClassifier
from collections import deque
import config


#--------- One of 6 main agents --------- Utilizes a logistic regression to predict:
# - Uses the last `lookback` mid‐price returns as features
# - Trains one step behind (partial_fit) w. label = next‐step up/down
# - Only trades when expected edge > fees

class SupervisedPredictorAgent:
    def __init__(self, manager, symbol,
                 lookback: int = 5,
                 size: int = 1,
                 client_id: str = "sp"):
        self.mgr       = manager
        self.symbol    = symbol
        self.lookback  = lookback
        self.size      = size
        self.client_id = client_id

        self.history   = deque(maxlen=lookback+2) #create siliding window of mids
        self.model     = SGDClassifier(loss="log_loss", learning_rate="optimal")
        self.is_fitted = False
        self.counter   = 0

        self.fpo = config.FEE_PER_ORDER
        self.fps = config.FEE_PER_SHARE

    def step(self):
        #a) first check current mid
        b = self.mgr.get_best_bid(self.symbol)
        a = self.mgr.get_best_ask(self.symbol)
        mid = (b + a) / 2
        self.history.append(mid)

        orders = []
        #b) Once we have lookback+2 mids, can finally actually train & predict...
        if len(self.history) >= self.lookback + 2:
            past = list(self.history)[:-1]   #drop newest
            feats = np.array([
                (past[i+1] - past[i]) / past[i]
                for i in range(len(past)-1)
            ]).reshape(1, -1)
            label = int(past[-1] < self.history[-1])

            #partial_fit
            if not self.is_fitted:
                self.model.partial_fit(feats, [label], classes=[0,1])
                self.is_fitted = True
            else:
                self.model.partial_fit(feats, [label])

            pred = self.model.predict(feats)[0]
            avg_ret       = float(np.mean(np.abs(feats)))
            expected_edge = avg_ret * mid * self.size
            total_cost    = self.fps * self.size + self.fpo

            #fee-adjust...
            #Now only trade if expected edge covers fees (optimal)
            if expected_edge >= total_cost:
                side = orderbook.Side.BUY if pred == 1 else orderbook.Side.SELL
                oid  = f"{self.client_id}-{self.counter}"
                self.counter += 1

                order = orderbook.Order(
                    oid, self.client_id, self.symbol,
                    side, 0.0, self.size,
                    orderbook.OrderType.MARKET
                )
                self.mgr.place_order(order)
                orders.append(order)

        return orders
