#include "OrderBook.hpp"
#include "OrderBookManager.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

//-------------TESTING FILE---------- IGNORE


void printMarketData(const OrderBookManager& manager, const std::vector<std::string>& symbols) {
    std::cout << "\nMarket Data:\n";
    for (const auto& symbol : symbols) {
        std::cout << symbol << " - Best Bid: " << manager.getBestBid(symbol)
                  << ", Best Ask: " << manager.getBestAsk(symbol) << "\n";
    }
}

void printTrades(const std::vector<Trade>& trades) {
    std::cout << "\nTrades:\n";
    for (const auto& trade : trades) {
        std::cout << "Trade ID: " << trade.trade_id
                  << ", Symbol: " << trade.symbol
                  << ", Price: " << trade.price
                  << ", Quantity: " << trade.quantity
                  << ", Buy Order: " << trade.buy_order_id
                  << ", Sell Order: " << trade.sell_order_id << "\n";
    }
}

void printOrderBookDepth(const OrderBookManager& manager, const std::string& symbol) {
    std::cout << "\nOrderbook Depth (" << symbol << "):\n";
    auto bid_depth = manager.getBidDepth(symbol, 5);
    auto ask_depth = manager.getAskDepth(symbol, 5);

    std::cout << "Bids:\n";
    for (const auto& [price, quantity] : bid_depth) {
        std::cout << "Price: " << std::setw(8) << price
                  << ", Quantity: " << quantity << "\n";
    }

    std::cout << "Asks:\n";
    for (const auto& [price, quantity] : ask_depth) {
        std::cout << "Price: " << std::setw(8) << price
                  << ", Quantity: " << quantity << "\n";
    }
}

int main() { //TESTS
    try {
        //Create orderbook manager
        OrderBookManager manager;
        std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL"};

        //Add orderbooks for different symbols
        for (const auto& symbol : symbols) {
            manager.addOrderBook(symbol);
        }

        std::cout << "=== Test 1: Basic Order Matching ===\n";
        //First I'll create and place some basic orders
        Order buy_order1("order1", "client1", "AAPL", Side::BUY, 100.0, 100);
        Order sell_order1("order2", "client2", "AAPL", Side::SELL, 100.0, 50);
        manager.placeOrder(buy_order1);
        manager.placeOrder(sell_order1);
        std::vector<Trade> trades1 = manager.processOrders();
        printTrades(trades1);
        printMarketData(manager, symbols);
        printOrderBookDepth(manager, "AAPL");

        //Next, I'll clear order books before next test
        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 2: Multiple Price Levels ===\n";
        //Add orders at diff price levels
        Order buy_order2("order3", "client3", "MSFT", Side::BUY, 200.0, 100);
        Order buy_order3("order4", "client4", "MSFT", Side::BUY, 195.0, 50);
        Order sell_order2("order5", "client5", "MSFT", Side::SELL, 205.0, 75);
        Order sell_order3("order6", "client6", "MSFT", Side::SELL, 210.0, 25);
        
        manager.placeOrder(buy_order2);
        manager.placeOrder(buy_order3);
        manager.placeOrder(sell_order2);
        manager.placeOrder(sell_order3);
        std::vector<Trade> trades2 = manager.processOrders();
        printTrades(trades2);
        printMarketData(manager, symbols);
        printOrderBookDepth(manager, "MSFT");

        //Clear order books before next test
        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 3: Market Buy Order Execution ===\n";
        //Now test market buy order execution
        Order limit_sell1("order9", "client9", "GOOGL", Side::SELL, 150.0, 100);
        Order market_buy1("order7", "client7", "GOOGL", Side::BUY, 0.0, 100, OrderType::MARKET);
        
        //Place limit sell first to ensure liquidity (will change in true simulation to have initial liquidity)
        manager.placeOrder(limit_sell1);
        
        //Place and process market buy
        manager.placeOrder(market_buy1);
        std::vector<Trade> trades3 = manager.processOrders();
        printTrades(trades3);
        printMarketData(manager, symbols);
        printOrderBookDepth(manager, "GOOGL");

        //Clear order books before next test
        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 3.1: Market Sell with No Liquidity ===\n";
        //Test market sell order with no liquidity -> Ensure errors function properly
        Order market_sell1("order8", "client8", "GOOGL", Side::SELL, 0.0, 100, OrderType::MARKET);
        
        try {
            manager.placeOrder(market_sell1);
            std::vector<Trade> trades3_1 = manager.processOrders();
            printTrades(trades3_1);
        } catch (const std::exception& e) {
            std::cout << "Expected error: " << e.what() << "\n";
        }
        printMarketData(manager, symbols);
        printOrderBookDepth(manager, "GOOGL");

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 4: Order Cancellation ===\n";
        //Now need to check cancellation mechanism
        Order buy_order4("order10", "client10", "AAPL", Side::BUY, 105.0, 100);
        manager.placeOrder(buy_order4);
        printOrderBookDepth(manager, "AAPL");
        
        manager.cancelOrder("AAPL", "order10");
        std::cout << "\nAfter cancelling order10:\n";
        printOrderBookDepth(manager, "AAPL");

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 5: Error Cases ===\n";
        try {
            //Error checking
            manager.cancelOrder("AAPL", "nonexistent");
        } catch (const std::exception& e) {
            std::cout << "Expected error: " << e.what() << "\n";
        }

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        try {
            //Error checking pt2
            Order invalid_order("order11", "client11", "INVALID", Side::BUY, 100.0, 100);
            manager.placeOrder(invalid_order);
        } catch (const std::exception& e) {
            std::cout << "Expected error: " << e.what() << "\n";
        }

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 6: Price Validation ===\n";
        try {
            Order invalid_price("order12", "client12", "AAPL", Side::BUY, -100.0, 100);
            manager.placeOrder(invalid_price);
        } catch (const std::exception& e) {
            std::cout << "Expected error: " << e.what() << "\n";
        }

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 7: Quantity Validation ===\n";
        try {
            Order invalid_quantity("order13", "client13", "AAPL", Side::BUY, 100.0, 0);
            manager.placeOrder(invalid_quantity);
        } catch (const std::exception& e) {
            std::cout << "Expected error: " << e.what() << "\n";
        }

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 8: Market Order Validation ===\n";
        try {
            //Limit sell order
            Order limit_sell2("order14", "client14", "AAPL", Side::SELL, 100.0, 50);
            manager.placeOrder(limit_sell2);
            
            //Place market buy order
            Order invalid_market("order15", "client15", "AAPL", Side::BUY, 100.0, 50, OrderType::MARKET);
            manager.placeOrder(invalid_market);
        } catch (const std::exception& e) {
            std::cout << "Expected error: " << e.what() << "\n";
        }

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 9: Market Order Execution ===\n";
        //Limit sell order pt2
        Order limit_sell3("order16", "client16", "AAPL", Side::SELL, 100.0, 50);
        manager.placeOrder(limit_sell3);
        
        //Matching market buy order
        Order market_buy2("order17", "client17", "AAPL", Side::BUY, 0.0, 50, OrderType::MARKET);
        manager.placeOrder(market_buy2);
        
        std::vector<Trade> trades9 = manager.processOrders();
        printTrades(trades9);
        printMarketData(manager, symbols);
        printOrderBookDepth(manager, "AAPL");

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 10: Partial Fills and Remaining Quantity ===\n";
        //Test out partial fills mechanism
        Order large_buy("order18", "client18", "AAPL", Side::BUY, 100.0, 200);
        Order small_sell1("order19", "client19", "AAPL", Side::SELL, 100.0, 50);
        Order small_sell2("order20", "client20", "AAPL", Side::SELL, 100.0, 75);
        
        manager.placeOrder(large_buy);
        manager.placeOrder(small_sell1);
        manager.placeOrder(small_sell2);
        std::vector<Trade> trades10 = manager.processOrders();
        printTrades(trades10);
        printOrderBookDepth(manager, "AAPL");

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 11: Multiple Trades at Same Price Level ===\n";
        //Test at same price
        Order buy1("order21", "client21", "MSFT", Side::BUY, 150.0, 100);
        Order buy2("order22", "client22", "MSFT", Side::BUY, 150.0, 50);
        Order sell1("order23", "client23", "MSFT", Side::SELL, 150.0, 75);
        Order sell2("order24", "client24", "MSFT", Side::SELL, 150.0, 100);
        
        manager.placeOrder(buy1);
        manager.placeOrder(buy2);
        manager.placeOrder(sell1);
        manager.placeOrder(sell2);
        std::vector<Trade> trades11 = manager.processOrders();
        printTrades(trades11);
        printOrderBookDepth(manager, "MSFT");

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 12: Order Book Depth with Many Levels ===\n";
        //Depth testing...
        for (int i = 0; i < 10; i++) {
            Order buy("buy" + std::to_string(i), "client" + std::to_string(i), "GOOGL", 
                     Side::BUY, 100.0 + i, 100);
            Order sell("sell" + std::to_string(i), "client" + std::to_string(i), "GOOGL", 
                      Side::SELL, 110.0 + i, 100);
            manager.placeOrder(buy);
            manager.placeOrder(sell);
        }
        printOrderBookDepth(manager, "GOOGL");

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 13: Order Lookup Functionality ===\n";
        //Order lookup mechanism
        Order test_order("order25", "client25", "AAPL", Side::BUY, 100.0, 100);
        manager.placeOrder(test_order);
        
        if (manager.hasOrder("AAPL", "order25")) {
            const Order* found_order = manager.getOrder("AAPL", "order25");
            std::cout << "Found order: ID=" << found_order->order_id 
                      << ", Price=" << found_order->price 
                      << ", Quantity=" << found_order->quantity << "\n";
        }


        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 14: Multi-Symbol Order Matching ===\n";
        //Test manager functionality with multi order matching
        Order aapl_buy("order26", "client26", "AAPL", Side::BUY, 100.0, 100);
        Order aapl_sell("order27", "client27", "AAPL", Side::SELL, 100.0, 100);
        Order msft_buy("order28", "client28", "MSFT", Side::BUY, 200.0, 100);
        Order msft_sell("order29", "client29", "MSFT", Side::SELL, 200.0, 100);
        
        manager.placeOrder(aapl_buy);
        manager.placeOrder(aapl_sell);
        manager.placeOrder(msft_buy);
        manager.placeOrder(msft_sell);
        std::vector<Trade> trades14 = manager.processOrders();
        printTrades(trades14);
        printMarketData(manager, symbols);

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 15: Market Order with No Liquidity ===\n";
        //Test when no matching orders -> Error checking
        Order market_buy3("order30", "client30", "AAPL", Side::BUY, 0.0, 100, OrderType::MARKET);
        
        try {
            manager.placeOrder(market_buy3);
            std::vector<Trade> trades15 = manager.processOrders();
            printTrades(trades15);
        } catch (const std::exception& e) {
            std::cout << "Expected error: " << e.what() << "\n";
        }
        printOrderBookDepth(manager, "AAPL");

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 16: Order Book State Persistence ===\n";
        //Test between ops
        Order buy3("order31", "client31", "AAPL", Side::BUY, 100.0, 100);
        Order sell3("order32", "client32", "AAPL", Side::SELL, 100.0, 50);
        
        manager.placeOrder(buy3);
        printOrderBookDepth(manager, "AAPL");
        manager.placeOrder(sell3);
        std::vector<Trade> trades16 = manager.processOrders();
        printTrades(trades16);
        printOrderBookDepth(manager, "AAPL");

        for (const auto& symbol : symbols) {
            manager.removeOrderBook(symbol);
            manager.addOrderBook(symbol);
        }

        std::cout << "\n=== Test 17: Trade ID Uniqueness ===\n";
        //Ensure unique IDs
        Order buy4("order33", "client33", "AAPL", Side::BUY, 100.0, 100);
        Order sell4("order34", "client34", "AAPL", Side::SELL, 100.0, 100);
        Order buy5("order35", "client35", "AAPL", Side::BUY, 100.0, 100);
        Order sell5("order36", "client36", "AAPL", Side::SELL, 100.0, 100);
        
        manager.placeOrder(buy4);
        manager.placeOrder(sell4);
        manager.placeOrder(buy5);
        manager.placeOrder(sell5);
        std::vector<Trade> trades17 = manager.processOrders();
        printTrades(trades17);

    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
        return 1;
    }

    return 0;
} 