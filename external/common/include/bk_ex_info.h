#ifndef EXCHANGE_INFO_H_
#define EXCHANGE_INFO_H_

#include <sys/time.h>
#include <fstream>
#include <stdio.h>
#include "define.h"
#include "info_type.h"
#include "order_side.h"

struct ExchangeInfo {
  InfoType::Enum type;
  char contract[MAX_CONTRACT_LENGTH];
  char order_ref[MAX_ORDERREF_SIZE];
  int trade_size;
  double trade_price;
  char reason[EXCHANGE_INFO_SIZE];
  OrderSide::Enum side;

  ExchangeInfo();

  void Show(std::ofstream &stream) const;

  void ShowCsv(FILE* stream) const;

  void Show(FILE* stream) const;
};

#endif  //  EXCHANGE_INFO_H_
