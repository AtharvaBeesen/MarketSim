#pragma once

#include <string>
#include <chrono>

struct Trade {
    std::string trade_id;
    std::string symbol;
    double price;
    int quantity;
    std::string buy_order_id;
    std::string sell_order_id;
    uint64_t timestamp;

    // Constructor
    Trade(const std::string& trade_id,
          const std::string& symbol,
          double price,
          int quantity,
          const std::string& buy_order_id,
          const std::string& sell_order_id)
        : trade_id(trade_id),
          symbol(symbol),
          price(price),
          quantity(quantity),
          buy_order_id(buy_order_id),
          sell_order_id(sell_order_id),
          timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch()).count()) {}

    // Default constructor
    Trade() = default;
}; 