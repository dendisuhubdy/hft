#ifndef SENDER_H_
#define SENDER_H_

#include <zmq.hpp>
#include <unistd.h>
#include <string>
#include <memory>
#include "exchange_info.h"
#include "market_snapshot.h"
#include "pricer_data.h"
#include "define.h"
#include "order.h"
#include "command.h"

using namespace std;

class Sender {
 public:
  explicit Sender(const std::string& name, const std::string & bs_mode = "bind", const std::string & mode = "ipc", std::ofstream* shot_recorder = nullptr, std::ofstream* order_recorder = nullptr, std::ofstream* info_recorder = nullptr);

  ~Sender();
  void Bind(const std::string & address);

  template <typename T>
    void Send(const T & t) {
      char buffer[BUFFER_SIZE];
        if (BUFFER_SIZE < sizeof(t)) {
          printf("buffer size is not enough, 28\n");
          exit(1);
        }   
        memcpy(buffer, &t, sizeof(t));
        pthread_mutex_lock(&mutex);
        sock.get()->send(buffer, sizeof(buffer));
        pthread_mutex_unlock(&mutex);
    }

  /*
  void Send(const MarketSnapshot& shot);
  void Send(const Order& order);
  void Send(const ExchangeInfo& info);
  void Send(const PricerData& p);
  void Send(const Command& p);
  void Send(const char* s);
*/

 private:
  unique_ptr<zmq::context_t> con;
  unique_ptr<zmq::socket_t> sock;
  pthread_mutex_t mutex;
  unique_ptr<std::ofstream> shoter;
  unique_ptr<std::ofstream> orderer;
  unique_ptr<std::ofstream> infoer;
};

#endif // SENDER_H_
