#include "Parser.hpp"

regex cutReg("[\\s]+");                                       // 以连续空格分割
regex cutEq("^(.+?)=(.+)$");                                  // 以第一个等号进行分割
regex visible("^[[:graph:]]{1,}$");                           // 除不可见字符外ascll字符
regex alnunder("^[0-9a-zA-Z_]{1,}$");                         // 数字，字母，下划线
regex hasNoBrace("^\"[^\"]*\"$");                             // 只有头尾有双引号
regex isInt("^[1-9]{1}\\d{0,9}$");                            // 合法数字
regex isFloat("^[0-9]{1,10}+\\.[0-9]{1,2}$|0|0\\.0|0\\.00");  // 合法float

Parser::Parser() {
  Piler.Init();
}

void cutString(const string& line, vector<string>& vec) {
  vec.clear();
  vec = {std::sregex_token_iterator(line.begin(), line.end(), cutReg, -1), std::sregex_token_iterator()};
}

double stringToFloat(const string& fl) {
  return std::stod(fl);
}

int stringToInt(const string& in) {
  return std::stoi(in);
}

void Parser::getString(const string& _line) {
  line = _line;
}
/**
 * 记得以下坑要填：
 * 1.每条操作之后要记录对应日志
 * 2.buy操作中，需要trade信息修改
 * 3.import操作中，需要trade信息修改
 * （考虑整合在bookMaster里面）
 */
bool Parser::funcAllocate() {
  if (line.empty()) {
    Piler.~AccountPiler();
    stringBusket.clear();
    smallerBusket.clear();
    line.clear();
    token.clear();
    exit(0);
  }
  // 使用search完全匹配
  stringBusket.clear();
  stringBusket = {std::sregex_token_iterator(line.begin(), line.end(), cutReg, -1),
                  std::sregex_token_iterator()};
  if (stringBusket.empty()) {
    cout << '\n';
    return true;
  }
  if (stringBusket[0].empty())
    stringBusket.erase(stringBusket.begin());  // 开头可能有空串
  if (stringBusket.empty()) {
    cout << '\n';
    return true;
  }

  // su [UserID] ([Password])?
  if (stringBusket[0] == "su") {
    if (stringBusket.size() > 3)
      return false;
    if (!regex_match(stringBusket[1], alnunder) || stringBusket[1].size() > 30)
      return false;
    if (stringBusket.size() == 2)
      stringBusket.push_back("");
    else if (stringBusket[2].size() > 30 || !regex_match(stringBusket[2], alnunder))
      return false;
    if (!Piler.Login(stringBusket[1], stringBusket[2]))
      return false;
    return true;
  }

  // logout
  else if (stringBusket[0] == "logout") {
    if (stringBusket.size() > 1)
      return false;
    if (!Piler.Logout())
      return false;
    return true;
  }

  // register [UserID] [Password] [Username]
  else if (stringBusket[0] == "register") {
    if (stringBusket.size() != 4)
      return false;
    for (const string& str : stringBusket)
      if (str.size() > 30)
        return false;
    if (!(regex_match(stringBusket[1], alnunder) && regex_match(stringBusket[2], alnunder) && regex_match(stringBusket[3], visible)))
      return false;
    if (!Piler.Register(stringBusket[1], stringBusket[2], stringBusket[3]))
      return false;
    return true;
  }

  // passwd [UserID] ([CurrentPassword])? [NewPassword]
  else if (stringBusket[0] == "passwd") {
    if (stringBusket.size() != 4 && stringBusket.size() != 3)
      return false;
    for (const string& str : stringBusket)
      if (str.size() > 30)
        return false;
    if (stringBusket.size() == 3) {
      if (!(regex_match(stringBusket[1], alnunder) && regex_match(stringBusket[2], alnunder)))
        return false;
      if (!Piler.revisePwd(stringBusket[1], stringBusket[2]))
        return false;
      return true;
    } else {
      if (!(regex_match(stringBusket[1], alnunder) && regex_match(stringBusket[2], alnunder)) && regex_match(stringBusket[3], alnunder))
        return false;
      if (!Piler.revisePwd(stringBusket[1], stringBusket[3], stringBusket[2]))
        return false;
      return true;
    }
  }

  // useradd [UserID] [Password] [Privilege] [Username]
  else if (stringBusket[0] == "useradd") {
    if (stringBusket.size() != 5)
      return false;
    for (const string& str : stringBusket)
      if (str.size() > 30)
        return false;
    if (!(regex_match(stringBusket[1], alnunder) && regex_match(stringBusket[2], alnunder)) && regex_match(stringBusket[3], isInt) && regex_match(stringBusket[4], visible))
      return false;
    if (!Piler.Create(stringBusket[1], stringBusket[2], stringToInt(stringBusket[3]), stringBusket[4]))
      return false;
    return true;
  }

  // delete [UserID]
  else if (stringBusket[0] == "delete") {
    if (stringBusket.size() != 2)
      return false;
    if (stringBusket[1].size() > 30)
      return false;
    if (!(regex_match(stringBusket[1], alnunder)))
      return false;
    if (!Piler.Delete(stringBusket[1]))
      return false;
    return true;
  }

  // 接下来有一个问题：实际上不能用等号来分割，因为“所有可见字符”包括等号（就很离谱）
  //  show (-ISBN=[ISBN] | -name="[BookName]" | -author="[Author]" | -keyword="[Keyword]")?
  else if (stringBusket[0] == "show") {
    // show finance (count)?
    if (Piler.getPrivilege() >= 7 && stringBusket.size() >= 2 && stringBusket[1] == "finance") {
      if (stringBusket.size() == 2) {
        if (!Piler.Master.Trade.traceBack())
          return false;
        return true;
      } else if (stringBusket.size() == 3) {
        if (!regex_match(stringBusket[2], isInt))
          return false;
        if (!Piler.Master.Trade.traceBack(stringToInt(stringBusket[2])))
          return false;
        return true;
      } else
        return false;
    }
    if (Piler.getPrivilege() < 1)
      return false;
    if (stringBusket.size() < 1 && stringBusket.size() > 2)
      return false;
    if (stringBusket.size() == 1) {  // show
      Piler.Master.Search();
      Piler.Master.show();
      return true;
    }
    // 只会出现一种搜索要求
    std::smatch match;
    if (regex_search(stringBusket[1], match, cutEq)) {
      token = match[2];
      if (token.empty() || !regex_match(token, visible))
        return false;
      if (match[1] == "-ISBN") {
        if (token.size() > 20)
          return false;
        Piler.Master.Search(token.c_str(), ISBN);
        Piler.Master.show();
        return true;
      }
      if (!regex_match(token, hasNoBrace))
        return false;
      // 此时，token保证是一个头尾为"的字符串
      token.erase(token.begin());
      token.pop_back();
      if (token.empty() || token.size() > 60)
        return false;

      if (match[1] == "-name") {
        Piler.Master.Search(token.c_str(), BOOKNAME);
        Piler.Master.show();
        return true;
      } else if (match[1] == "-author") {
        Piler.Master.Search(token.c_str(), AUTHOR);
        Piler.Master.show();
        return true;
      } else if (match[1] == "-keyword") {
        Piler.Master.Search(token.c_str(), KEYWORD);
        Piler.Master.show();
        return true;
      }
    } else
      return false;
  }

  // buy [ISBN] [Quantity]
  else if (stringBusket[0] == "buy") {
    if (Piler.getPrivilege() < 1)
      return false;
    if (stringBusket.size() != 3)
      return false;
    if (!regex_match(stringBusket[1], visible) || !regex_match(stringBusket[2], isInt))
      return false;
    // isbn
    if (stringBusket[1].size() > 20)
      return false;
    if (!Piler.Master.Buy(stringBusket[1].c_str(), stringToInt(stringBusket[2])))
      return false;
    return true;
  }

  // select [ISBN]
  else if (stringBusket[0] == "select") {
    if (Piler.getPrivilege() < 3)
      return false;
    if (stringBusket.size() != 2)
      return false;
    if (!regex_match(stringBusket[1], visible) || stringBusket[1].size() > 20)
      return false;
    Piler.Master.Select(stringBusket[1].c_str());
    Piler.Stack.changeSelected(Piler.Master.selected);
    return true;
  }

  // modify (-ISBN=[ISBN] | -name="[BookName]" | -author="[Author]"
  //| -keyword="[Keyword]" | -price=[Price])+
  else if (stringBusket[0] == "modify") {
    if (Piler.getPrivilege() < 3)
      return false;
    if (stringBusket.size() < 2 || stringBusket.size() > 6)
      return false;
    bool isisbn = false, isname = false, isauthor = false, iskeyword = false, isprice = false;
    // 预处理每个块
    for (int i = 1; i < stringBusket.size(); i++) {
      std::smatch match;
      if (regex_search(stringBusket[i], match, cutEq)) {
        token = match[2];
        if (token.empty() || !regex_match(token, visible) || match.size() > 3)
          return false;

        if (match[1] == "-ISBN") {
          if (isisbn) {
            Piler.Master.resetModify();
            return false;
          }
          if (token.empty() || token.size() > 20) {
            Piler.Master.resetModify();
            return false;
          }
          isisbn = true;
          if (!Piler.Master.preModify(token)) {
            Piler.Master.resetModify();
            return false;
          }
        } else if (match[1] == "-price") {
          if (isprice) {
            Piler.Master.resetModify();
            return false;
          }
          if (!regex_match(token, isFloat) && !regex_match(token, isInt)) {
            Piler.Master.resetModify();
            return false;
          }
          isprice = true;
          if (!Piler.Master.preModify("", "", "", "", stringToFloat(token))) {
            Piler.Master.resetModify();
            return false;
          }
        } else {
          // 对token去双引号
          if (!regex_match(token, hasNoBrace)) {
            Piler.Master.resetModify();
            return false;
          }
          // 此时，token保证是一个头尾为"的字符串
          token.erase(token.begin());
          token.pop_back();
          if (token.empty() || token.size() > 60) {
            Piler.Master.resetModify();
            return false;
          }
          if (match[1] == "-name") {
            if (isname) {
              Piler.Master.resetModify();
              return false;
            }
            isname = true;
            if (!Piler.Master.preModify("", token)) {
              Piler.Master.resetModify();
              return false;
            }
          } else if (match[1] == "-author") {
            if (isauthor) {
              Piler.Master.resetModify();
              return false;
            }
            isauthor = true;
            if (!Piler.Master.preModify("", "", token)) {
              Piler.Master.resetModify();
              return false;
            }
          } else if (match[1] == "-keyword") {
            if (iskeyword) {
              Piler.Master.resetModify();
              return false;
            }
            // 查重
            iskeyword = true;
            if (!Piler.Master.preModify("", "", "", token)) {
              Piler.Master.resetModify();
              return false;
            }
          } else {
            Piler.Master.resetModify();
            return false;
          }
        }
      }
    }
    // 开始Modify
    if (!Piler.Master.Modify(isisbn, isname, isauthor, iskeyword, isprice)) {
      Piler.Master.resetModify();
      return false;
    }
    Piler.Stack.modifySelected(Piler.Master.selected);
    return true;
  }

  // import [Quantity] [TotalCost]
  else if (stringBusket[0] == "import") {
    if (Piler.getPrivilege() < 3)
      return false;
    if (stringBusket.size() != 3)
      return false;
    if (!regex_match(stringBusket[1], isInt) || !regex_match(stringBusket[2], isFloat) && !regex_match(stringBusket[2], isInt))
      return false;
    if (!Piler.Master.Import(stringToInt(stringBusket[1]), stringToFloat(stringBusket[2])))
      return false;
    return true;
  }

  // quit&exit
  else if (stringBusket[0] == "quit" || stringBusket[0] == "exit") {
    Piler.~AccountPiler();
    stringBusket.clear();
    smallerBusket.clear();
    line.clear();
    token.clear();
    exit(0);
  }
  return false;

  // show finance ([Count])?
  // log
  // report finance
  // report employee
}