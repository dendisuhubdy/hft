#ifndef ZMQ_SENDER_HPP_
#define ZMQ_SENDER_HPP_

#include <zmq.hpp>
#include <unistd.h>
#include <string>
#include <memory>
#include <mutex>
#include "struct/exchange_info.h"
#include "struct/market_snapshot.h"
#include "util/common_tools.h"
#include "define.h"
#include "struct/order.h"
#include "struct/command.h"
#include "base_sender.hpp"

using namespace std;

template <typename T>
class ZmqSender : public BaseSender <T> {
 public:
  explicit ZmqSender(const std::string& name, const std::string & bs_mode = "bind", const std::string & zmq_mode = "ipc", const std::string& file_name = "")
    : con(new zmq::context_t(1)),
      sock(new zmq::socket_t(*(con), ZMQ_PUB)),
      f(file_name.empty() ? nullptr : new std::ofstream(file_name.c_str(), ios::out | ios::binary)) {
    sock->setsockopt(ZMQ_SNDHWM, 0);
    sock->setsockopt(ZMQ_SNDBUF, 2000000000);
    string address = zmq_mode + "://" + name;
    printf("sender address is %s\n", address.c_str());
    if (bs_mode == "connect") {
      sock.get()->connect(address.c_str());
    } else if (bs_mode == "bind") {
      printf("bind address is %s\n", address.c_str());
      sock.get()->bind(address.c_str());
    } else {
      printf("ZmqSender wrong zmq_mode %s\n", bs_mode.c_str());
      exit(1);
    }
    sleep(1);
  }


  ~ZmqSender() {
    sock.get()->close();
    con.get()->close();
  }

  inline void Send(const T & t) override {
    sock.get()->send(&t, sizeof(T));
    if (f.get()) {
      std::lock_guard<std::mutex> lck(mtx);  // for mutli-thread backtest file writting
      f.get()->write((char*)&t, sizeof(T));
      f.get()->flush();
    }
  }

 private:
  unique_ptr<zmq::context_t> con;
  unique_ptr<zmq::socket_t> sock;
  std::mutex mtx;
  unique_ptr<std::ofstream> f;
};

#endif // ZMQ_SENDER_HPP_
