#include "data_transformer.h"

DataTransformer::DataTransformer(int num)
    : tc(new TimeController()),
      cpu_count(num) {
  printf("%d processor ready for dt\n", cpu_count);
  pool = new ThreadPool(cpu_count);
}

DataTransformer::DataTransformer()
    : tc(new TimeController()),
      cpu_count(get_nprocs()-1) {
  int n_cpu = get_nprocs();
  if (n_cpu == 1) {
    cpu_count = 1;
  } else if (n_cpu < 4) {
    cpu_count = n_cpu - 1;
  } else {
    cpu_count = n_cpu - 2;
  }
  printf("%d processor ready for dt\n", cpu_count);
  pool = new ThreadPool(cpu_count);
}


DataTransformer::~DataTransformer() {
  delete tc;
  delete pool;
}

std::string DataTransformer::GenFileName(const std::string & source_path, const std::string & file_name, const std::string & post_prefix) {
  std::string f = file_name;
  if (f == "") {
    f = Split(Split(source_path, '/').back(), '.').front();
  }
  return f + "." + post_prefix;
}

void DataTransformer::BinToLog(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
  TimeController tc;
  tc.StartTimer();
  std::string dfile = dest_dir == "" ? GenFileName(source_path, file_name, "log") : dest_dir + "/" + GenFileName(source_path, file_name, "log");
  printf("bintolog is running! %s->%s\n", source_path.c_str(), dfile.c_str());
  FILE* log = fopen(dfile.c_str(), "a");
  if (!log) {
    printf("logfile %s open failed!\n", dfile.c_str());
    exit(1);
  }
  ifstream input(source_path.c_str(), ios::in | ios::binary);
  if (!input) {
    printf("%s not exsited!\n", source_path.c_str());
    exit(1);
  }
  MarketSnapshot shot;
  while (!input.eof()) {
    input.read((char*)&shot,sizeof(shot));
    shot.Show(log);
  }
  input.close();
  fclose(log);
  std::string label = "";
  label += source_path + "->" + dfile + "";
  tc.EndTimer(label);
}

void DataTransformer::BinToCsv(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
  TimeController tc;
  tc.StartTimer();
  std::string dfile = dest_dir == "" ? GenFileName(source_path, file_name, "csv") : dest_dir + "/" + GenFileName(source_path, file_name, "csv");
  printf("csv file is %s\n", dfile.c_str());
  FILE* csv = fopen(dfile.c_str(), "w");
  if (!csv) {
    printf("csvfile %s open failed!\n", dfile.c_str());
    exit(1);
  }
  ifstream input(source_path.c_str(), ios::app | ios::in | ios::binary);
  if (!input) {
    printf("%s not exsited!\n", source_path.c_str());
    exit(1);
  }
  MarketSnapshot shot;
  while (!input.eof()) {
    input.read((char*)&shot,sizeof(shot));
    shot.ShowCsv(csv);
  }
  input.close();
  fclose(csv);
  std::string label = "";
  label += source_path + "->" + dfile + "";
  tc.EndTimer(label);
}

void DataTransformer::LogToBin(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
  // printf("logtobin is called! %s->%s\n", source_path.c_str(), dest_dir.c_str());
  TimeController tc;
  tc.StartTimer();
  std::string dfile = dest_dir == "" ? GenFileName(source_path, file_name, "dat") : dest_dir + "/" + GenFileName(source_path, file_name, "dat");
  /*
  if (Split(source_path, '/').back() == "data.log" && access(dest_dir.c_str(), F_OK) != -1) {
    if (remove(dest_dir.c_str()) != 0) {
      printf("remove %s failed\n", dest_dir.c_str());
      exit(1);
    }
    printf("%s exsited, remove it to redo\n", dest_dir.c_str());
  }
  */
  // printf("%s %s %d\n", Split(source_path, '/').back().c_str(), dest_dir.c_str(), access(dest_dir.c_str(), F_OK));
  ifstream raw_file;
  ofstream bfile;
  bfile.open(dest_dir.c_str(), ios::app | ios::out | ios::binary);
  char buffer[SIZE_OF_SNAPSHOT];
  raw_file.open(source_path.c_str(), ios::in);
  while (raw_file.getline(buffer, SIZE_OF_SNAPSHOT)) {
    if (buffer[0] == '\0') {
      break;
    }   
    MarketSnapshot snapshot = HandleSnapshot(buffer);
    SaveBin(bfile, snapshot);
    // snapshot.Show(stdout);
  }
  raw_file.close();
  bfile.close();
  std::string label = "";
  label += source_path + "->" + dest_dir + "";
  tc.EndTimer(label);
}

void DataTransformer::LogToCsv(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
}

void DataTransformer::CsvToBin(const std::string & source_path, const std::string & dest_dir, const std::string & file_name){
  TimeController tc;
  tc.StartTimer();
  std::string label = "";
  label += source_path + "->" + dest_dir + "";
  tc.EndTimer(label);
}

void DataTransformer::CsvToLog(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
}


void DataTransformer::GzToCsv(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
  TimeController tc;
  tc.StartTimer();
  std::string dfile = dest_dir == "" ? GenFileName(source_path, file_name, "csv") : dest_dir + "/" + GenFileName(source_path, file_name, "csv");
  FILE* f = fopen(dfile.c_str(), "w");
  if (!f) {
    printf("open file failed! %s\n", dfile.c_str());
    exit(1);
  }
  gzFile gzfp = gzopen(source_path.c_str(), "rb");
  if (!gzfp) {
    printf("open source_path failed! %s\n", source_path.c_str());
    exit(1);
  }
  unsigned char buf[SIZE_OF_SNAPSHOT];
  MarketSnapshot* shot;
  while (gzread(gzfp, buf, sizeof(*shot)) > 0) {
    shot = reinterpret_cast<MarketSnapshot*>(buf);
    if (!shot->IsGood()) {
      continue;
    }
    shot->ShowCsv(f);
  }
  gzclose(gzfp);
  std::string label = "";
  label += source_path + "->" + dfile;
  tc.EndTimer(label);
}

void DataTransformer::GzToLog(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
}

void DataTransformer::CsvCatContract(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
}

void DataTransformer::BinCatContract(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
  TimeController tc;
  tc.StartTimer();
  printf("handling %s\n", source_path.c_str());
  BinToCsv(source_path, dest_dir);
  HistoryWorker ct(source_path);
  std::string dir = GetDir(source_path);
  std::string f_name = Split(Split(source_path, '/').back(), '.').front();
  std::string csv_file = dest_dir == "" ? f_name : dest_dir + "/" + f_name + ".csv";
  std::vector<std::string> all_tick = ct.GetAllTick();
  for (auto tick : all_tick) {
    std::vector<std::string> all_ticker = ct.GetTicker(tick);
    std::string main_ticker = GetCon(all_ticker.front()) + "8888";
    std::string main_command = "cat " + csv_file + " | grep " + all_ticker.front() + " > " + dest_dir + "/" +main_ticker + ".csv";
    system(main_command.c_str());
    printf("executing command %s\n", main_command.c_str());
    for (auto ticker : all_ticker) {
      std::string command = "cat ";
      command += csv_file;
      command += "| grep ";
      command = command + ticker;
      command += " > " + dest_dir + "/" + ticker + ".csv";
      printf("executing command %s\n", command.c_str());
      system(command.c_str());
    }
  }
  std::string label = "cat file " + source_path + " finished";
  tc.EndTimer(label);
}

void DataTransformer::GzCatContract(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
  TimeController tc;
  tc.StartTimer();
  printf("handling %s\n", source_path.c_str());
  GzToCsv(source_path, dest_dir);
  HistoryWorker ct(source_path);
  std::string dir = GetDir(source_path);
  std::string f_name = Split(Split(source_path, '/').back(), '.').front();
  std::string csv_file = dest_dir == "" ? f_name : dest_dir + "/" + f_name + ".csv";
  std::vector<std::string> all_tick = ct.GetAllTick();
  for (auto tick : all_tick) {
    std::vector<std::string> all_ticker = ct.GetTicker(tick);
    std::string main_ticker = GetCon(all_ticker.front()) + "8888";
    std::string main_command = "cat " + csv_file + " | grep " + all_ticker.front() + " > " + dest_dir + "/" +main_ticker + ".csv";
    system(main_command.c_str());
    printf("executing command %s\n", main_command.c_str());
    for (auto ticker : all_ticker) {
      std::string command = "cat ";
      command += csv_file;
      command += "| grep ";
      command = command + ticker;
      command += " > " + dest_dir + "/" + ticker + ".csv";
      printf("executing command %s\n", command.c_str());
      system(command.c_str());
    }
  }
  std::string label = "cat file " + source_path + " finished";
  tc.EndTimer(label);
}

std::string Con(std::string ticker) {
  int pos = 0;
  for (int i = 0; i < ticker.size(); i ++) {
    if (isdigit(ticker[i])) {
      pos = i;
      break;
    }   
  }
  return ticker.substr(0,pos);
}

void DataTransformer::GzDirectCat(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
  TimeController tc;
  tc.StartTimer();
  gzFile gzfp = gzopen(source_path.c_str(), "rb");
  if (!gzfp) {
    printf("open source_path failed! %s\n", source_path.c_str());
    exit(1);
  }
  std::unordered_map <std::string, std::ofstream*> file_map;
  unsigned char buf[SIZE_OF_SNAPSHOT];
  MarketSnapshot* shot;
  while (gzread(gzfp, buf, sizeof(*shot)) > 0) {
    shot = reinterpret_cast<MarketSnapshot*>(buf);
    if (!shot->IsGood()) {
      continue;
    }
    std::string ticker = shot->ticker;
    if (file_map.find(ticker) == file_map.end()) {
      file_map[ticker] = new std::ofstream;
      std::string file = (dest_dir == "") ? ticker + ".csv" : dest_dir + "/" + ticker + ".csv";
      file_map[ticker]->open(file.c_str(), ios::out);
    }
    (*(file_map[ticker])) << shot->ToCsv();
  }
  gzclose(gzfp);
  for (auto i : file_map) {
    i.second->close();
  }
  HistoryWorker ct(source_path);
  std::vector<std::string> all_tick = ct.GetAllTick();
  for (auto tick : all_tick) {
    std::vector<std::string> all_ticker = ct.GetTicker(tick);
    std::string main_ticker = all_ticker.front() + ".csv";
    std::string out_file = Con(main_ticker) + "8888.csv";
    std::string prefix = (dest_dir == "") ? dest_dir : dest_dir + "/";
    std::string cmd = "cp " + prefix + main_ticker + " " + prefix + out_file;
    system(cmd.c_str());
  }
  std::string label = "finished gzcat for ";
  label += source_path;
  tc.EndTimer(label);
}

void DataTransformer::BinDirectCat(const std::string & source_path, const std::string & dest_dir, const std::string & file_name) {
  TimeController tc;
  tc.StartTimer();
  ifstream input(source_path.c_str(), ios::app | ios::in | ios::binary);
  if (!input) {
    printf("%s not exsited!\n", source_path.c_str());
    exit(1);
  }
  printf("handling %s\n", source_path.c_str());
  std::unordered_map <std::string, std::ofstream*> file_map;
  MarketSnapshot shot;
  while (input.read(reinterpret_cast<char *>(&shot), sizeof(shot))) {
    if (!shot.IsGood()) {
      shot.Show(stdout);
      continue;
    }
    std::string ticker = shot.ticker;
    if (file_map.find(ticker) == file_map.end()) {
      file_map[ticker] = new std::ofstream;
      std::string file = (dest_dir == "") ? ticker + ".csv" : dest_dir + "/" + ticker + ".csv";
      file_map[ticker]->open(file.c_str(), ios::out);
    }
    (*(file_map[ticker])) << shot.ToCsv();
  }
  input.close();
  for (auto i : file_map) {
    i.second->close();
  }
  HistoryWorker ct(source_path);
  std::vector<std::string> all_tick = ct.GetAllTick();
  for (auto tick : all_tick) {
    std::vector<std::string> all_ticker = ct.GetTicker(tick);
    std::string main_ticker = all_ticker.front() + ".csv";
    std::string out_file = Con(main_ticker) + "8888.csv";
    std::string prefix = (dest_dir == "") ? dest_dir : dest_dir + "/";
    std::string cmd = "cp " + prefix + main_ticker + " " + prefix + out_file;
    system(cmd.c_str());
  }
  std::string label = "finished gzcat for ";
  label += source_path;
  tc.EndTimer(label);
}

/*
void DataTransformer::FSYCatContract(const std::string & file_name, const std::string& csv_name, int count, int size) {
  TimeController tc;
  printf("handling %s\n", file_name.c_str());
  tc.StartTimer();
  HistoryWorker ct(file_name);
  std::string dir = GetDir(csv_name);
  // std::string csv_file = dir + "data.csv";
  std::string csv_file = csv_name;
  std::vector<std::string> all_tick = ct.GetAllTick();
  for (auto tick : all_tick) {
    std::vector<std::string> all_ticker = ct.GetTicker(tick);
    std::string main_ticker = GetCon(all_ticker.front()) + "8888";
    std::string main_command = "cat " + csv_file + " | grep " + all_ticker.front() + " > " + dir +main_ticker + ".csv";
    system(main_command.c_str());
    printf("executing command %s\n", main_command.c_str());
    for (auto ticker : all_ticker) {
      std::string command = "cat ";
      command += csv_file;
      command += "| grep ";
      command = command + ticker;
      command += " > " + dir + ticker + ".csv";
      printf("executing command %s\n", command.c_str());
      system(command.c_str());
    }
  }
  std::string label = "cat file " + file_name + " finished";
  tc.EndTimer(label);
}
*/

/*
void DataTransformer::AllBinToLog(const std::string & source_path, const std::string & dest_dir, const std::string & file_name = "") const {
  std::vector<std::string> files = GetFiles(source_path);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(BinToLog, f, GetDir(f) + "data.log", ++count, size);
  }
}

void DataTransformer::AllLogToBin(const std::string & source_path, const std::string & dest_dir, const std::string & data_file_name) const {
  std::vector<std::string> files = GetFiles(source_path, data_file_name);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(LogToBin, f, GetDir(f) + "data_binary.dat", ++count, size);
  }
}

void DataTransformer::AllBinToCsv(const std::string & source_path, const std::string & dest_dir) const {
  std::vector<std::string> files = GetFiles(source_path);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(BinToCsv, f, GetDir(f) + "data.csv", ++count, size);
  }
}

void DataTransformer::AllCsvToBin(const std::string & source_path, const std::string & dest_dir) const {
  std::vector<std::string> files = GetFiles(dest_dir);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(CsvToBin, f, GetDir(f) + "data_binary.dat", ++count, size);
  }
}

void DataTransformer::AllGzToCsv(const std::string & source_path, const std::string & dest_dir) const {
  std::vector<std::string> files = GetFiles(dest_dir);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(GzToCsv, f, GetDir(f) + "data_binary.dat.gz", ++count, size);
  }
}

void DataTransformer::AllCat(const std::string & source_path, const std::string & dest_dir) const {
  std::vector<std::string> files = GetFiles(source_path, "data_binary.dat");
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(CatContract, f, ++count, size);
  }
}
*/
