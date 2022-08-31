#include <iostream>
#include <fstream>
#include <string>
using namespace std;

extern "C" void query(int64_t, int64_t*, int64_t*, int64_t*, int64_t*);

int main () {
  string s;
  ifstream myfile;
  myfile.open("/home/michel/MasterThesis/dbgen/lineitem2.tbl");

  int row_count = 1;

  int64_t shipdate[row_count];
  int64_t extendedprice[row_count];
  int64_t discount[row_count];
  int64_t quantity[row_count];
  if ( myfile.is_open() ) {
    std::string delimiter = "|";
    for(int k = 0; k < row_count; k++) {
      std::getline(myfile, s);
      for(int i = 0; i < 16; i++) {
        std::string token = s.substr(0, s.find(delimiter));
        int j;
        if(i == 4) {
          j = std::stoi(token);
          quantity[k] = j;
        }
        if(i == 5) {
          j = std::stoi(token);
          extendedprice[k] = j;
        }
        if(i == 6) {
          j = std::stoi(token);
          discount[k] = j;
        }
        if(i == 10) {
          j = std::stoi(token);
          shipdate[k] = j;
        }
        s.erase(0, s.find(delimiter) + delimiter.length());
      }
    }
  }


  query(row_count, shipdate, discount, quantity, extendedprice);

  return 0;
}
