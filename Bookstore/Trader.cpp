#include "Trader.hpp"

TradeInfo::TradeInfo() {
  money = 0.0;
  pos = 0;
  seq = 0;
}

TradeInfo::TradeInfo(const double& _money, const int& _pos, const int& _seq) {
  money = _money;
  pos = _pos;
  seq = _seq;
}

TradeInfo::TradeInfo(const TradeInfo& rhs) {
  money = rhs.money;
  pos = rhs.pos;
  seq = rhs.seq;
}

void TradeInfo::reInfo(const double& _money, const int& _pos, const int& _seq) {
  money = _money;
  pos = _pos;
  seq = _seq;
}

void TradeInfo::Clear() {
  money = 0.0;
  pos = 0.0;
  seq = 0;
}

TradeInfo& TradeInfo::operator=(const TradeInfo& rhs) {
  money = rhs.money;
  pos = rhs.pos;
  seq = rhs.seq;
  return *this;
}

bool operator==(const TradeInfo& lhs, const TradeInfo& rhs) {
  return lhs.seq == rhs.seq;
}

bool operator<(const TradeInfo& lhs, const TradeInfo& rhs) {
  return lhs.seq < rhs.seq;
}

Trader::Trader() {
  TradeBase.setList("trade.dat");
  totalcount = TradeBase.getLen();
}

// 创造8位key
char* Trader::intToChar(const int& _seq) {
  int key = 100000000 - _seq;
  memset(tempseq, '0', sizeof(tempseq));
  for (int i = _seq, j = 9; i != 0; i /= 10, j--) {
    tempseq[j] = i % 10 + '0';
  }
  return tempseq;
}

bool Trader::traceBack(int count) {
  if (count > totalcount)
    return false;
  if (count == 0) {
    cout << '\n';
    return true;
  }
  tempin = 0.0, tempout = 0.0;
  if (count == -1) {
    TradeBase.findAll(retTrade);
  } else {
    TradeBase.findFront(count, retTrade);
  }
  for (const TradeInfo& info : retTrade) {
    if (info.pos)
      tempin += info.money;
    else
      tempout += info.money;
  }
  cout << "+ " << std::fixed << std::setprecision(2) << tempin << " - " << std::fixed << std::setprecision(2) << tempout << '\n';
  return true;
}

void Trader::Insert(const double& _money, int isin) {
  totalcount++;
  tempT.reInfo(_money, isin, totalcount);
  tempN.reNode(intToChar(totalcount), tempT);
  TradeBase.Insert(tempN);
}
