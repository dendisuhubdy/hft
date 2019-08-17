#include <iostream>
#include <string>
#include <vector>

#include "simdata/datagener.h"

#define MAXN 10000000

// using namespace std;
DataGener::DataGener(std::string des_name, std::string data_file)
  : file_name(data_file),
    context(NULL),
    socket(NULL) {
  sender = new Sender("10.20.2.37:10021", "bind", "tcp");
}

DataGener::~DataGener() {
  raw_file.close();
  delete sender;
}

void DataGener::Run() {
  std::cout << file_name << endl;
  char buffer[SIZE_OF_SNAPSHOT];
  while (true) {
    raw_file.open(file_name.c_str(), ios::in);
    // ofstream output;
    // output.open("binary.dat", ios::out | ios::binary);
    // output.write((char*)&m_book[0], m_book.size()*sizeof(Tick));

    while (raw_file.getline(buffer, SIZE_OF_SNAPSHOT)) {
      // raw_file.getline(buffer, SIZE_OF_SNAPSHOT);
      // std::cout << buffer << endl;
      if (buffer[0] == '\0') {
        break;
      }
      MarketSnapshot snapshot = HandleSnapshot(buffer);
      // HandleSnapshot(buffer);
      /*
      if (count++ % 100000 == 0) {
        snapshot.Show(stdout);
      }
      */
      sender->Send(snapshot);
      usleep(1000);
      /*
      // output.write(reinterpret_cast<char*>(&snapshot), sizeof(snapshot));
      if (count++ % 1000000 == 0) {
        snapshot.Show(stdout);
      }
      output.write(reinterpret_cast<char*>(&snapshot), sizeof(snapshot));
      // std::cout << "send" << c << " out" << endl;
      // sleep(1);
      */
    }
    raw_file.close();
  }
  printf("readfile over!\n");
}
