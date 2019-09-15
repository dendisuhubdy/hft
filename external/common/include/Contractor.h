#ifndef CONTRACTOR_H_
#define CONTRACTOR_H_

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

#include "./market_snapshot.h"
#include "./common_tools.h"
#include "./timecontroller.h"

/*
struct ticker_volume {
  std::string ticker;
  int volume;
  bool operator<(const struct ticker_volume & tv) const {
    if (this->volume < tv.volume) {
      return false;
    }
    return true;
  }
};
*/

class Contractor {
 public:
  Contractor(std::string file_path);
  Contractor();
  ~Contractor();
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
  // std::unordered_map<std::string, std::ofstream> file_map;
  TimeController tc;
  bool is_ready;
};

#endif //  CONTRACTOR_H_
