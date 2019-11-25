#ifndef SENDER_H_
#define SENDER_H_

#include <zmq.hpp>
#include <unistd.h>
#include <string>
#include <memory>
#include "exchange_info.h"
#include "market_snapshot.h"
#include "pricer_data.h"
#include "common_tools.h"
#include "define.h"
#include "order.h"
#include "command.h"

using namespace std;

class Sender {
 public:
  explicit Sender(const std::string& name, const std::string & bs_mode = "bind", const std::string & zmq_mode = "ipc", std::string file_name = "");

  ~Sender();
  inline void Bind(const std::string & address);

  template <typename T>
    inline void Send(const T & t) {
      sock.get()->send(&t, sizeof(T));
      SaveBin(*f.get(), t);
    }

 private:
  unique_ptr<zmq::context_t> con;
  unique_ptr<zmq::socket_t> sock;
  pthread_mutex_t mutex;
  unique_ptr<std::ofstream> f;
};

#endif // SENDER_H_
