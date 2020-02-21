#include "./dater.h"

Dater::Dater() {

}

Dater::~Dater() {

}

std::string Dater::GetOneZipDataFileNameByDate(const std::string & date) {
  std::string date_str = TransDateFormat(date);
  if (date_str.size() < 5) {
    printf("bad date %s\n", date.c_str());
    return "";
  }
  return "/running/"+date_str+"/future" + date_str + ".dat.gz";
}

std::string Dater::GetOneDataFileNameByDate(const std::string & date) {
  std::string date_str = TransDateFormat(date);
  if (date_str.size() < 5) {
    printf("bad date %s\n", date.c_str());
    return "";
  }
  return "/running/"+date_str+"/future" + date_str + ".dat";
}

std::string Dater::GetValidFile(const std::string & start_date, int num_days) {
  std::vector<std::string> v = GetDataFilesNameByDate(start_date, num_days);
  for (auto i : v) {
    struct stat info;  
    stat(i.c_str(), &info);  
    if (access(i.c_str(), F_OK) != -1) {
      if (i.back() == 'z' && info.st_size > 10000000) {
        printf("%s exsited! return info size is %ld\n", i.c_str(), info.st_size);
        return i;
      } else if (i.back() == 't' && info.st_size > 100000000) {
        printf("%s exsited! return info size is %ld\n", i.c_str(), info.st_size);
        return i;
      }
    }
    printf("%s not exsited!\n", i.c_str());
  }
  printf("for %s, in %d days, no valid data!\n", start_date.c_str(), num_days);
  return "";
}

std::string Dater::GetCurrentDate() {
  boost::gregorian::date d = boost::gregorian::day_clock::local_day();
  return boost::gregorian::to_iso_extended_string(d);
}

std::string Dater::GetDate(std::string start_date, int diff) {
  boost::gregorian::date d;
  if (start_date == "") {
    d = boost::gregorian::day_clock::local_day();
  } else {
    d = from_simple_string(start_date.c_str());
  }
  day_iterator it(d);
  std::string date = to_iso_extended_string(*it);
  if (diff > 0) {
    while (diff--) {
      date = to_iso_extended_string(*++it);
    }
  } else {
    while (diff++) {
      date = to_iso_extended_string(*--it);
    }
  }
  return date;
}

std::vector<std::string> Dater::GetDataFilesNameByDate(const std::string & start_date, const std::string & end_date, bool reverse) {
  std::vector<std::string> v;
  return v;
}

std::vector<std::string> Dater::GetDataFilesNameByDate(const std::string & start_date, int num_days, bool reverse) {
  std::vector<std::string> v;
  boost::gregorian::date d = from_simple_string(start_date.c_str());
  day_iterator it(d);
  boost::gregorian::date_facet *df = new boost::gregorian::date_facet("%Y-%m-%d"); 
  std::cout.imbue(std::locale(std::cout.getloc(), df));
  v.emplace_back(GetOneDataFileNameByDate(to_iso_extended_string(*it)));
  v.emplace_back(GetOneZipDataFileNameByDate(to_iso_extended_string(*it)));
  if (num_days > 0) {
    for (int i = 0; i < num_days-1; i++) {
      std::string date = to_iso_extended_string(*++it);
      if (reverse) {
        v.emplace_back(GetOneDataFileNameByDate(date));
        v.emplace_back(GetOneZipDataFileNameByDate(date));
      } else {
        v.insert(v.begin(), GetOneDataFileNameByDate(date));
        v.insert(v.begin(), GetOneZipDataFileNameByDate(date));
      }
    }
  } else if (num_days < 0) {
    for (int i = 0; i < -num_days-1; i++) {
      std::string date = to_iso_extended_string(*--it);
      if (reverse) {
        v.insert(v.begin(), GetOneDataFileNameByDate(date));
        v.insert(v.begin(), GetOneZipDataFileNameByDate(date));
      } else {
        v.emplace_back(GetOneDataFileNameByDate(date));
        v.emplace_back(GetOneZipDataFileNameByDate(date));
      }
    }
  }
  return v;
}

std::string Dater::TransDateFormat(const std::string & date, char split_c) {
  std::vector<std::string> v = Split(date, '-');
  std::string re_str = "";
  if (v.size() == 3) {
    std::string year = v[0];
    std::string month = v[1];
    std::string day = v[2];
    return CheckDateLegal(year, month, day) ? year + split_c + month + split_c + day : "";
  } else if (v.size() == 1 && v.front().size() == 8) {
    std::string year = date.substr(0, 4);
    std::string month = date.substr(4, 2);
    std::string day = date.substr(6, 2);
    return CheckDateLegal(year, month, day) ? year + split_c + month + split_c + day : "";
  } else {
    PrintVector(v);
    printf("wrong date %s\n", date.c_str());
    return "";
  }
}

bool Dater::CheckDateLegal(const std::string & year, const std::string & month, const std::string & day) {
  int y = atoi(year.c_str());
  int m = atoi(month.c_str());
  int d = atoi(day.c_str());
  if (y > 2050 || y < 1970 || m > 12 || m <=0 || d <= 0 || d > 31) {
    printf("get wrong date for %s %s %s\n", year.c_str(), month.c_str(), day.c_str());
    return false;
  }
  return true;
}
