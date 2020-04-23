#ifndef SRC_MANUAL_CTP_MANUAL_CONTROLLER_H_
#define SRC_MANUAL_CTP_MANUAL_CONTROLLER_H_

#include <stdio.h>

#include <struct/order.h>
#include <struct/exchange_info.h>
#include <struct/order_side.h>

#include <util/sender.hpp>
#include <util/recver.hpp>

#include <thread>
#include <iostream>
#include <string>

class ManualController {
 public:
  ManualController();
  ~ManualController();

  void Start();

 private:
  static void RunSend(Sender<Order> * s);
  static void RunRecv(Recver<ExchangeInfo> * e);
  std::unique_ptr<Sender<Order> > order_sender;
  std::unique_ptr<Recver<ExchangeInfo> > exchange_recver;
};

#endif  // SRC_MANUAL_CTP_MANUAL_CONTROLLER_H_
