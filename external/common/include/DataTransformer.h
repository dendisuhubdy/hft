#ifndef DATATRANSFORMER_H_
#define DATATRANSFORMER_H_

#include <iostream>
#include <fstream>
#include <memory>
#include <market_snapshot.h>
#include <common_tools.h>
#include <timecontroller.h>
#include <ThreadPool.h>
#include <Contractor.h>
#include <zlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/sysinfo.h>

class DataTransformer {
 public:
  DataTransformer(int num);
  DataTransformer();
  ~DataTransformer();
  static void BinToLog(const std::string & source_file, const std::string & dest_file="", int count = 0, int size = 0);
  static void LogToBin(const std::string & source_path, const std::string & dest_file="", int count = 0, int size = 0);
  static void BinToCsv(const std::string & source_path, const std::string & dest_path="", int count = 0, int size = 0);
  static void CsvToBin(const std::string & source_path, const std::string & dest_path="", int count = 0, int size = 0);
  static void GzToCsv(const std::string & source_path, const std::string & dest_path="", int count = 0, int size = 0);
  static void ShowProcess(int count, int size);

  // multi-thread method
  void AllBinToLog(const std::string & source_dir, const std::string & dest_file="") const;
  void AllLogToBin(const std::string & source_dir, const std::string & dest_file="", const std::string & data_file_name = "data.log") const;
  void AllBinToCsv(const std::string & source_dir, const std::string & dest_file="") const;
  void AllCsvToBin(const std::string & source_dir, const std::string & dest_file="") const;
  void AllGzToCsv(const std::string & source_dir, const std::string & dest_file="") const;

  static void CatContract(const std::string & file_name, int count, int size);

  void AllCat(const std::string & source_dir, const std::string & dest_file ="") const;

 private:
  TimeController* tc;
  int cpu_count;
  ThreadPool * pool;
};

#endif  // DATATRANSFORMER_H_
