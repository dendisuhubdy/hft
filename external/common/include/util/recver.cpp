#include "recver.h"

Recver::Recver(const std::string & name, const std::string & mode, const std::string & bc) :
    con(new zmq::context_t(1)),
    sock(new zmq::socket_t(*con, ZMQ_SUB)) {
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

Recver::~Recver() {
  sock.get()->close();
  con.get()->close();
}
