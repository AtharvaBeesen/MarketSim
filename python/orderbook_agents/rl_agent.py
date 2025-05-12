import orderbook
import numpy as np
from collections import defaultdict

#--------- One of 6 main agents -------
#Relatively simply, uses v timple tabular Q-learning methods
# - Discrete the one‐step return into {down, flat, up}
# - Has 3 actions: 0=no‐op, 1=market buy, 2=market sell
# - Uses e-greedy and updates Q w. reward = P&L from mid‐price moves

class RLAgent:
    def __init__(self, manager, symbol,
                 alpha: float   = 0.1,
                 gamma: float   = 0.9,
                 epsilon: float = 0.1,
                 size: int      = 1,
                 client_id: str = "rl"):
        self.mgr        = manager
        self.symbol     = symbol
        self.alpha      = alpha
        self.gamma      = gamma
        self.epsilon    = epsilon
        self.size       = size
        self.client_id  = client_id

        #Q‐table: state (0,1,2) --> 3 actions
        self.actions    = [0, 1, 2]  #[no‐op, buy, sell]
        self.q_table    = defaultdict(lambda: np.zeros(len(self.actions)))

        self.prev_state  = None
        self.prev_mid    = None
        self.prev_action = None
        self.counter     = 0

    def _get_mid(self):
        b = self.mgr.get_best_bid(self.symbol)
        a = self.mgr.get_best_ask(self.symbol)
        return (b + a) / 2

    def _discretize(self, ret: float) -> int:
        #ret < -0.001 -> 0; |ret| <= 0.001 -> 1; ret > 0.001 -> 2
        if ret < -0.001:
            return 0
        if ret > 0.001:
            return 2
        return 1

    def step(self):
        #a) first, observe current mid & compute state
        mid = self._get_mid()
        if self.prev_mid is None or self.prev_mid == 0.0: #block against zeroes and missing data
            ret = 0.0
        else:
            ret = (mid - self.prev_mid) / self.prev_mid

        state = self._discretize(ret)

        #b) now, if we have a previous state/action, calc reward & update Q
        if self.prev_state is not None and self.prev_action in (1,2):
            #P&L = (mid - prev_mid) * position
            pos = 1 if self.prev_action == 1 else -1
            reward = pos * (mid - self.prev_mid)
            # Q‐learning update....
            best_next = np.max(self.q_table[state])
            old_q     = self.q_table[self.prev_state][self.prev_action]
            self.q_table[self.prev_state][self.prev_action] = (
                old_q + self.alpha * (reward + self.gamma * best_next - old_q)
            )

        if np.random.rand() < self.epsilon: #epsilon greedy selection:
            action = np.random.choice(self.actions)
        else:
            action = int(np.argmax(self.q_table[state]))

        orders = []
        #Implement actions to market orders...
        if action == 1:   #buy
            oid = f"{self.client_id}-{self.counter}"
            self.counter += 1
            o = orderbook.Order(
                oid, self.client_id, self.symbol,
                orderbook.Side.BUY,
                0.0, self.size,
                orderbook.OrderType.MARKET
            )
            self.mgr.place_order(o)
            orders.append(o)
        elif action == 2: #sell
            oid = f"{self.client_id}-{self.counter}"
            self.counter += 1
            o = orderbook.Order(
                oid, self.client_id, self.symbol,
                orderbook.Side.SELL,
                0.0, self.size,
                orderbook.OrderType.MARKET
            )
            self.mgr.place_order(o)
            orders.append(o)
        #else action == 0: (no orders)

        #save info for the next update...
        self.prev_mid    = mid
        self.prev_state  = state
        self.prev_action = action

        return orders
