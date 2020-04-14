#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <util/recver.hpp>
#include <util/sender.hpp>
#include <struct/market_snapshot.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>

int main() {
  Recver<MarketSnapshot> recver("data_source");
  Sender<MarketSnapshot> sender("data_sub", "connect");
  while (true) {
    MarketSnapshot shot;
    recver.Recv(shot);
    sender.Send(shot);
    // shot.Show(stdout);
  }
}
