#ifndef DATA_HANDLER_H_
#define DATA_HANDLER_H_
#include <iostream>
#include <string>
#include <zlib.h>
#include "struct/market_snapshot.h"
#include "util/common_tools.h"
#include "define.h"

class DataHandler {
 public:
  DataHandler();
  ~DataHandler();
  void LoadData(const std::string& file_path);
  virtual void HandleShot(MarketSnapshot* shot) = 0;
};

#endif // DATA_HANDLER_H_
