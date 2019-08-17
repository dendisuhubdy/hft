#ifndef SENDER_H_
#define SENDER_H_

#include <zmq.hpp>
#include <unistd.h>
#include <string>
#include "exchange_info.h"
#include "market_snapshot.h"
#include "pricer_data.h"
#include "define.h"
#include "order.h"

using namespace std;

class Sender {
 public:
  Sender(const std::string& name, const std::string & bs_mode = "bind", const std::string & mode = "ipc", std::ofstream* shot_recorder = nullptr, std::ofstream* order_recorder = nullptr, std::ofstream* info_recorder = nullptr);

  ~Sender();
  void Bind(const std::string & address);

  void Send(const MarketSnapshot& shot);
  void Send(const Order& order);
  void Send(const ExchangeInfo& info);
  void Send(const PricerData& p);
  void Send(const char* s);

 private:
  zmq::context_t* con;
  zmq::socket_t* sock;
  pthread_mutex_t mutex;
  std::ofstream* shoter;
  std::ofstream* orderer;
  std::ofstream* infoer;
};

#endif // SENDER_H_
