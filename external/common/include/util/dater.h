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
  static std::string GetOneZipDataFileNameByDate(const std::string & date, std::string fixed_path = "");
  static std::string GetOneDataFileNameByDate(const std::string & date, std::string fixed_path = "");
  static std::vector<std::string> GetDataFilesNameByDate(const std::string & start_date, const std::string & end_date);
  static std::map<std::string, std::vector<std::string> > GetDataFilesNameMapByDate(const std::string & start_date, int num_days);
  static std::vector<std::string> GetDataFilesNameByDate(const std::string & start_date, int num_days, bool reverse = false);
  static std::vector<std::string> GetValidList(const std::string & start_date, int num_days, bool reverse = false);
  static std::map<std::string, std::string> GetValidMap(const std::string & start_date, int num_days);
  static std::string GetCurrentDate();
  static std::string FindOneValid(const std::string & start_date, int num_days = 20);
  static std::string GetDate(std::string start_date = "", int diff = 0);
  static std::vector<std::string> GetDateList(const std::string& start_date, int num_days, bool reverse = false);
 private:
  static std::string TransDateFormat(const std::string & date, char split_c = '-');
  static bool CheckDateLegal(const std::string & year, const std::string & month, const std::string & day);
};

#endif  // DATER_H_
