#include <string.h>
#include <stdio.h>
#include <libconfig.h++>
#include <order.h>
#include <define.h>
#include <market_snapshot.h>
#include <common_tools.h>
#include <sys/time.h>
#include <dirent.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <unordered_map>

#include <iostream>
#include <cctype>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <map>
#include <queue>
#include <string>

#define SEARCH_PATH_LENGTH 1024

class convert_pair {
 public:
    convert_pair(std::string be_converted_file, std::string converted_to_file) {
        this->be_converted_file = be_converted_file;
        this->converted_to_file = converted_to_file;
    }
    convert_pair(string path, convert_pair cp) {
        this->path = path;
        this->be_converted_file = cp.be_converted_file;
        this->converted_to_file = cp.converted_to_file;
    }
    std::string path;
    std::string be_converted_file;
    std::string converted_to_file;
};

/*
*	search the file under the dir  return pairs(path,filename);
*/
vector<convert_pair> get_all_files(std::string path, vector<convert_pair> file_pairs) {
  struct dirent *entry = (struct dirent *)malloc(sizeof(struct dirent));
  vector<convert_pair> files;
  std::queue<std::string> paths;
  paths.push(path);
  while (!paths.empty()) {
    string curr_path = paths.front();
    paths.pop();
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(curr_path.c_str())) == NULL) {
      cerr << "can not open this dir." << endl;
      continue;
    }
    while (!readdir_r(dp, entry, &dirp) && dirp != NULL) {
      if (dirp->d_type == 4) {
        if ((dirp->d_name)[0] == '.') {
          continue;
        }
         string tmp_path = curr_path + "/" + dirp->d_name;
         paths.push(tmp_path);
      } else if (dirp->d_type == 8) {
        for (auto pair : file_pairs) {
          if (pair.be_converted_file.compare(dirp->d_name) == 0) {
            files.emplace_back(convert_pair(curr_path, pair));
          }
        }
      } else {
      }
    }
    closedir(dp);
  }
  return files;
}

void convert_data(convert_pair pair) {
  string be_converted_file = pair.path + "/" + pair.be_converted_file;
  string converted_to_file = pair.path + "/" + pair.converted_to_file;
  printf("handling %s\n", be_converted_file.c_str());
  char buffer[SIZE_OF_SNAPSHOT];
  std::ifstream raw_file;
  std::ofstream binary_file(converted_to_file, ios::out | ios::binary);
  raw_file.open(be_converted_file.c_str(), ios::in);
  if (!raw_file) {
       printf("%s is not existed!", be_converted_file.c_str());
       return;
  }
  while (!raw_file.eof()) {
    raw_file.getline(buffer, SIZE_OF_SNAPSHOT);
    MarketSnapshot shot;
    shot = HandleSnapshot(buffer);
    SaveBin(binary_file, shot);
  }
  printf("  convert finished %s\n", be_converted_file.c_str());
}

int main() {
  std::string path = "/running";
  vector<convert_pair> file_pairs;
  file_pairs.emplace_back(convert_pair("data.log", "data_binary.dat"));
  file_pairs.emplace_back(convert_pair("data_night.log", "data_night_binary.dat"));
  vector<convert_pair> files = get_all_files(path, file_pairs);
  std::cout << files.size() << " files need to be converted"  << endl;
  for (auto cp : files) {
    convert_data(cp);
  }
  return 0;
}
