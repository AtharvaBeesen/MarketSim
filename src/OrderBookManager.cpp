#include "OrderBookManager.hpp"
#include <stdexcept>

//----------- Essentially a wrapper to help manage multiple orderbooks ---------
//----------- Most functions just call their relevant symbol's orderbook -------

void OrderBookManager::addOrderBook(const std::string& symbol) {
    if (hasOrderBook(symbol)) {
        throw std::runtime_error("Orderbook already exists for symbol: " + symbol);
    }
    orderbooks_[symbol] = std::make_unique<OrderBook>(symbol);
}

bool OrderBookManager::hasOrderBook(const std::string& symbol) const {
    return orderbooks_.find(symbol) != orderbooks_.end();
}

void OrderBookManager::removeOrderBook(const std::string& symbol) {
    if (!hasOrderBook(symbol)) {
        throw std::runtime_error("Orderbook not found for symbol: " + symbol);
    }
    orderbooks_.erase(symbol);
}

void OrderBookManager::placeOrder(const Order& order) {
    auto* orderbook = getOrderBook(order.symbol);
    if (!orderbook) {
        throw std::runtime_error("No orderbook found for symbol: " + order.symbol);
    }
    orderbook->addOrder(order);
}

void OrderBookManager::cancelOrder(const std::string& symbol, const std::string& order_id) {
    auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        throw std::runtime_error("No orderbook found for symbol: " + symbol);
    }
    orderbook->cancelOrder(order_id);
}

std::vector<Trade> OrderBookManager::processOrders() {
    std::vector<Trade> all_trades;
    for (auto& [symbol, orderbook] : orderbooks_) {
        auto trades = orderbook->matchOrders();
        all_trades.insert(all_trades.end(), trades.begin(), trades.end());
    }
    return all_trades;
}

double OrderBookManager::getBestBid(const std::string& symbol) const {
    const auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        throw std::runtime_error("No orderbook found for symbol: " + symbol);
    }
    return orderbook->getBestBid();
}

double OrderBookManager::getBestAsk(const std::string& symbol) const {
    const auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        throw std::runtime_error("No orderbook found for symbol: " + symbol);
    }
    return orderbook->getBestAsk();
}

int OrderBookManager::getBidSize(const std::string& symbol) const {
    const auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        throw std::runtime_error("No orderbook found for symbol: " + symbol);
    }
    return orderbook->getBidSize();
}

int OrderBookManager::getAskSize(const std::string& symbol) const {
    const auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        throw std::runtime_error("No orderbook found for symbol: " + symbol);
    }
    return orderbook->getAskSize();
}

std::vector<std::pair<double, int>> OrderBookManager::getBidDepth(
    const std::string& symbol, int levels) const {
    const auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        throw std::runtime_error("No orderbook found for symbol: " + symbol);
    }
    return orderbook->getBidDepth(levels);
}

std::vector<std::pair<double, int>> OrderBookManager::getAskDepth(
    const std::string& symbol, int levels) const {
    const auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        throw std::runtime_error("No orderbook found for symbol: " + symbol);
    }
    return orderbook->getAskDepth(levels);
}

bool OrderBookManager::hasOrder(const std::string& symbol, const std::string& order_id) const {
    const auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        return false;
    }
    return orderbook->hasOrder(order_id);
}

const Order* OrderBookManager::getOrder(const std::string& symbol, const std::string& order_id) const {
    const auto* orderbook = getOrderBook(symbol);
    if (!orderbook) {
        return nullptr;
    }
    return orderbook->getOrder(order_id);
}

OrderBook* OrderBookManager::getOrderBook(const std::string& symbol) {
    auto it = orderbooks_.find(symbol);
    return it != orderbooks_.end() ? it->second.get() : nullptr;
}

const OrderBook* OrderBookManager::getOrderBook(const std::string& symbol) const {
    auto it = orderbooks_.find(symbol);
    return it != orderbooks_.end() ? it->second.get() : nullptr;
} 