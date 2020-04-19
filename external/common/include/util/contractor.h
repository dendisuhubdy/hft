#ifndef CONTRACTOR_H_
#define CONTRACTOR_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <libconfig.h++>

class Contractor {
 public:
  explicit Contractor(const std::string & config_path, const std::string & key = "map");
  ~Contractor();
  const libconfig::Setting& Lookup(const std::string& ticker);
  std::vector<std::string> GetTicker() const;

 private:
  libconfig::Config cfg;
  const libconfig::Setting& setting;
  std::unordered_map<std::string, int> ticker_index_map;
  std::unordered_map<std::string, libconfig::Setting &> m_ticker_setting_map;
};

#endif  // CONTRACTOR_H_
