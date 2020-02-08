#include <header.h>
#include <data_transformer.h>
#include <timecontroller.h>
#include <string>

int main() {
  TimeController tc;
  tc.StartTimer();
  DataTransformer dt;
  std::string dir = "/running";
  dt.AllLogToBin(dir);
  dt.AllLogToBin(dir, "", "data_night.log");
  tc.EndTimer("alllogtobin "+ dir);
}
