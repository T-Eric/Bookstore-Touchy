#ifndef PILER_HPP
#define PILER_HPP

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <regex>
#include <stack>
#include "BookMaster.hpp"
#include "FileRover.hpp"
using std::array;
using std::stack;

class Account;
class AccountStack;
class AccountPiler;

// the base use uid(unique) as the key
class Account {
 private:
  int privilege;
  array<char, 32> uid;
  array<char, 32> password;
  array<char, 32> uname;

 public:
  friend class AccountStack;
  friend class AccountPiler;

  Account();
  Account(const string& _uid, const string& _pwd, const string& _unm, int _pv = 1);
  Account(const Account& rhs);
  ~Account() = default;

  void Clear();
  void reAcc(const string& _uid, const string& _pwd, const string& _unm, int _pv = 1);
  void changePwd(const string& _pwd);
  Account& operator=(const Account& rhs);
  friend bool operator==(const Account& lhs, const Account& rhs);
  friend bool operator<(const Account& lhs, const Account& rhs);
};

class AccountStack {
  stack<Account> Accountstack;  // active user
  stack<BookInfo> Bookstack;    // selected book
  stack<Account> Tempstack;
  stack<BookInfo> tempstack;
  BookInfo tempinfo;

  // 貌似实际上只要在登录、登出时改变这里的selected就可以了

 public:
  friend class Account;
  friend class AccountPiler;
  friend class BookMaster;
  AccountStack();
  ~AccountStack();

  bool comp_strarr(const string& str, const array<char, 32>& arr);

  // 用这个函数统一接在所有与selected改变有关的指令后面
  void changeSelected(const BookInfo& info);  // 与Login、select同步调用

  void modifySelected(const BookInfo& info);  // 与modify同步调用，为了将栈中的相关书全部修改

  bool inStack(const string& _uid);
};

class AccountPiler {
 private:
  AccountStack Stack;
  BookMaster Master;

  BlockLinkedList<Account> Accountbase;
  array<char, 32> temparr;
  Account active;          // current stacktop
  vector<Account> retacc;  // returned account from findAccount
  Account tempacc;
  char tofind[32];  // uid to find
  Node<Account> tempN;

 public:
  friend class Account;
  friend class AccountStack;
  friend class Parser;
  AccountPiler();
  ~AccountPiler() = default;

  void Init();

  int getPrivilege() { return active.privilege; }
  char* arrayToChar(const array<char, 32>& item);
  bool comp_strarr(const string& str, const array<char, 32>& arr);

  // 0
  bool findAccount(const char _uid[]);

  // 0
  bool Login(const string& _uid, const string& _pwd = "");

  // 1
  bool Logout();

  // 0
  bool Register(const string& _uid, const string& _pwd, const string& _unm);

  // 1
  bool revisePwd(const string& _uid, const string& newpwd, const string& curpwd = "");

  // 3
  bool Create(const string& _uid, const string& _pwd, const int& _pv, const string& _unm);

  // 7
  bool Delete(const string& _uid);
};

// 虽然login和logout是在这里执行的，但还是要在对外接口那里重新同步
// 在外部接口，同时做三个函数的同步

#endif  // !USER_H