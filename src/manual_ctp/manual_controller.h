#ifndef SRC_MANUAL_CTP_MANUAL_CONTROLLER_H_
#define SRC_MANUAL_CTP_MANUAL_CONTROLLER_H_

#include <stdio.h>

#include <struct/order.h>
#include <struct/exchange_info.h>
#include <struct/order_side.h>

#include <util/zmq_sender.hpp>
#include <util/zmq_recver.hpp>

#include <thread>
#include <iostream>
#include <string>

class ManualController {
 public:
  ManualController();
  ~ManualController();

  void Start();

 private:
  static void RunSend(Sender<Order> * sender);
  static void RunRecv(Recver<ExchangeInfo> * recver);
  ZmqSender<Order> * order_sender;
  ZmqRecver<ExchangeInfo>* exchange_recver;
};

#endif  // SRC_MANUAL_CTP_MANUAL_CONTROLLER_H_
