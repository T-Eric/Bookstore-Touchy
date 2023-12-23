#include "BookMaster.hpp"

BookMaster::BookMaster() {
  Namebase.setList("bookName.dat");
  Authorbase.setList("bookAuthor.dat");
  Keybase.setList("bookKey.dat");
  ISBNbase.setList("bookISBN.dat");
}
BookMaster::~BookMaster() = default;

bool BookMaster::Nan(const array<char, 24>& _isbn) {
  return _isbn[0] == 0;
}

bool BookMaster::Nan(const array<char, 64>& item) {
  return item[0] == 0;
}

char* BookMaster::arrayToChar(const array<char, 24>& _isbn) {
  for (int i = 0; i < _isbn.size(); i++)
    tempisbn[i] = _isbn[i];
  return tempisbn;
}

char* BookMaster::arrayToLongChar(const array<char, 64>& other) {
  for (int i = 0; i < other.size(); i++)
    tempother[i] = other[i];
  return tempother;
}

bool BookMaster::Search(const char item[], OperationType op) {
  switch (op) {
    case ISBN:
      ISBNbase.Find(item, infoBusket);
      if (infoBusket.empty())
        return false;
      return true;
      break;

    case AUTHOR:
      infoBusket.clear();
      Authorbase.Find(item, isbnBusket);
      if (isbnBusket.empty())
        return false;
      for (int i = 0; i < isbnBusket.size(); i++) {
        ISBNbase.Find(arrayToChar(isbnBusket[i]), tempBusket);
        infoBusket.insert(infoBusket.end(), tempBusket.begin(), tempBusket.end());
      }
      // std::sort(infoBusket.begin(), infoBusket.end());
      return true;
      break;

    case KEYWORD:
      infoBusket.clear();
      Keybase.Find(item, isbnBusket);
      if (isbnBusket.empty())
        return false;
      for (int i = 0; i < isbnBusket.size(); i++) {
        ISBNbase.Find(arrayToChar(isbnBusket[i]), tempBusket);
        infoBusket.insert(infoBusket.end(), tempBusket.begin(), tempBusket.end());
      }
      // std::sort(infoBusket.begin(), infoBusket.end());
      return true;
      break;

    case BOOKNAME:
      infoBusket.clear();
      Namebase.Find(item, isbnBusket);
      if (isbnBusket.empty())
        return false;
      for (int i = 0; i < isbnBusket.size(); i++) {
        ISBNbase.Find(arrayToChar(isbnBusket[i]), tempBusket);
        infoBusket.insert(infoBusket.end(), tempBusket.begin(), tempBusket.end());
      }
      // std::sort(infoBusket.begin(), infoBusket.end());
      return true;
      break;

    default:
      ISBNbase.findAll(infoBusket);  // 一定排好了序
      if (infoBusket.empty())
        return false;
      return true;
      break;
  }
}

void BookMaster::Select(const char _isbn[]) {
  if (Search(_isbn, ISBN))
    selected = infoBusket[0];
  else {
    selected.newISBN(_isbn);
    Import(0, 0.0);  // 添加到了isbn库中
    // 不用管其他库，因为信息空白
  }
}
void BookMaster::unSelect() {
  selected = BookInfo();
}

bool BookMaster::cutKey(const array<char, 64>& keys_, vector<array<char, 64>>& vec) {
  if (keys_[0] == 0)
    return false;
  vec.clear();
  array<char, 64> temp;
  temp.fill(0);
  int ptr = 0, sm = 0;
  for (int i = 0; keys_[i] != 0; i++) {
    if (keys_[i] != '|')
      temp[ptr++] = keys_[i];
    else {
      if (temp[0] == 0)
        return false;
      vec.push_back(temp);
      ptr = 0;
      temp.fill(0);
      sm++;
    }
  }
  if (temp[0] == 0)
    return false;
  vec.push_back(temp);
  sm++;
  for (int i = 0; i != sm; i++) {
    for (int j = i + 1; j != sm; j++) {
      if (vec[i] == vec[j])
        return false;
    }
  }  // 查重
  return true;
}

bool BookMaster::Import(int quantity, double totalcost) {
  if (selected.storage == -1)
    return false;
  if (quantity < 0)
    return false;
  tempN.reNode(arrayToChar(selected.isbn), selected);
  ISBNbase.Delete(tempN);
  selected.storage += quantity;
  tempN.reNode(tempisbn, selected);
  ISBNbase.Insert(tempN);
  if (quantity > 0) {
    Trade.Insert(totalcost, 0);
  }
  return true;
}

bool BookMaster::Buy(const char _isbn[], int quantity) {
  if (quantity < 0)
    return false;
  if (Search(_isbn, ISBN)) {
    tempinfo = infoBusket[0];
    if (tempinfo.storage < quantity)
      return false;
    tempinfo.storage -= quantity;
    tempN.reNode(arrayToChar(tempinfo.isbn), tempinfo);
    ISBNbase.sameModify(tempN);  // 只修改了库存，那么就只做一次sameModify
    cout << std::fixed << std::setprecision(2) << tempinfo.price * quantity << '\n';
    Trade.Insert((double)(tempinfo.price * quantity), 1);
    return true;
  }
  return false;
}

bool BookMaster::preModify(const string& _isbn, const string& _name, const string& _author, const string& _keys, double _price) {
  if (selected.storage == -1)
    return false;
  if (_isbn != "")
    modified.revise(_isbn, ISBN);
  if (_name != "")
    modified.revise(_name, BOOKNAME);
  if (_author != "")
    modified.revise(_author, AUTHOR);
  if (_keys != "")
    modified.revise(_keys, KEYWORD);
  if (_price >= 0)
    modified.revise("", PRICE, _price);
  return true;
}

// use bool to mark whether to modify
bool BookMaster::Modify(bool isisbn, bool isname, bool isauthor, bool iskeyword, bool isprice) {
  tempinfo = selected;  // 之前的
  if (!(isisbn || isname || isauthor || iskeyword || isprice))
    return false;
  if (isisbn && selected.isbn == modified.isbn)
    return false;
  if (isisbn)
    selected.isbn = modified.isbn;
  if (isname)
    selected.name = modified.name;
  if (isauthor)
    selected.author = modified.author;
  if (iskeyword)
    selected.key = modified.key;
  if (isprice)
    selected.price = modified.price;
  if (isisbn) {
    tempN.reNode(arrayToChar(tempinfo.isbn), tempinfo);
    ISBNbase.Delete(tempN);
    tempN.reNode(arrayToChar(selected.isbn), selected);
    ISBNbase.Insert(tempN);  //
    if (!Nan(tempinfo.name)) {
      tempC.reNode(arrayToLongChar(tempinfo.name), tempinfo.isbn);
      Namebase.Delete(tempC);
    }
    if (!Nan(selected.name)) {
      tempC.reNode(arrayToLongChar(selected.name), selected.isbn);
      Namebase.Insert(tempC);
    }
    //
    if (!Nan(tempinfo.author)) {
      tempC.reNode(arrayToLongChar(tempinfo.author), tempinfo.isbn);
      Authorbase.Delete(tempC);
    }
    if (!Nan(selected.author)) {
      tempC.reNode(arrayToLongChar(selected.author), selected.isbn);
      Authorbase.Insert(tempC);
    }
    // 注意，这里需要切割关键词并分别处理
    if (!Nan(tempinfo.key)) {
      if (cutKey(tempinfo.key, keyBusket)) {
        for (int i = 0; i < keyBusket.size(); i++) {
          tempC.reNode(arrayToLongChar(keyBusket[i]), tempinfo.isbn);
          Keybase.Delete(tempC);
        }
      }
    }
    if (!Nan(selected.key)) {
      if (cutKey(selected.key, keyBusket)) {
        for (int i = 0; i < keyBusket.size(); i++) {
          tempC.reNode(arrayToLongChar(keyBusket[i]), selected.isbn);
          Keybase.Insert(tempC);
        }
      }
    }
  } else {
    tempN.reNode(arrayToChar(selected.isbn), selected);
    ISBNbase.sameModify(tempN);  // 无论如何，修改信息
    if (isname) {
      if (!Nan(tempinfo.name)) {
        tempC.reNode(arrayToLongChar(tempinfo.name), tempinfo.isbn);
        Namebase.Delete(tempC);
      }
      tempC.reNode(arrayToLongChar(selected.name), selected.isbn);
      Namebase.Insert(tempC);
    }
    if (isauthor) {
      if (!Nan(tempinfo.author)) {
        tempC.reNode(arrayToLongChar(tempinfo.author), tempinfo.isbn);
        Authorbase.Delete(tempC);
      }
      tempC.reNode(arrayToLongChar(selected.author), selected.isbn);
      Authorbase.Insert(tempC);
    }
    if (iskeyword) {
      if (!Nan(tempinfo.key)) {
        if (cutKey(tempinfo.key, keyBusket)) {
          for (int i = 0; i < keyBusket.size(); i++) {
            tempC.reNode(arrayToLongChar(keyBusket[i]), tempinfo.isbn);
            Keybase.Delete(tempC);
          }
        }
      }
      if (cutKey(selected.key, keyBusket)) {
        for (int i = 0; i < keyBusket.size(); i++) {
          tempC.reNode(arrayToLongChar(keyBusket[i]), selected.isbn);
          Keybase.Insert(tempC);
        }
      }
    }
  }
  modified.newISBN("\0");
  return true;
}

// outer interface, show current books in the infoBUsket
void BookMaster::show() {
  if (infoBusket.empty()) {
    puts("");
    return;
  }
  std::sort(infoBusket.begin(), infoBusket.end());                                      // 升序
  int endpos = std::unique(infoBusket.begin(), infoBusket.end()) - infoBusket.begin();  // 去重
  for (int i = 0; i < endpos; i++) {
    cout << infoBusket[i] << '\n';
  }
  infoBusket.clear();
}