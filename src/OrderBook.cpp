#include "OrderBook.hpp"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

OrderBook::OrderBook(const std::string& symbol)
    : symbol_(symbol) {}

void OrderBook::addOrder(const Order& order) {
    if (order.symbol != symbol_) {
        throw std::runtime_error("Order symbol does not match orderbook symbol");
    }

    if (order.isMarket()) {
        processMarketOrder(order); // Immediately process market orders
    } else {
        addOrderToBook(order); //Limit orders go to book
    }
}

void OrderBook::processMarketOrder(const Order& order) {
    Order working_order = order;
    
    if (working_order.isBuy()) { // Market buy orders match against asks
        if (asks_.empty()) {
            throw std::runtime_error("No liquidity available for market buy order");
        }
        while (!asks_.empty() && working_order.quantity > 0) {
            auto best_ask = asks_.begin();
            auto& ask_orders = best_ask->second;
            
            while (!ask_orders.empty() && working_order.quantity > 0) {
                Order& ask = ask_orders.front();
                int trade_quantity = std::min(working_order.quantity, ask.quantity);
                double trade_price = ask.price;
                

                Trade trade(
                    generateTradeId(),
                    symbol_,
                    trade_price,
                    trade_quantity,
                    working_order.order_id,
                    ask.order_id
                ); // Initialize trade
                pending_trades_.push_back(trade);
                
                working_order.quantity -= trade_quantity;
                ask.quantity -= trade_quantity;
                
                if (ask.quantity == 0) {
                    order_lookup_.erase(ask.order_id);
                    ask_orders.pop_front();
                } // Remove executed orders
            }
            
            //Clean up
            if (ask_orders.empty()) {
                asks_.erase(best_ask);
            }
        }
    } else {
        if (bids_.empty()) {
            throw std::runtime_error("No liquidity available for market sell order"); // For market sell orders -> match against bids
        }
        // Same methodology as above...
        while (!bids_.empty() && working_order.quantity > 0) {
            auto best_bid = bids_.begin();
            auto& bid_orders = best_bid->second;
            
            while (!bid_orders.empty() && working_order.quantity > 0) {
                Order& bid = bid_orders.front();
                int trade_quantity = std::min(working_order.quantity, bid.quantity);
                double trade_price = bid.price;
                
                Trade trade(
                    generateTradeId(),
                    symbol_,
                    trade_price,
                    trade_quantity,
                    bid.order_id,
                    working_order.order_id
                );
                pending_trades_.push_back(trade);
                
                working_order.quantity -= trade_quantity;
                bid.quantity -= trade_quantity;
                
                if (bid.quantity == 0) {
                    order_lookup_.erase(bid.order_id);
                    bid_orders.pop_front();
                }
            }
            
            // Clean up empty price levels
            if (bid_orders.empty()) {
                bids_.erase(best_bid);
            }
        }
    }
    
    //If there's remaining quantity, reject order:
    if (working_order.quantity > 0) {
        throw std::runtime_error("Insufficient liquidity for market order");
    }
}

void OrderBook::cancelOrder(const std::string& order_id) {
    if (!hasOrder(order_id)) {
        throw std::runtime_error("Order not found");
    }
    removeOrderFromBook(order_id);
}

std::vector<Trade> OrderBook::matchOrders() { //Process all orders in the book
    std::vector<Trade> all_trades = std::move(pending_trades_);
    pending_trades_.clear();
    
    while (!bids_.empty() && !asks_.empty()) {
        auto best_bid = bids_.begin();
        auto best_ask = asks_.begin();
        
        if (best_bid->first >= best_ask->first) {
            // We have a match!
            auto& bid_orders = best_bid->second;
            auto& ask_orders = best_ask->second;
            
            while (!bid_orders.empty() && !ask_orders.empty()) {
                Order& bid = bid_orders.front();
                Order& ask = ask_orders.front();
                
                int trade_quantity = std::min(bid.quantity, ask.quantity);
                double trade_price = ask.price; //Use ask price as execution price
                
                Trade trade(
                    generateTradeId(),
                    symbol_,
                    trade_price,
                    trade_quantity,
                    bid.order_id,
                    ask.order_id
                ); // initialize and add trade to all_trades
                all_trades.push_back(trade);
                
                //Updates
                bid.quantity -= trade_quantity;
                ask.quantity -= trade_quantity;
                
                //Remove fully executed orders
                if (bid.quantity == 0) {
                    order_lookup_.erase(bid.order_id);
                    bid_orders.pop_front();
                }
                if (ask.quantity == 0) {
                    order_lookup_.erase(ask.order_id);
                    ask_orders.pop_front();
                }
            }
            
            //Clean up
            if (bid_orders.empty()) {
                bids_.erase(best_bid);
            }
            if (ask_orders.empty()) {
                asks_.erase(best_ask);
            }
        } else {
            break; //All fully matched up, nothing else left
        }
    }
    
    return all_trades;
}

//------------ Helper methods ----------------

double OrderBook::getBestBid() const {
    return bids_.empty() ? 0.0 : bids_.begin()->first;
}

double OrderBook::getBestAsk() const {
    return asks_.empty() ? 0.0 : asks_.begin()->first;
}

int OrderBook::getBidSize() const {
    int total = 0;
    for (const auto& [price, orders] : bids_) {
        for (const auto& order : orders) {
            total += order.quantity;
        }
    }
    return total;
}

int OrderBook::getAskSize() const {
    int total = 0;
    for (const auto& [price, orders] : asks_) {
        for (const auto& order : orders) {
            total += order.quantity;
        }
    }
    return total;
}

std::vector<std::pair<double, int>> OrderBook::getBidDepth(int levels) const {
    std::vector<std::pair<double, int>> depth;
    int count = 0;
    
    for (const auto& [price, orders] : bids_) {
        if (count >= levels) break;
        
        int total_quantity = 0;
        for (const auto& order : orders) {
            total_quantity += order.quantity;
        }
        
        depth.emplace_back(price, total_quantity);
        count++;
    }
    
    return depth;
}

std::vector<std::pair<double, int>> OrderBook::getAskDepth(int levels) const {
    std::vector<std::pair<double, int>> depth;
    int count = 0;
    
    for (const auto& [price, orders] : asks_) {
        if (count >= levels) break;
        
        int total_quantity = 0;
        for (const auto& order : orders) {
            total_quantity += order.quantity;
        }
        
        depth.emplace_back(price, total_quantity);
        count++;
    }
    
    return depth;
}

bool OrderBook::hasOrder(const std::string& order_id) const {
    return order_lookup_.find(order_id) != order_lookup_.end();
}

const Order* OrderBook::getOrder(const std::string& order_id) const {
    auto it = order_lookup_.find(order_id);
    if (it == order_lookup_.end()) {
        return nullptr;
    }
    return &(*it->second.second);
}

void OrderBook::addOrderToBook(const Order& order) {
    if (order.isBuy()) {
        auto& price_level = bids_[order.price];
        price_level.push_back(order);
        order_lookup_[order.order_id] = std::make_pair(order.side, --price_level.end());
    } else {
        auto& price_level = asks_[order.price];
        price_level.push_back(order);
        order_lookup_[order.order_id] = std::make_pair(order.side, --price_level.end());
    }
}

void OrderBook::removeOrderFromBook(const std::string& order_id) {
    auto it = order_lookup_.find(order_id);
    if (it == order_lookup_.end()) {
        throw std::runtime_error("Order not found");
    }

    auto& [side, order_it] = it->second;
    if (side == Side::BUY) {
        auto price = order_it->price;
        bids_[price].erase(order_it);
        if (bids_[price].empty()) {
            bids_.erase(price);
        }
    } else {
        auto price = order_it->price;
        asks_[price].erase(order_it);
        if (asks_[price].empty()) {
            asks_.erase(price);
        }
    }
    order_lookup_.erase(it);
}

std::string OrderBook::generateTradeId() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex_digits = "0123456789ABCDEF";
    
    std::stringstream ss;
    ss << symbol_ << "-";
    for (int i = 0; i < 8; ++i) {
        ss << hex_digits[dis(gen)];
    }
    return ss.str();
}

void OrderBook::clear() {
    bids_.clear();
    asks_.clear();
    order_lookup_.clear();
    pending_trades_.clear();
} 