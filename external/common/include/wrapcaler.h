#include "./caler.h"
#include "./order_side.h"
#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(caler) {
  class_<Fee>("Fee")
    .def_readwrite("open_fee", &Fee::open_fee)
    .def_readwrite("close_fee", &Fee::close_fee);
  class_<FeePoint>("FeePoint")
    .def_readwrite("open_fee_point", &FeePoint::open_fee_point)
    .def_readwrite("close_fee_point", &FeePoint::close_fee_point);
  class_<CALER>(
      "CALER", init<std::string>())
    .def(init<const libconfig::Setting&>())
    .def("CalFee", &CALER::CalFee)
    .def("CalFeePoint", &CALER::CalFeePoint)
    .def("CalPnl", &CALER::CalPnl)
    .def("CalNetPnl", &CALER::CalNetPnl)
    .def("GetMinPriceMove", &CALER::GetMinPriceMove)
    .def("GetConSize", &CALER::GetConSize);
  enum_<OrderSide::Enum>("OrderSide")
    .value("Buy", OrderSide::Buy)
    .value("Sell", OrderSide::Sell);
}
