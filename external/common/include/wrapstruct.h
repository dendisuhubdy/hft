#include "exchange_info.h"
#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(exchangeinfo) {
  class_<ExchangeInfo>("ExchangeInfo", init<>())
    .def_readwrite("type", &ExchangeInfo::type)
    .def_readwrite("contract", &ExchangeInfo::contract)
    .def_readwrite("order_ref", &ExchangeInfo::order_ref)
    .def_readwrite("trade_size", &ExchangeInfo::trade_size)
    .def_readwrite("trade_price", &ExchangeInfo::trade_price)
    .def_readwrite("reason", &ExchangeInfo::reason)
    .def_readwrite("side", &ExchangeInfo::side)
    .def("Show", &ExchangeInfo::ShowCsv);
  enum_<InfoType::Enum>("InfoType")
    .value("Uninited", InfoType::Uninited)
    .value("Acc", InfoType::Acc)
    .value("Rej", InfoType::Rej)
    .value("Cancelled", InfoType::Cancelled)
    .value("CancelRej", InfoType::CancelRej)
    .value("Filled", InfoType::Filled)
    .value("Pfilled", InfoType::Pfilled)
    .value("Position", InfoType::Position)
    .value("Unknown", InfoType::Unknown);
  enum_<OrderSide::Enum>("OrderSide")
    .value("Buy", OrderSide::Buy)
    .value("Sell", OrderSide::Sell);
};
