#include "data_handler.h"

DataHandler::DataHandler() {

}

DataHandler::~DataHandler() {

}

void DataHandler::LoadData(const std::string& file_path) {
    std::string file_mode = Split(file_path, ".").back();
    if (file_mode == "gz") {
      gzFile gzfp = gzopen(file_path.c_str(), "rb");
      if (!gzfp) {
        printf("gzfile open failed!%s\n", file_path.c_str());
        return;
      }
      printf("handling %s\n", file_path.c_str());
      unsigned char buf[SIZE_OF_SNAPSHOT];
      MarketSnapshot* shot;
      bool is_cut = false;
      while (gzread(gzfp, buf, sizeof(*shot)) > 0) {
        shot = reinterpret_cast<MarketSnapshot*>(buf);
        HandleShot(shot);
      }
    }
  }

