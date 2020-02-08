#ifndef HISTORYWORKER_H_
#define HISTORYWORKER_H_

#include <zlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "./struct/market_snapshot.h"
#include "./util/common_tools.h"
#include "./util/timecontroller.h"

class HistoryWorker {
 public:
  HistoryWorker(std::string file_path);
  HistoryWorker();
  ~HistoryWorker();
  std::vector<std::string> GetAllTick();
  std::vector<std::pair<std::string, int> > GetAllTicker(std::string tick);
  std::vector<std::string> GetTicker(std::string tick, int num = -1);

  void Clear();
  void UpdateByShot(MarketSnapshot shot);
  void LoadFile(std::string file_path);

  void GenConCSV();
  void EnReady();
 private:
  void CheckReady();
  void create_file(std::string file_name);
  std::map<std::string, int> volume_map;
  std::unordered_map<std::string, std::map<std::string, int> > tick_map;
  std::unordered_map<std::string, std::pair<std::string, int> > tick_pair;
  std::unordered_set<std::string> tick_set;
  std::unordered_set<std::string> ticker_set;
  TimeController tc;
  bool is_ready;
};

#endif //  HISTORYWORKER_H_
