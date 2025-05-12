#pragma once

#include "OrderBook.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

class OrderBookManager {
public:
    OrderBookManager() = default;


    //Main tools to manage Symbols
    void addOrderBook(const std::string& symbol);
    bool hasOrderBook(const std::string& symbol) const;
    void removeOrderBook(const std::string& symbol);

    //-------Wrappers for individual orderbooks----------

    void placeOrder(const Order& order);
    void cancelOrder(const std::string& symbol, const std::string& order_id);
    std::vector<Trade> processOrders();

    double getBestBid(const std::string& symbol) const;
    double getBestAsk(const std::string& symbol) const;
    int getBidSize(const std::string& symbol) const;
    int getAskSize(const std::string& symbol) const;
    std::vector<std::pair<double, int>> getBidDepth(const std::string& symbol, int levels) const;
    std::vector<std::pair<double, int>> getAskDepth(const std::string& symbol, int levels) const;

    bool hasOrder(const std::string& symbol, const std::string& order_id) const;
    const Order* getOrder(const std::string& symbol, const std::string& order_id) const;

private:
    //Map of symbol to OrderBook
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> orderbooks_;

    //Helper methods
    OrderBook* getOrderBook(const std::string& symbol);
    const OrderBook* getOrderBook(const std::string& symbol) const;
}; 