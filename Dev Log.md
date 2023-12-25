# DevLog

**Project:** Bookstore-Touchy

**<u>Dever</u>:** TEric ( JHT )

## 程序功能概述

这是一个很简单的 Bookstore，具有以下主要功能：

- 账户

  - 创建或注册带有自定义用户名与权限等级的用户
  - 登录账户，以账户的名义执行该权限下的可用操作，保持实时信息的独立（选中图书独立保存）

- 图书+采购销售操作

  - 普通用户（权限1）：
    - 可以以 ISBN 号、书名、作者、关键词中的任意一种查找项进行书本查找
    - 可以根据 ISBN 号购买某本书
  - 销售人员（权限3）：
    - 可以以 ISBN 号选中一本书，对其进行任意信息的合法修改
    - 可以给选中的书进货以增加库存
    - 在选中不存在的书时，可以创建一本空信息的书，而后可对其进行修改

- 日志信息记录

  所有用户的所有成功操作，都会记录到系统日志中

  所有用户的成功采购、销售操作产生的金额流动，都会记录到财务日志中

  所有销售人员（权限3）及以上的所有成功操作，都会记录到工作日志中

## 主体逻辑

主体分为前端与后端。

- 前端（包括 `main` 函数与 `Parser` 、`LogRecorder `类）负责获取并解析指令。

  - `Parser` 通过正则表达式与字符串操作检查其格式合法性，并拆解成对应指令的参数传给后端接口；如果指令执行成功（返回 `true` ），则将各个参数传给 `LogRecorder` 用于记录操作。
  - `LogRecorder` 在接受成功指令及其参数后，可以记录之。

- 后端（包括 `AccountPiler`, `BookMaster`, `Trader` 及其下的小类 ）负责登录栈、选中图书栈的维护、负责数据库的维护（读写）、负责查找等用户操作，是典型的根据输入获得输出的“工程题”。

  特别的是，几乎每个接口（除了非常底层的类型转换函数）都返回 `bool` 值，以指示这一步操作是否成功。这对程序报错、中断后恢复先前数据十分重要。

总体来说，前端解析一行指令，交由后端处理，后端在处理后告知处理结果、维护数据库、进行可能的输出。

## 代码结构

`main` 函数与各个类之间几乎呈线性的结构。这是为了更加方便各个类之间共享信息、同步修改，并减少类间接口。

代码目录：

```
┳ Bookstore
┗┳ AccountPiler.cpp  //账户、登录栈
 ┣ AccountPiler.hpp
 ┣ BookMaster.cpp    //图书库
 ┣ BookMaster.hpp
 ┣ Client.cpp
 ┣ FileRover.hpp
 ┣ LogRecorder.cpp   //日志
 ┣ LogRecorder.hpp
 ┣ Parser.cpp        //指令分解、任务分配
 ┣ Parser.hpp
 ┣ Trader.cpp        //交易所
 ┣ Trader.hpp
 ┣ code              //可执行文件
```

编译产物（除 `code` 外与 `CMakeLists.txt` 不在其中。

线性结构，指的是 `Client` 作为 `main` 函数所在，只有一个类 `Parser core`；

在 `Parser` 中，有成员类 `LogRecorder Logger` （日志记录）和类 `AccountPiler Piler`；

在 `AccountPiler` 中，有成员类 `AccountStack Stack`（登录栈、选中的图书栈）和类 `BookMaster Master`；

在 `BookMaster` 中，有成员类 `Trader Trade`，用于记录采销记录。

除了 `FileRover` 中的 `BlockLinkedList` 是除 `Parser` 类都在使用的类之外，其他类呈现线性的嵌套关系。

## 类接口、成员说明

有一些成员是temp型成员，即在文件中作为一个中间变量的承载者，这部分变量略去介绍（具体可以看各个 `.hpp` 文件）。一些私有类型转换函数也略去介绍。

类的所有字符串型成员都用 `std::array<char,[length]>` 存储，目的是使字符串定长，并能被对应的 `vector<array>` 容纳（数组不可容纳）。所有带存储功能的类都有一个方法：

```cpp
char* arrayToChar(const array<char,[len]>&)
```

返回类中定义的成员 `char tempkey[]` 的指针。

信息类成员如下：

- `TradeInfo` ：key值另将 ` seq` 进行字典倒序处理，本身用于记录次数、输入进Log。

```cpp
  double money;
  int pos;
  int seq;  // pseudo-key
```

- `BookInfo`：

```cpp
 private:
  array<char, 24> isbn; //key
  array<char, 64> author;
  array<char, 64> name;
  array<char, 64> key;
  long long storage = -1; //用于在select中表示未选中
  double price = 0;

public:
	friend class AccountPiler;
  friend class AccountStack;
  friend class Parser;

  bool Search(const char item[] = "", OperationType op = PRICE);

  void Select(const char _isbn[]);
  void unSelect();

  // keys包括重复段就操作失败，包含空段也操作失败
  bool cutKey(const array<char, 64>& keys_, vector<array<char, 64>>& vec);

  bool Import(int quantity, double totalcost);

  bool Buy(const char _isbn[], int quantity);

  // use bool to mark whether to modify
  void resetModify();
  bool preModify(const string& _isbn = "", const string& _name = "", const string& _author = "", const string& _keys = "", double _price = -1.0);
  bool Modify(bool isisbn = false, bool isname = false, bool isauthor = false, bool iskeyword = false, bool isprice = false);

  // outer interface
  void show();
```

采用4个数据库，分别胜任 ISBN 、书名、作者、关键词为key的存储。 `ISBNbase` 存储所有信息，而另外三个数据库只存储 `isbn` 信息。

`Search` 是私有的通用查找接口，可以分类型查找。在非 ISBN 查找下，先在其他库中找到 ISBN，用 `unique` 函数去重，再在 ISBN 库中寻书。

`Modify` 可以将 `preModify` 成功修改后的修改信息附加给 `selected`，并增删对应的数据库。

`Select` 与 `Accountpiler` 中的相关函数同步调用。

- `Account`：

```cpp
  int privilege;  //权限
  array<char, 32> uid;
  array<char, 32> password;
  array<char, 32> uname;
```

执行类如下：

- `Trader`：

```cpp
 private:  
  BlockLinkedList<TradeInfo> TradeBase; //finance数据库
  vector<TradeInfo> retTrade;
  Node<TradeInfo> tempN;
  TradeInfo tempT;
  char tempseq[10];      // key
  double tempin = 0.0;   // +
  double tempout = 0.0;  // -
  int totalcount = -1;   // 实时存储交易笔数

 public:
  bool traceBack(int count = -1);// -1全部加入

  // 只用插入就行了
  void Insert(const double& _money, int isin);
```

`traceBack` 可以回溯最后 `count` 次或全部交易进出额之和。key 为 totalcount 的处理版本，维持序列倒序。

- `BookMaster`：

```cpp
 private:
Trader Trade;
  BlockLinkedList<array<char, 24>> Namebase;
  BlockLinkedList<array<char, 24>> Authorbase;
  BlockLinkedList<array<char, 24>> Keybase;
  BlockLinkedList<BookInfo> ISBNbase;
  vector<array<char, 24>> isbnBusket;
  vector<array<char, 64>> keyBusket;
  vector<BookInfo> infoBusket;
  BookInfo modified;
  BookInfo selected;

 public:
```

- `AccountStack`：

```cpp
 private:
  stack<Account> Accountstack;    // user
  stack<BookInfo> Bookstack;      // their selected book
  stack<Account> Tempstack;     
  stack<BookInfo> tempstack;      //modifySelected
  BookInfo tempinfo;              //old-selected

public:
  void changeSelected(const BookInfo& info);  // 与Login、select同步调用

  void modifySelected(const BookInfo& info);  // 与modify同步调用，将栈中的相关书全部修改

  bool inStack(const string& _uid);
```

管理登录栈与选中书栈。`changeselected` 更换当前选中书目，`modifySelected` 则会完全修改栈中与待修改书本的 ISBN 值相同的书。

- `AccountPiler`：

```cpp
 private:
  AccountStack Stack;
  BookMaster Master;       // 直接管理图书信息
  BlockLinkedList<Account> Accountbase;//账号库
  array<char, 32> temparr;
  Account active;          // current stacktop
  vector<Account> retacc;  // returned account from findAccount

 public:
  friend class Account;
  friend class AccountStack;
  friend class Parser;
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
```

`findAccount` 将查找结果放入 `retacc`，其他的接口功能同标准文档。

- `Parser`：

```cpp
 private:
  AccountPiler Piler;
  vector<string> stringBusket;
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
  void getString(const string& _line);

  // function allocator, also judge the format
  bool funcAllocate();
```

`funcAllocate()` 根据正则规则拆解字符串、解析格式与内容合法性，确定无误后交由以上类执行。

接收其他类的执行成功与否的返回值，确定终止执行（`return false`，使 `main` 函数输出 `Invalid`）或者成功执行（`return true`）。

`Parser.cpp` 中使用了一些全局变量——正则表达式：

```cpp
regex cutReg("[\\s]+");                                       // 以连续空格分割
regex cutEq("^(.+?)=(.+)$");                                  // 以第一个等号进行分割
regex visible("^[[:graph:]]{1,}$");                           // 除不可见字符外ascll字符
regex alnunder("^[0-9a-zA-Z_]{1,}$");                         // 数字，字母，下划线
regex hasNoBrace("^\"[^\"]*\"$");                             // 只有头尾有双引号
regex isInt("^[1-9]{1}\\d{0,9}$");                            // 合法数字
regex isFloat("^[0-9]{1,10}+\\.[0-9]{1,2}$|0|0\\.0|0\\.00");  // 合法float
```

## 文件存储说明

在二进制文件中建立块状链表，通过获取每一块信息 `Block` 的头部指针 `int position` ，以及前一块信息、后一块信息的头部指针 `int before, int after` ，实现类似双端链表的块访问能力；每一块则是包含固定数目的 `Node ` （key-value 节点）按照key值字典序排序，使用`BlockInfo` 存储每一块的最大key、最小key、前指针、当前指针、后指针。这样，可以通过遍历 `BlockInfo` 获得块的信息，而不用完整地读入块。

插入、删除时，外部需要提供已处理好的节点 `Node<[infotype]> tempN`；查找时，需要提供 `const char _key[], vector<T> &basket`，将所有符合条件的信息输入 `basket` 中并返回。

之所以用 `char[]` 做key，是为方便满足存储的定长需求。使用`ArrayToChar`可以转化，但我也在思考“不如全部用 `array<char,64>`”会更方便（有内置 `operator`，比 `strcmp` 更方便安全）。
