#ifndef _PARSER_HPP
#define _PARSER_HPP

#include <array>
#include <cstring>
#include <iostream>
#include <regex>
#include <vector>
#include "AccountPiler.hpp"
#include "BookMaster.hpp"
#include "Trader.hpp"
using std::array;
using std::regex;
using std::regex_match;
using std::regex_search;
using std::sregex_token_iterator;
using std::string;
using std::vector;

class AccountPiler;

// parser is the client interface and
// i'll try to parse with regex

void cutString(const string& line, vector<string>& vec);

double stringToFloat(const string& fl);

int stringToInt(const string& in);

extern regex cutReg;      // 以连续空格分割
extern regex cutEq;       // 以第一个等号进行分割
extern regex visible;     // 除不可见字符外ascll字符
extern regex alnunder;    // 数字，字母，下划线
extern regex hasNoBrace;  // 只有头尾有双引号
extern regex isInt;       // 合法数字
extern regex isFloat;     // 合法float
extern regex multiKey;    // keyword是否有多个

// Parser:the total interface!
class Parser {
 private:
  AccountPiler Piler;
  vector<string> stringBusket;
  vector<string> smallerBusket;  // 存放多元指令中更小的一部分
  string line;
  string token;

 public:
  // some utils

  Parser();
  ~Parser() = default;
  friend class AccountPiler;
  friend class BookMaster;
  friend class AccountStack;
  // get one line from stdin
  void getString();

  // function allocator, also judge the format
  bool funcAllocate();
};

#endif  // !PARSER_HPP