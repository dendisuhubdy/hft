#ifndef EXCHANGE_INFO_H_
#define EXCHANGE_INFO_H_

#include <sys/time.h>
#include <fstream>
#include <stdio.h>
#include "define.h"
#include "info_type.h"
#include "order_side.h"
// #include "wrapstruct.h"

struct ExchangeInfo {
  InfoType::Enum type;
  char ticker[MAX_CONTRACT_LENGTH];
  char order_ref[MAX_ORDERREF_SIZE];
  int trade_size;
  double trade_price;
  char reason[EXCHANGE_INFO_SIZE];
  OrderSide::Enum side;

  ExchangeInfo()
    : trade_size(0),
      trade_price(-1) {
  }

  void ShowCsv(FILE* stream) const {
    /*
    char time_s[32];
    snprintf(time_s, sizeof(time_s), "%ld.%ld", time.tv_sec, time.tv_usec);
    double time_sec = atof(time_s);
    */
    fprintf(stream, "%s,%s,%s,%d,%lf,%s,%s\n", InfoType::ToString(type),ticker,order_ref,trade_size,trade_price,reason,OrderSide::ToString(side));
  }

  void Show(FILE* stream) const {
    timeval time;
    gettimeofday(&time, NULL);
    fprintf(stream, "%ld %06ld exchangeinfo %s |",
            time.tv_sec, time.tv_usec, order_ref);

    fprintf(stream, " %lf@%d %s %s %s\n", trade_price, trade_size, InfoType::ToString(type), ticker, OrderSide::ToString(side));
  }
};

#endif  //  EXCHANGE_INFO_H_
