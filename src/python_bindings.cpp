#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Order.hpp"
#include "Trade.hpp"
#include "OrderBookManager.hpp"

namespace py = pybind11;

//---------- PYBIND11 TO CREATE CPP PYTHON INTERACTION -------------

PYBIND11_MODULE(orderbook, m) {
    m.doc() = "Python bindings for the C++ OrderBook engine";

    //Cpp Enums
    py::enum_<Side>(m, "Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL)
        .export_values();

    py::enum_<OrderType>(m, "OrderType")
        .value("LIMIT", OrderType::LIMIT)
        .value("MARKET", OrderType::MARKET)
        .export_values();

    //Bind the 7-arg constructor... (No timestamp)
    py::class_<Order>(m, "Order")
        .def(py::init<const std::string&,
                      const std::string&,
                      const std::string&,
                      Side,
                      double,
                      int,
                      OrderType>(),
             py::arg("order_id"),
             py::arg("client_id"),
             py::arg("symbol"),
             py::arg("side"),
             py::arg("price"),
             py::arg("quantity"),
             py::arg("type"))
        .def_readwrite("order_id", &Order::order_id)
        .def_readwrite("client_id", &Order::client_id)
        .def_readwrite("symbol", &Order::symbol)
        .def_readwrite("side", &Order::side)
        .def_readwrite("type", &Order::type)
        .def_readwrite("price", &Order::price)
        .def_readwrite("quantity", &Order::quantity)
        .def_readwrite("timestamp", &Order::timestamp)  //timestamp is public but not in ctor
        ;

    //Trade... (unchanged)
    py::class_<Trade>(m, "Trade")
        .def_readonly("trade_id", &Trade::trade_id)
        .def_readonly("symbol", &Trade::symbol)
        .def_readonly("price", &Trade::price)
        .def_readonly("quantity", &Trade::quantity)
        .def_readonly("buy_order_id", &Trade::buy_order_id)
        .def_readonly("sell_order_id", &Trade::sell_order_id)
        .def_readonly("timestamp", &Trade::timestamp)
        ;

    //Manager... (unchanged)
    py::class_<OrderBookManager>(m, "OrderBookManager")
        .def(py::init<>())
        .def("add_order_book",    &OrderBookManager::addOrderBook,    py::arg("symbol"))
        .def("remove_order_book", &OrderBookManager::removeOrderBook, py::arg("symbol"))
        .def("has_order_book",    &OrderBookManager::hasOrderBook,    py::arg("symbol"))
        .def("place_order",       &OrderBookManager::placeOrder,       py::arg("order"))
        .def("cancel_order",      &OrderBookManager::cancelOrder,      py::arg("symbol"), py::arg("order_id"))
        .def("process_orders",    &OrderBookManager::processOrders)
        .def("get_best_bid",      &OrderBookManager::getBestBid,       py::arg("symbol"))
        .def("get_best_ask",      &OrderBookManager::getBestAsk,       py::arg("symbol"))
        .def("get_bid_size",      &OrderBookManager::getBidSize,       py::arg("symbol"))
        .def("get_ask_size",      &OrderBookManager::getAskSize,       py::arg("symbol"))
        .def("get_bid_depth",     &OrderBookManager::getBidDepth,      py::arg("symbol"), py::arg("levels"))
        .def("get_ask_depth",     &OrderBookManager::getAskDepth,      py::arg("symbol"), py::arg("levels"))
        .def("has_order",         &OrderBookManager::hasOrder,         py::arg("symbol"), py::arg("order_id"))
        .def("get_order",         &OrderBookManager::getOrder,         py::arg("symbol"), py::arg("order_id"),
             py::return_value_policy::reference_internal)
        ;
}
