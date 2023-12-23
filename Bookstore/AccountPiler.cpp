#include "AccountPiler.hpp"

Account::Account() {
  privilege = 0;
  uid.fill(0);
  password.fill(0);
  uname.fill(0);
}

Account::Account(const string& _uid, const string& _pwd, const string& _unm, int _pv) {
  privilege = _pv;
  uid.fill(0);
  password.fill(0);
  uname.fill(0);
  for (int i = 0; i < _uid.size(); i++)
    uid[i] = _uid[i];
  for (int i = 0; i < _pwd.size(); i++)
    password[i] = _pwd[i];
  for (int i = 0; i < _unm.size(); i++)
    uname[i] = _unm[i];
}

Account::Account(const Account& rhs) {
  privilege = rhs.privilege;
  uid = rhs.uid;
  password = rhs.password;
  uname = rhs.uname;
}

void Account::Clear() {
  privilege = 0;
  uid.fill(0);
  password.fill(0);
  uname.fill(0);
}

void Account::reAcc(const string& _uid, const string& _pwd, const string& _unm, int _pv) {
  privilege = _pv;
  uid.fill(0);
  password.fill(0);
  uname.fill(0);
  for (int i = 0; i < _uid.size(); i++)
    uid[i] = _uid[i];
  for (int i = 0; i < _pwd.size(); i++)
    password[i] = _pwd[i];
  for (int i = 0; i < _unm.size(); i++)
    uname[i] = _unm[i];
}

void Account::changePwd(const string& _pwd) {
  password.fill(0);
  for (int i = 0; i < _pwd.size(); i++)
    password[i] = _pwd[i];
}

Account& Account::operator=(const Account& rhs) {
  privilege = rhs.privilege;
  uid = rhs.uid;
  password = rhs.password;
  uname = rhs.uname;
  return *this;
}

bool operator==(const Account& lhs, const Account& rhs) {
  return lhs.uid == rhs.uid;
}

bool operator<(const Account& lhs, const Account& rhs) {
  return lhs.uid < rhs.uid;
}

AccountStack::AccountStack() = default;
AccountStack::~AccountStack() = default;

bool AccountStack::comp_strarr(const string& str, const array<char, 32>& arr) {
  for (int i = 0; i < std::min(str.size(), arr.size()); i++) {
    if (arr[i] != str[i])
      return false;
  }
  return true;
}

void AccountStack::changeSelected(const BookInfo& info) {
  Bookstack.pop();
  Bookstack.push(info);
}

bool AccountStack::inStack(const string& _uid) {
  while (!Accountstack.empty()) {
    Tempstack.push(Accountstack.top());
    Accountstack.pop();
    if (comp_strarr(_uid, Tempstack.top().uid)) {
      while (!Tempstack.empty()) {
        Accountstack.push(Tempstack.top());
        Tempstack.pop();
      }
      return true;
    }
  }
  while (!Tempstack.empty()) {
    Accountstack.push(Tempstack.top());
    Tempstack.pop();
  }
  return false;
}

AccountPiler::AccountPiler() {
  active = Account();
  Accountbase.setList("account.dat");
  temparr.fill(0);
}

void AccountPiler::Init() {
  active = Account("temp", "temp", "temp", 7);
  Create("root", "sjtu", 7, "root");
  active = Account();
}

char* AccountPiler::arrayToChar(const array<char, 32>& item) {
  for (int i = 0; i < item.size(); i++)
    tofind[i] = item[i];
  return tofind;
}

bool AccountPiler::comp_strarr(const string& str, const array<char, 32>& arr) {
  for (int i = 0; i < std::min(str.size(), arr.size()); i++) {
    if (arr[i] != str[i])
      return false;
  }
  return true;
}

bool AccountPiler::findAccount(const char _uid[]) {
  Accountbase.Find(_uid, retacc);
  if (retacc.empty())
    return false;
  return true;
}

bool AccountPiler::Login(const string& _uid, const string& _pwd) {
  if (findAccount(_uid.c_str())) {
    if (active.privilege >= retacc[0].privilege || _pwd != "" && comp_strarr(_pwd, retacc[0].password)) {
      Stack.Accountstack.push(retacc[0]);
      Stack.Bookstack.push(BookInfo());  // 压入储存量为-1的空书，表示没有选中
      active = retacc[0];
      Master.unSelect();
      return true;
    } else
      return false;
  } else
    return false;
  // 登陆的时候，至少要压入一本空书
}

bool AccountPiler::Logout() {
  if (active.privilege < 1)
    return false;
  if (Stack.Accountstack.empty())
    return false;
  Stack.Accountstack.pop();
  Stack.Bookstack.pop();
  if (Stack.Accountstack.empty()) {
    active = Account();
    Master.unSelect();
  }
  active = Stack.Accountstack.top();
  if (Stack.Bookstack.top().storage != -1)
    Master.Select(Master.arrayToChar(Stack.Bookstack.top().isbn));
  else
    Master.unSelect();
  return true;
}

bool AccountPiler::Register(const string& _uid, const string& _pwd, const string& _unm) {
  if (findAccount(_uid.c_str()))
    return false;
  tempacc.reAcc(_uid, _pwd, _unm);
  tempN.reNode(arrayToChar(tempacc.uid), tempacc);
  Accountbase.Insert(tempN);
  return true;
}

// 默认参数只能指定一次
bool AccountPiler::revisePwd(const string& _uid, const string& newpwd, const string& curpwd) {
  if (active.privilege >= 1 && findAccount(_uid.c_str())) {
    if (curpwd == "" && active.privilege == 7 || comp_strarr(curpwd, retacc[0].password)) {
      retacc[0].changePwd(newpwd);
      tempN.reNode(arrayToChar(retacc[0].uid), retacc[0]);
      Accountbase.sameModify(tempN);
      return true;
    } else
      return false;
  } else
    return false;
}

bool AccountPiler::Create(const string& _uid, const string& _pwd, const int& _pv, const string& _unm) {
  if (_pv >= active.privilege || active.privilege < 3)
    return false;
  if (findAccount(_uid.c_str()))
    return false;
  Account New(_uid, _pwd, _unm, _pv);
  tempN.reNode(arrayToChar(New.uid), New);
  Accountbase.Insert(tempN);
  return true;
}

bool AccountPiler::Delete(const string& _uid) {
  if (active.privilege < 7 || Stack.inStack(_uid))
    return false;
  if (findAccount(_uid.c_str())) {
    tempN.reNode(arrayToChar(retacc[0].uid), retacc[0]);
    Accountbase.Delete(tempN);
    return true;
  } else
    return false;
}
