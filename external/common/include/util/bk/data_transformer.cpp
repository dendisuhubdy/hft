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

void DataTransformer::CsvToBin(const std::string & source_file, const std::string & dest_file, int count, int size){
  TimeController tc;
  tc.StartTimer();
  std::string label = "";
  label += source_file + "->" + dest_file + "";
  tc.EndTimer(label);
  ShowProcess(count, size);
}

void DataTransformer::BinToLog(const std::string & source_file, const std::string & dest_file, int count, int size) {
  printf("bintolog is running! %s->%s\n", source_file.c_str(), dest_file.c_str());
  TimeController tc;
  tc.StartTimer();
  std::string dfile = dest_file;
  if (dest_file == "") {
    dfile = Split(source_file, '.').front() + ".log";
  }
  FILE* log = fopen(dfile.c_str(), "a");
  if (!log) {
    printf("logfile %s open failed!\n", dfile.c_str());
    exit(1);
  }
  ifstream input(source_file.c_str(), ios::in | ios::binary);
  if (!input) {
    printf("%s not exsited!\n", source_file.c_str());
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
  label += source_file + "->" + dfile + "";
  tc.EndTimer(label);
  ShowProcess(count, size);
}

void DataTransformer::LogToBin(const std::string & source_file, const std::string & dest_file, int count, int size) {
  // printf("logtobin is called! %s->%s\n", source_file.c_str(), dest_file.c_str());
  TimeController tc;
  tc.StartTimer();
  if (Split(source_file, '/').back() == "data.log" && access(dest_file.c_str(), F_OK) != -1) {
    if (remove(dest_file.c_str()) != 0) {
      printf("remove %s failed\n", dest_file.c_str());
      exit(1);
    }
    printf("%s exsited, remove it to redo\n", dest_file.c_str());
  }
  // printf("%s %s %d\n", Split(source_file, '/').back().c_str(), dest_file.c_str(), access(dest_file.c_str(), F_OK));
  ifstream raw_file;
  ofstream bfile;
  bfile.open(dest_file.c_str(), ios::app | ios::out | ios::binary);
  char buffer[SIZE_OF_SNAPSHOT];
  raw_file.open(source_file.c_str(), ios::in);
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
  label += source_file + "->" + dest_file + "";
  tc.EndTimer(label);
  ShowProcess(count, size);
}

void DataTransformer::BinToCsv(const std::string & source_file, const std::string & dest_file, int count, int size) {
  TimeController tc;
  tc.StartTimer();
  std::string dfile = dest_file;
  if (dest_file == "") {
    dfile = Split(source_file, '.').front() + ".csv";
  }
  FILE* csv = fopen(dfile.c_str(), "w");
  if (!csv) {
    printf("csvfile %s open failed!\n", dfile.c_str());
    exit(1);
  }
  ifstream input(source_file.c_str(), ios::app | ios::in | ios::binary);
  if (!input) {
    printf("%s not exsited!\n", source_file.c_str());
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
  label += source_file + "->" + dfile + "";
  tc.EndTimer(label);
  ShowProcess(count, size);
}

void DataTransformer::GzToCsv(const std::string & source_file, const std::string & dest_file, int count, int size) {
  TimeController tc;
  tc.StartTimer();
  FILE* f = fopen(dest_file.c_str(), "w");
  if (!f) {
    printf("open file failed! %s\n", dest_file.c_str());
    exit(1);
  }
  gzFile gzfp = gzopen(source_file.c_str(), "rb");
  if (!gzfp) {
    printf("open source_file failed! %s\n", source_file.c_str());
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
  label += source_file + "->" + dest_file + "";
  tc.EndTimer(label);
  ShowProcess(count, size);
}

void DataTransformer::ShowProcess(int count, int size) {
  if (count % 10 == 1) {
    printf("handling [%d/%d]\n", count, size);
  }
}

void DataTransformer::AllBinToLog(const std::string & source_file, const std::string & dest_file) const {
  std::vector<std::string> files = GetFiles(source_file);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(BinToLog, f, GetDir(f) + "data.log", ++count, size);
  }
}

void DataTransformer::AllLogToBin(const std::string & source_file, const std::string & dest_file, const std::string & data_file_name) const {
  std::vector<std::string> files = GetFiles(source_file, data_file_name);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(LogToBin, f, GetDir(f) + "data_binary.dat", ++count, size);
  }
}

void DataTransformer::AllBinToCsv(const std::string & source_file, const std::string & dest_file) const {
  std::vector<std::string> files = GetFiles(source_file);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(BinToCsv, f, GetDir(f) + "data.csv", ++count, size);
  }
}

void DataTransformer::AllCsvToBin(const std::string & source_file, const std::string & dest_file) const {
  std::vector<std::string> files = GetFiles(dest_file);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(CsvToBin, f, GetDir(f) + "data_binary.dat", ++count, size);
  }
}

void DataTransformer::AllGzToCsv(const std::string & source_file, const std::string & dest_file) const {
  std::vector<std::string> files = GetFiles(dest_file);
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(GzToCsv, f, GetDir(f) + "data_binary.dat.gz", ++count, size);
  }
}

void DataTransformer::AllCat(const std::string & source_file, const std::string & dest_file) const {
  std::vector<std::string> files = GetFiles(source_file, "data_binary.dat");
  int size = files.size();
  int count = 0;
  for (auto f : files) {
    pool->enqueue(CatContract, f, ++count, size);
  }
}

void DataTransformer::CatContract(const std::string & file_name, int count, int size) {
  TimeController tc;
  printf("handling %s\n", file_name.c_str());
  tc.StartTimer();
  HistoryWorker ct(file_name);
  std::string dir = GetDir(file_name);
  std::string csv_file = dir + "data.csv";
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
  ShowProcess(count, size);
}
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
  ShowProcess(count, size);
}
