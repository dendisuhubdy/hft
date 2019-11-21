#ifndef RECVER_H_
#define RECVER_H_

#include <zmq.hpp>
#include <unistd.h>
#include <string>
#include <memory>
#include "exchange_info.h"
#include "market_snapshot.h"
#include "pricer_data.h"
#include "define.h"
#include "order.h"

using namespace std;

class Recver {
 public:
  Recver(const std::string& name, const std::string& mode = "ipc", const std::string& bc = "connect");

  ~Recver();


  template <typename T>
  void Recv(T& t) {
    /*
    if (BUFFER_SIZE < sizeof(T)) {
      printf("buffer size is not enough!\n");
      exit(1);
    }
    */
    void* buffer = nullptr;
    sock.get()->recv(buffer, sizeof(T));
    t = *reinterpret_cast<T*>(buffer); //alias-strict, use union to slove this
  }

  /*
  MarketSnapshot Recv(const MarketSnapshot& shot);

  Order Recv(const Order& order);

  ExchangeInfo Recv(const ExchangeInfo& i);

  PricerData Recv(const PricerData& p);
  */

 private:
  unique_ptr<zmq::context_t> con;
  unique_ptr<zmq::socket_t> sock;
};

#endif  //  RECVER_H_
