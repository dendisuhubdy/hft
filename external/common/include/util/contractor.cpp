#include "./contractor.h"

Contractor::Contractor(const std::string& config_path) :setting((cfg.readFile(config_path.c_str()), cfg.lookup("map"))) {
  for (int i = 0; i < setting.getLength(); i++) {
    const libconfig::Setting & s = setting[i];
    std::string ticker = s["ticker"];
    ticker_index_map[ticker] = i;
  }
}

Contractor::~Contractor() {

}

std::vector<std::string> Contractor::GetTicker() const {
  std::vector<std::string> v;
  for (auto m : ticker_index_map) {
    v.push_back(m.first);
  }
  return v;
}

const libconfig::Setting& Contractor::Lookup(const std::string & ticker) {
  if (ticker_index_map.find(ticker) == ticker_index_map.end()) {
    printf("ticker %s not found in contractor\n", ticker.c_str());
    exit(1);
  }
  return setting[ticker_index_map[ticker]];
}
