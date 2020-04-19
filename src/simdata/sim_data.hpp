#ifndef SIM_DATA_H_
#define SIM_DATA_H_

#include <memory>

#include "util/data_handler.hpp"
#include "util/sender.hpp"

class SimData : public DataHandler<MarketSnapshot> {
 public:
  SimData()
   : up(new Sender<MarketSnapshot> ("data_sender", "connect")),
     count(0) {
  }

  ~SimData() {
  }

  void HandleShot(MarketSnapshot* this_shot, MarketSnapshot* next_shot) override {
    up.get()->Send(*this_shot);
    if (count++ % 100000 == 1) {
      this_shot->Show(stdout);
    }
  }
 private:
  std::unique_ptr<Sender<MarketSnapshot> > up;
  int count;
};

#endif // SIM_DATA_H_
