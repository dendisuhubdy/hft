#ifndef TRANSACTION_H_
#define TRANSACTION_H_

#include "define.h"
#include "order_side.h"

#include <stdio.h>
#include <sys/time.h>
#include <fstream>

struct Transaction {
  timeval time;
  char ticker[MAX_CONTRACT_LENGTH];
  double trade_price;
  int trade_size;
  OrderSide::Enum side;
  char tbd[24];

  std::string ToString() {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%ld %04ld Trade %s %d@%lf %s", time.tv_sec, time.tv_usec, ticker, trade_size, trade_price, OrderSide::ToString(side));
    return buffer;
  }
};
#endif  //  TRANSACTION_H_
