#include "sender.h"

Sender::Sender(const std::string& name, const std::string & bs_mode, const std::string & zmq_mode, std::string file_name) :
    con(new zmq::context_t(1)),
    sock(new zmq::socket_t(*(con), ZMQ_PUB)),
    f(file_name.empty() ? nullptr : new std::ofstream(file_name.c_str(), ios::out | ios::app | ios::binary)) {
  string address = zmq_mode + "://" + name;
  printf("sender address is %s\n", address.c_str());
  if (bs_mode == "connect") {
    sock.get()->connect(address.c_str());
  } else if (bs_mode == "bind") {
    printf("bind address is %s\n", address.c_str());
    sock.get()->bind(address.c_str());
  } else {
    printf("Sender wrong zmq_mode %s\n", bs_mode.c_str());
    exit(1);
  }
  sleep(1);
}

Sender::~Sender() {
  sock.get()->close();
  con.get()->close();
}

void Sender::Bind(const std::string & address) {
  sock.get()->bind(address);
}
