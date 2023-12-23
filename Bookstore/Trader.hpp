#ifndef TRADER_HPP
#define TRADER_HPP

#include <iomanip>
#include "FileRover.hpp"

// 需要储存当下的记录总条数，也就是说，要想个办法获取当前能有的记录总条数

class BookMaster;

class TradeInfo {
 private:
  double money;
  int pos;
  int seq;  // 第几次交易

 public:
  friend class Trader;
  friend class Parser;
  TradeInfo();
  TradeInfo(const double& _money, const int& _pos, const int& _seq);
  TradeInfo(const TradeInfo& rhs);
  ~TradeInfo() = default;

  void reInfo(const double& _money, const int& _pos, const int& _seq);

  TradeInfo& operator=(const TradeInfo& rhs);
  friend bool operator==(const TradeInfo& lhs, const TradeInfo& rhs);
  friend bool operator<(const TradeInfo& lhs, const TradeInfo& rhs);

  void Clear();
};

class Trader {
  // 暂且用double吧
  // 设计8位数应该够了吧，第一条就是99999999，以此类推
  // seq=1,key=99999999;seq=2,key=99999998
 private:
  BlockLinkedList<TradeInfo> TradeBase;
  vector<TradeInfo> retTrade;
  Node<TradeInfo> tempN;
  TradeInfo tempT;
  char tempseq[10];      // 预留一点空间
  double tempin = 0.0;   //+
  double tempout = 0.0;  //-
  double total = 0.0;    // 交易总额，实时算出
  int tempcount = 0;     // 暂时存储查询笔数
  int totalcount = -1;   // 实时存储交易笔数

 public:
  friend class TradeInfo;
  friend class BookMaster;
  friend class Parser;
  Trader();
  ~Trader() = default;

  char* intToChar(const int& _seq);
  // 返回指定的交易笔数
  bool traceBack(int count = -1);

  // 只用插入就行了
  void Insert(const double& _money, int isin);
};

// 也就是，记录历史交易总笔数与每一笔交易的盈亏

#endif  // !TRADER_HPP