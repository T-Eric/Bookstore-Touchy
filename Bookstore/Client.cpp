#include "AccountPiler.hpp"
#include "BookMaster.hpp"
#include "FileRover.hpp"
#include "LogRecorder.hpp"
#include "Parser.hpp"
#include "Trader.hpp"

int main() {
  // freopen("in.in", "r", stdin);
  // freopen("out.out", "w", stdout);
  Parser core;
  char ch;
  string input;
  int i = 0;
  while (true) {
    // i++;
    // cout << i << ' ';
    getline(std::cin, input);
    core.getString(input);
    if (!core.funcAllocate())
      cout << "Invalid\n";
  }
}