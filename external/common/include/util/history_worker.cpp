#include "./history_worker.h"
typedef std::pair<std::string, int> Pair;

bool cmp(const Pair& a, const Pair& b) {
  return a.second > b.second;
}

HistoryWorker::HistoryWorker() {
}

HistoryWorker::HistoryWorker(std::string file_path) {
  LoadFile(file_path);
}

HistoryWorker::~HistoryWorker() {
}

void HistoryWorker::Clear() {
  volume_map.clear();
  tick_map.clear();
  tick_pair.clear();
  tick_set.clear();
  ticker_set.clear();
  is_ready = false;
}

void HistoryWorker::EnReady() {
  is_ready = true;
}

void HistoryWorker::UpdateByShot(MarketSnapshot shot) {
    std::string tick = GetCon(shot.ticker);
    volume_map[shot.ticker] = shot.volume;
    tick_set.insert(tick);
    ticker_set.insert(shot.ticker);
    tick_map[tick][shot.ticker] = shot.volume;
}

void HistoryWorker::LoadFile(std::string file_path) {
  tc.StartTimer();
  std::string file_mode = Split(file_path, ".").back();
  if (file_mode == "gz") {
    gzFile gzfp = gzopen(file_path.c_str(), "rb");
    if (!gzfp) {
      printf("HistoryWorker %s not found!\n", file_path.c_str());
      exit(1);
    }
    unsigned char buf[SIZE_OF_SNAPSHOT];
    MarketSnapshot* shot;
    while (gzread(gzfp, buf, sizeof(*shot)) > 0) {
      shot = reinterpret_cast<MarketSnapshot*>(buf);
      if (!shot->IsGood()) {
        continue;
      }
      UpdateByShot(*shot);
    }
    gzclose(gzfp);
  } else if (file_mode == "dat") {
    std::ifstream raw_file;
    raw_file.open(file_path.c_str(), ios::in|ios::binary);
    if (!raw_file) {
      printf("%s is not existed!\n", file_path.c_str());
      exit(1);
    }
    MarketSnapshot shot;
    while (raw_file.read(reinterpret_cast<char *>(&shot), sizeof(shot))) {
      if (!shot.IsGood()) {
        continue;
      }
      UpdateByShot(shot);
    }
    raw_file.close();
  } else {
    printf("HistoryWorker unknown mode %s\n", file_path.c_str());
    exit(1);
  }
  tc.EndTimer("loadfile");
  is_ready = true;
}

void HistoryWorker::CheckReady() {
  if (!is_ready) {
    printf("HistoryWorker is not ready!\n");
    exit(1);
  }
}

std::vector<std::string> HistoryWorker::GetAllTick() {
  CheckReady();
  return std::vector<std::string>(tick_set.begin(), tick_set.end());
}

std::vector<std::pair<std::string, int> > HistoryWorker::GetAllTicker(std::string tick) {
  CheckReady();
  // tc.StartTimer();
  std::vector<std::string> v;
  std::map<std::string, int> map = tick_map[tick];
  std::vector<std::pair<std::string, int> > p;
  for (auto m : map) {
    p.emplace_back(std::make_pair(m.first, m.second));
  }
  std::sort(p.begin(), p.end(), cmp);
  // tc.EndTimer("GetAllTicker");
  return p;
}

std::vector<std::string> HistoryWorker::GetTicker(std::string tick, int num) {
  CheckReady();
  std::vector<std::pair<std::string, int> > v = GetAllTicker(tick);
  if (num == -1) {
    num = v.size();
  }
  v.resize(num);
  std::vector<std::string> rv;
  for (auto s : v) {
    rv.emplace_back(s.first);
  }
  return rv;
}

void HistoryWorker::create_file(std::string file_name) {
  /*
  std::ofstream raw_file;
  raw_file.open(file_name.c_str(), ios::out);
  if (!raw_file) {
    printf("%s cannot be created", file_name.c_str());
    exit(1);
  }
  std::pair<std::string, std::ofstream> p(file_name, raw_file);
  file_map.insert(p);
  */
}

void HistoryWorker::GenConCSV() {
  /*
  for_each(ticker_set.begin(), ticker_set.end(), [](std::string file_name) {std::ofstream raw_file(file_name.c_str(), ios::out);file_map[file_name] = raw_file});
  */
}
