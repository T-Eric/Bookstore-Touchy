#ifndef MASTER_HPP
#define MASTER_HPP

#include <array>
#include <cstring>
#include <iomanip>
#include "FileRover.hpp"
#include "Trader.hpp"
using std::array;

enum OperationType {
  ISBN,
  AUTHOR,
  KEYWORD,
  BOOKNAME,
  PRICE
};

/**
 * @brief we must bring this to replace char[]
 */
struct BookInfo {
  array<char, 24> isbn;
  array<char, 64> author;
  array<char, 64> name;
  array<char, 64> key;
  long long storage = -1;
  double price = 0;
  BookInfo() {
    isbn.fill(0);
    author.fill(0);
    name.fill(0);
    key.fill(0);
  };
  BookInfo(const char isbn_[], const char author_[],  //
           const char name_[],
           const char key_[],  //
           int storage_,
           double price_) {
    isbn.fill(0);
    author.fill(0);
    name.fill(0);
    key.fill(0);
    for (int i = 0; isbn_[i] != 0; i++)
      isbn[i] = isbn_[i];
    for (int i = 0; author_[i] != 0; i++)
      author[i] = author_[i];
    for (int i = 0; name_[i] != 0; i++)
      name[i] = name_[i];
    for (int i = 0; key_[i] != 0; i++)
      key[i] = key_[i];
    storage = storage_;
    price = price_;
  }
  BookInfo(const BookInfo& rhs) {
    isbn = rhs.isbn;
    author = rhs.author;
    name = rhs.name;
    key = rhs.key;
    storage = rhs.storage;
    price = rhs.price;
  }

  void newISBN(const char isbn_[]) {
    isbn.fill(0);
    author.fill(0);
    name.fill(0);
    key.fill(0);
    for (int i = 0; isbn_[i] != 0; i++)
      isbn[i] = isbn_[i];
    storage = 0;
    price = 0;
  }

  void revise(const string& in, OperationType op, double fl = -1.0) {
    switch (op) {
      case ISBN:
        isbn.fill(0);
        for (int i = 0; i != in.size(); i++)
          isbn[i] = in[i];
        break;
      case BOOKNAME:
        name.fill(0);
        for (int i = 0; i != in.size(); i++)
          name[i] = in[i];
        break;
      case AUTHOR:
        author.fill(0);
        for (int i = 0; i != in.size(); i++)
          author[i] = in[i];
        break;
      case KEYWORD:
        key.fill(0);
        for (int i = 0; i != in.size(); i++)
          key[i] = in[i];
        break;
      case PRICE:
        price = fl;
      default:
        break;
    }
  }

  BookInfo& operator=(const BookInfo& rhs) {
    isbn = rhs.isbn;
    author = rhs.author;
    name = rhs.name;
    key = rhs.key;
    storage = rhs.storage;
    price = rhs.price;
    return *this;
  }

  friend bool operator<(const BookInfo& lhs, const BookInfo& rhs) {
    return lhs.isbn < rhs.isbn;
  }

  friend bool operator==(const BookInfo& lhs, const BookInfo& rhs) {
    return lhs.isbn == rhs.isbn;
  }

  friend ostream& operator<<(ostream& os, const BookInfo& info) {
    string out;
    for (const char& it : info.isbn) {
      if (it == 0)
        break;
      out += it;
    }
    os << out << '\t';
    out.clear();
    for (const char& it : info.name) {
      if (it == 0)
        break;
      out += it;
    }
    os << out << '\t';
    out.clear();
    for (const char& it : info.author) {
      if (it == 0)
        break;
      out += it;
    }
    os << out << '\t';
    out.clear();
    for (const char& it : info.key) {
      if (it == 0)
        break;
      out += it;
    }
    os << out << '\t' << std::fixed << std::setprecision(2) << info.price;
    os << '\t' << info.storage;
    return os;
  }
};

class BookMaster {
 private:
  Trader Trade;
  BlockLinkedList<array<char, 24>> Namebase;
  BlockLinkedList<array<char, 24>> Authorbase;
  BlockLinkedList<array<char, 24>> Keybase;
  BlockLinkedList<BookInfo> ISBNbase;
  vector<array<char, 24>> isbnBusket;
  vector<array<char, 64>> keyBusket;
  vector<BookInfo> infoBusket;
  vector<BookInfo> tempBusket;
  BookInfo selected;
  BookInfo tempinfo;
  BookInfo modified;
  Node<BookInfo> tempN;
  Node<array<char, 24>> tempC;
  char tempisbn[24];
  char tempother[64];

 public:
  friend class AccountPiler;
  friend class AccountStack;
  friend class Parser;
  BookMaster();
  ~BookMaster();

  bool Nan(const array<char, 24>& _isbn);
  bool Nan(const array<char, 64>& item);

  char* arrayToChar(const array<char, 24>& _isbn);
  char* arrayToLongChar(const array<char, 64>& _item);

  bool Search(const char item[] = "", OperationType op = PRICE);

  void Select(const char _isbn[]);
  void unSelect();
  // void reSelect(const char _isbn[]);

  // keys包括重复段就操作失败，包含空段也操作失败
  bool cutKey(const array<char, 64>& keys_, vector<array<char, 64>>& vec);

  bool Import(int quantity, double totalcost);

  bool Buy(const char _isbn[], int quantity);

  // use bool to mark whether to modify
  bool preModify(const string& _isbn = "", const string& _name = "", const string& _author = "", const string& _keys = "", double _price = -1.0);
  bool Modify(bool isisbn = false, bool isname = false, bool isauthor = false, bool iskeyword = false, bool isprice = false);

  // outer interface
  void show();
};

#endif  // ! _MASTER_HPP
