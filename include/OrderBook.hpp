#pragma once

#include "Order.hpp"
#include "Trade.hpp"
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

class OrderBook {
public:
    explicit OrderBook(const std::string& symbol);

    //Main Orderbook ops
    void addOrder(const Order& order);
    void cancelOrder(const std::string& order_id);
    std::vector<Trade> matchOrders();
    void clear();

    //Market data queries for agents
    double getBestBid() const;
    double getBestAsk() const;
    int getBidSize() const;
    int getAskSize() const;
    std::vector<std::pair<double, int>> getBidDepth(int levels) const;
    std::vector<std::pair<double, int>> getAskDepth(int levels) const;

    //Helper methods
    bool hasOrder(const std::string& order_id) const;
    const Order* getOrder(const std::string& order_id) const;
    const std::string& getSymbol() const { return symbol_; }

private:
    std::string symbol_;
    
    // Price levels for bids and asks --> ordered levels with map
    std::map<double, std::list<Order>, std::greater<double>> bids_; //Highest First
    std::map<double, std::list<Order>, std::less<double>> asks_; //Lowest First
    
    //Fast order lookup
    std::unordered_map<std::string, 
        std::pair<Side, std::list<Order>::iterator>> order_lookup_;

    std::vector<Trade> pending_trades_; //Helps with matching

    //Helper methods
    void addOrderToBook(const Order& order);
    void removeOrderFromBook(const std::string& order_id);
    void processMarketOrder(const Order& order);
    std::string generateTradeId() const;
    void addTrade(const Trade& trade);
}; 