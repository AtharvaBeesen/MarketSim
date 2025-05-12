#pragma once

#include <string>
#include <chrono>
#include <stdexcept>

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET };

struct Order {
    std::string order_id;
    std::string client_id;
    std::string symbol;
    Side side;
    double price;
    int quantity;
    OrderType type;
    uint64_t timestamp;

    //Constructor
    Order(const std::string& order_id,
          const std::string& client_id,
          const std::string& symbol,
          Side side,
          double price,
          int quantity,
          OrderType type = OrderType::LIMIT)
        : order_id(order_id),
          client_id(client_id),
          symbol(symbol),
          side(side),
          price(price),
          quantity(quantity),
          type(type),
          timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch()).count()) {
        if (type == OrderType::LIMIT && price <= 0) {
            throw std::runtime_error("Limit order price must be positive");
        }
        if (type == OrderType::MARKET && price != 0) {
            throw std::runtime_error("Market order price must be 0");
        }
        if (quantity <= 0) {
            throw std::runtime_error("Order quantity must be positive");
        }
    }

    //Default constructor
    Order() = default;

    //Helper methods
    bool isBuy() const { return side == Side::BUY; }
    bool isSell() const { return side == Side::SELL; }
    bool isLimit() const { return type == OrderType::LIMIT; }
    bool isMarket() const { return type == OrderType::MARKET; }
}; 