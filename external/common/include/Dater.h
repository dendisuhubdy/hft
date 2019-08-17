#ifndef DATER_H_
#define DATER_H_

#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "./common_tools.h"

using namespace boost::gregorian;

class Dater {
 public:
  Dater();
  ~Dater();
  std::string GetOneZipDataFileNameByDate(const std::string & date) const;
  std::string GetOneDataFileNameByDate(const std::string & date) const;
  std::vector<std::string> GetDataFilesNameByDate(const std::string & start_date, const std::string & end_date, bool reverse = false) const;
  std::vector<std::string> GetDataFilesNameByDate(const std::string & start_date, int num_days, bool reverse = false) const;
  std::string GetCurrentDate() const;
  std::string GetValidFile(const std::string & start_date, int num_days = 20) const;
 private:
  std::string TransDateFormat(const std::string & date, char split_c = '-') const;
  bool CheckDateLegal(const std::string & year, const std::string & month, const std::string & day) const;
};

#endif  // DATER_H_
