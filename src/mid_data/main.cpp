#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <util/zmq_recver.hpp>
#include <util/zmq_sender.hpp>
#include <struct/market_snapshot.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>

int main() {
  ZmqRecver<MarketSnapshot> recver("data_source");
  ZmqSender<MarketSnapshot> sender("data_sender", "connect");
  while (true) {
    MarketSnapshot shot;
    recver.Recv(shot);
    sender.Send(shot);
    // shot.Show(stdout);
  }
}
