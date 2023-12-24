#include "AccountPiler.hpp"
#include "BookMaster.hpp"
#include "FileRover.hpp"
#include "LogRecorder.hpp"
#include "Parser.hpp"
#include "Trader.hpp"

int main() {
  freopen("in.in", "r", stdin);
  Parser core;
  int i = 0;
  while (true) {
    i++;
    cout << i << '\n';
    core.getString();
    if (!core.funcAllocate())
      cout << "Invalid\n";
  }
}