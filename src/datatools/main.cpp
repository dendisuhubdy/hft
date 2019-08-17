#include <DataTransformer.h>
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
  }
  // df.BinToLog(binary_path, log_path);
  // df.LogToBin(log_path, binary_path);
  df.AllCat("/running");
}
