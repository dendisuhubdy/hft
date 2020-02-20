#include "backtester.h"

Backtester::Backtester(const std::unordered_map<std::string, std::vector<BaseStrategy*> > & m) : tsm(m) {

}
Backtester::~Backtester() {}

void Backtester::HandleShot(MarketSnapshot* shot) {
  if (!shot->IsGood()) {
    return;
  }
  if (tsm.find(shot->ticker) == tsm.end()) {
    return;
  }
  /*
  bool is_cut = false;
  if ((shot->time.tv_sec+8*3600) % (24*3600) >= 15*3600-10 && !is_cut) {
    for (auto v : sv) {
      v->Clear();
    }
    is_cut = true;
  }*/
  shot->is_initialized = true;
  std::vector<BaseStrategy*> ticker_sv = tsm.find(shot->ticker)->second;
  for (auto v : ticker_sv) {
    v->UpdateData(*shot);
  }
}
