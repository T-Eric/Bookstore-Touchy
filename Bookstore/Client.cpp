#include "AccountPiler.hpp"
#include "BookMaster.hpp"
#include "FileRover.hpp"
#include "LogRecorder.hpp"
#include "Parser.hpp"
#include "Trader.hpp"

int main() {
  FILE* file = freopen("in.in", "r", stdin);
  Parser sys;
  while (!std::feof(file)) {
    sys.getString();
    sys.funcAllocate();
  }
}