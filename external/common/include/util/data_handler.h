#ifndef DATA_HANDLER_H_
#define DATA_HANDLER_H_
#include <iostream>
#include <unordered_map>
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
  virtual void HandleShot(MarketSnapshot* this_shot, MarketSnapshot* next_shot) = 0;
 private:
  std::unordered_map<std::string, MarketSnapshot> lastshot_map;
};

#endif // DATA_HANDLER_H_
