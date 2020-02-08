#include <data_transformer.h>
#include <common_tools.h>
#include <string>

int main(int argc, char *argv[]) {
  DataTransformer df;
  std::string binary_path = "data_binary.dat";
  std::string log_path = "";
  if (argc > 2) {
    binary_path = argv[1];
    log_path = argv[2];
  } else if (argc == 2) {
    binary_path = argv[1];
  } else {
    printf("demo: ./datatools 2019-12-10/data_binary.dat.gz 2019-12-10/data.csv");
    exit(1);
  }
  // df.BinToLog(binary_path, log_path);
  // df.LogToBin(log_path, binary_path);
  df.GzToCsv(binary_path, log_path);
  // df.AllCat("/running/2020-01-09");
  auto v = Split(log_path, "/");
  std::string dir;
  v.pop_back();
  for (auto s:v) {
    dir += s;
    dir += "/";
  }
  printf("dir is %s\n", dir.c_str());
  // df.AllCat(dir);
  df.FSYCatContract(binary_path, log_path, 50, 550);
  // df.CatContract(binary_path, 50, 550);
}
