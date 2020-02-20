#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <util/recver.h>
#include <util/sender.h>
#include <struct/market_snapshot.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>

int main() {
  Recver recver("data_source");
  Sender sender("data_sub", "connect");
  while (true) {
    MarketSnapshot shot;
    recver.Recv(shot);
    sender.Send(shot);
    // shot.Show(stdout);
  }
}
