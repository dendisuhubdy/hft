#ifndef ZMQRECVER_HPP_
#define ZMQRECVER_HPP_

#include <zmq.hpp>
#include <unistd.h>
#include <string>
#include <memory>
#include "struct/exchange_info.h"
#include "struct/market_snapshot.h"
#include "define.h"
#include "struct/order.h"
#include "base_recver.hpp"

using namespace std;

template <typename T>
class ZmqRecver : public BaseRecver <T> {
 public:
  ZmqRecver(const std::string& name, const std::string& mode = "ipc", const std::string& bc = "connect")
    : con(new zmq::context_t(1)),
      sock(new zmq::socket_t(*con, ZMQ_SUB)) {
    sock->setsockopt(ZMQ_RCVHWM, 0);
    sock->setsockopt(ZMQ_RCVBUF, 2000000000);
    string address = mode + "://" + name;
    sock.get()->setsockopt(ZMQ_SUBSCRIBE, 0, 0);
    if (bc == "connect") {
      printf("recver connect address %s\n", address.c_str());
      sock.get()->connect(address.c_str());
    } else if (bc == "bind") {
      printf("recver bind address %s\n", address.c_str());
      sock.get()->bind(address.c_str());
    } else {
      printf("unknown bc mode for recver %s\n", bc.c_str());
      exit(1);
    }
    sleep(1);
  }


  ~ZmqRecver() {
    sock.get()->close();
    con.get()->close();
  }

  inline void Recv(T& t) override {
    sock.get()->recv(&t, sizeof(T));
  }

 private:
  unique_ptr<zmq::context_t> con;
  unique_ptr<zmq::socket_t> sock;
};

#endif  //  ZMQRECVER_HPP_
