#ifndef _ROVER_HPP
#define _ROVER_HPP

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
using std::cout;
using std::fstream;
using std::istream;
using std::ostream;
using std::string;
using std::vector;

// This file works as a file manager using blocklinklist
// all the data storaging and reading operations use interfaces here
//  关键在于如何找到那个文件指针

const int blocksize = 318;

template <class T>
class BlockLinkedList;

template <class T>
class BlockInfo;

// 只会存一个块
template <class T>
class Block;

template <class T>
class Node;

template <class T>
class FileRover : public fstream {
 public:
  // 取得一个块，文件指针保持为pointer
  void getBlock(int pointer, Block<T>& block) {
    seekg(pointer);
    read(reinterpret_cast<char*>(&block), sizeof(block));
  }

  void getInfo(int pointer, BlockInfo<T>& info) {
    seekg(pointer);
    read(reinterpret_cast<char*>(&info), sizeof(info));
  }

  FileRover& operator<<(const Block<T>& block) {
    if (block.Info.position == 0)
      return *this;              // 如果是未初始化的，或者空的块，什么也不做
    seekp(block.Info.position);  // streampos是Long类型，标志到文件开头的字节数
    write(reinterpret_cast<const char*>(&block), sizeof(block));
    return *this;
  }  // 写入块
};

template <class T>
class Node {
 private:
  char key[65];
  T info;

 public:
  Node()
      : key(""), info(T()) {}
  Node(const char key_[], const T info_)
      : info(info_) { strcpy(key, key_); }
  ~Node() = default;

  // need that T has defined operator=
  void reNode(const char key_[], const T info_) {
    strcpy(key, key_);
    info = info_;
  }

  Node& operator=(const Node<T>& rhs) {
    if (this == &rhs)
      return *this;
    strcpy(key, rhs.key);
    info = rhs.info;
    return *this;
  }
  template <class Te>
  friend bool operator==(const Node<Te>&, const Node<Te>&);
  template <class Te>
  friend bool operator!=(const Node<Te>&, const Node<Te>&);
  template <class Te>
  friend bool operator<(const Node<Te>&, const Node<Te>&);
  template <class Te>
  friend bool operator<=(const Node<Te>&, const Node<Te>&);
  template <class Te>
  friend bool operator>(const Node<Te>&, const Node<Te>&);
  template <class Te>
  friend bool operator>=(const Node<Te>&, const Node<Te>&);

  template <class Te>
  friend ostream& operator<<(ostream&, const Node<Te>&);
  friend class Block<T>;
  friend class BlockInfo<T>;
  friend class BlockLinkedList<T>;
  friend class FileRover<T>;
};

// 友元函数定义（模仿int2048）
// 不存在双相同的index-value

template <class T>
bool operator==(const Node<T>& lhs, const Node<T>& rhs) {
  return !strcmp(lhs.key, rhs.key) && lhs.info == rhs.info;
}

template <class T>
bool operator!=(const Node<T>& lhs, const Node<T>& rhs) {
  return !(lhs == rhs);
}
// strcmp,'a'<'b',return negative number
template <class T>
bool operator<(const Node<T>& lhs, const Node<T>& rhs) {
  return strcmp(lhs.key, rhs.key) ? strcmp(lhs.key, rhs.key) < 0 : lhs.info < rhs.info;
}

template <class T>
bool operator>(const Node<T>& lhs, const Node<T>& rhs) {
  return rhs < lhs;
}

template <class T>
bool operator<=(const Node<T>& lhs, const Node<T>& rhs) {
  return lhs < rhs || lhs == rhs;
}

template <class T>
bool operator>=(const Node<T>& lhs, const Node<T>& rhs) {
  return rhs <= lhs;
}

template <class T>
ostream& operator<<(ostream& os, const Node<T>& node) {
  os << "key = " << node.key << "\ninfo = " << node.info << '\n';
  return os;
}

template <class T>
class BlockInfo {
 public:
  Node<T> maxNode, minNode;
  int position = 0;
  int before = 0, after = 0;  // 用于定位文件指针+用于模拟链表的块
  static constexpr int maxSize = blocksize << 1;
  static constexpr int minSize = blocksize;

  friend class Block<T>;
  friend class BlockLinkedList<T>;
  friend class Node<T>;
  friend class FileRover<T>;
};

template <class T>
class Block {
 private:
  // 基本信息
  BlockInfo<T> Info;
  // 存值相关
  Node<T> nodes[blocksize * 3 + 1];  // 防止爆炸
  int nodeLen = 0;                   // how many nodes are there?

 public:
  Block() = default;
  ~Block() = default;
  friend class BlockInfo<T>;
  friend class Node<T>;
  friend class BlockLinkedList<T>;
  friend class FileRover<T>;
  template <class Te>
  friend ostream& operator<<(ostream&, const Block<Te>&);

  void updateM() {
    Info.minNode = nodes[0];
    Info.maxNode = nodes[nodeLen - 1];
  }

  void insertNode(const Node<T>& inNode) {
    int posone = std::lower_bound(nodes, nodes + nodeLen, inNode) - nodes;
    if (nodes[posone] == inNode)
      return;  // 相同的就不添加
    for (int i = nodeLen - 1; i >= posone; i--)
      nodes[i + 1] = nodes[i];
    nodes[posone] = inNode;
    nodeLen++;
    updateM();
  }

  void deleteNode(int pos) {
    for (int i = pos + 1; i < nodeLen; i++)
      nodes[i - 1] = nodes[i];
    nodes[nodeLen - 1] = Node<T>();
    nodeLen--;
    updateM();
  }

  bool inBlock(const char index[]) {
    return strcmp(index, Info.minNode.key) >= 0 && strcmp(index, Info.maxNode.key) <= 0;
  }
  bool inBlock(const Node<T>& block) {
    return block >= Info.minNode && block <= Info.maxNode;
  }
};

template <class T>  // 都是用来调试的，上一个也可以删掉
ostream& operator<<(ostream& os, const Block<T>& block) {
  os << "size=" << block.nodeLen << " before=" << block.Info.before
     << " after=" << block.Info.after << " pos=" << block.Info.position << '\n';
  for (int i = 0; i < block.nodeLen; i++)
    os << block.nodes[i];
  os << '\n';
  return os;
}

template <class T>
class BlockLinkedList {
 private:
  FileRover<T> file;
  string fileName;
  int blockLen = 0;  // 块的总数
  Block<T> Tempin, Tempout;
  Node<T> tnode;
  int reserved[blocksize];  // 用来记录哪些位置是空的，可以在那里插入
  int resptr = -1;
  int header = (blocksize + 2) * sizeof(int);

 public:
  friend class Node<T>;
  friend class Block<T>;
  friend class BlockInfo<T>;
  friend class FileRover<T>;
  BlockLinkedList() = default;
  BlockLinkedList(const string& fileName_) {
    fileName = fileName_;
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    bool isopen = file.is_open();
    file.close();
    if (!isopen) {
      file.open(fileName, std::ios::out);  // 新建
      file.close();
    }
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if (isopen) {  // 有这个文件
      file.seekg(0);
      file.read(reinterpret_cast<char*>(&blockLen), sizeof(int));
      file.read(reinterpret_cast<char*>(&resptr), sizeof(int));
      for (int i = 0; i < blocksize; i++)
        file.read(reinterpret_cast<char*>(&(reserved[i])), sizeof(int));
    } else {  // 头部存放块的总数
      file.seekp(0);
      file.write(reinterpret_cast<char*>(&blockLen), sizeof(int));
      file.write(reinterpret_cast<char*>(&resptr), sizeof(int));
      memset(reserved, -1, sizeof(reserved));
      for (int i = 0; i < blocksize; i++)
        file.write(reinterpret_cast<char*>(&(reserved[i])), sizeof(int));
      Block<T> emptyBlock;
      emptyBlock.Info.position = header;
      file << emptyBlock;
    }
    // 更新blockLen
  }
  ~BlockLinkedList() {
    file.seekp(0);
    file.write(reinterpret_cast<char*>(&blockLen), sizeof(int));
    file.write(reinterpret_cast<char*>(&resptr), sizeof(int));
    for (int i = 0; i < blocksize; i++)
      file.write(reinterpret_cast<char*>(&(reserved[i])), sizeof(int));
    file.close();
  }

  void setList(const string& fileName_) {
    fileName = fileName_;
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    bool isopen = file.is_open();
    file.close();
    if (!isopen) {
      file.open(fileName, std::ios::out);  // 新建
      file.close();
    }
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if (isopen) {  // 有这个文件
      file.seekg(0);
      file.read(reinterpret_cast<char*>(&blockLen), sizeof(int));
      file.read(reinterpret_cast<char*>(&resptr), sizeof(int));
      for (int i = 0; i < blocksize; i++)
        file.read(reinterpret_cast<char*>(&(reserved[i])), sizeof(int));
    } else {  // 头部存放块的总数
      file.seekp(0);
      file.write(reinterpret_cast<char*>(&blockLen), sizeof(int));
      file.write(reinterpret_cast<char*>(&resptr), sizeof(int));
      memset(reserved, -1, sizeof(reserved));
      for (int i = 0; i < blocksize; i++)
        file.write(reinterpret_cast<char*>(&(reserved[i])), sizeof(int));
      Block<T> emptyBlock;
      emptyBlock.Info.position = header;
      file << emptyBlock;
    }
  }

  // 块状链表正式开始！
  int newBlock(Block<T>& ref) {  // 在块后端插入空链表，返回值是插入位置
    Block<T> then;
    int curpos;
    if (resptr == -1)
      curpos = blockLen * sizeof(ref) + header;  // 定位到文件末端
    else {
      curpos = reserved[resptr];  // 定位到可能的空位
      reserved[resptr] = -1;
      resptr--;
    }
    // 链表一般插入
    then.Info.before = ref.Info.position;
    then.Info.position = curpos;
    then.Info.after = ref.Info.after;
    // 如果插入的链表有后继，那么更新原后继
    if (ref.Info.after != 0) {
      file.getBlock(ref.Info.after, Tempin);
      Tempin.Info.before = curpos;
      file << Tempin;
    }
    ref.Info.after = curpos;
    // 回到插入位置
    file << ref;  // 覆盖之
    file << then;
    blockLen++;
    return curpos;
    // 更新ref
  }  // 返回值正是新的链表末端值

  // 分裂这个块
  void Split(Block<T>& lhs) {
    int pointer = newBlock(lhs);
    file.getBlock(pointer, Tempin);  // 取得空块
    int siz = lhs.nodeLen >> 1;
    for (int i = siz; i < lhs.nodeLen; i++)
      Tempin.nodes[i - siz] = lhs.nodes[i];
    Tempin.nodeLen = lhs.nodeLen - siz;
    lhs.nodeLen = siz;
    Tempin.updateM();
    lhs.updateM();
    file << Tempin;
    file << lhs;  // 写回去
  }

  void Merge(Block<T>& lhs) {  // 吞并后面的块
    if (lhs.Info.after == 0) {
      file << lhs;
      return;  // 后面没有块，就不做
    }
    file.getBlock(lhs.Info.after, Tempin);
    // 在外存中删除Tempin
    resptr++;
    reserved[resptr] = Tempin.Info.position;

    for (int i = 0, j = lhs.nodeLen; i < Tempin.nodeLen; i++, j++)
      lhs.nodes[j] = Tempin.nodes[i];
    if (Tempin.Info.after != 0) {
      Block<T> anotherTempin;
      file.getBlock(Tempin.Info.after, anotherTempin);
      anotherTempin.Info.before = lhs.Info.position;
      file << anotherTempin;
    }
    lhs.Info.after = Tempin.Info.after;
    lhs.nodeLen += Tempin.nodeLen;
    lhs.updateM();
    blockLen--;
    if (lhs.nodeLen > lhs.Info.maxSize) {
      Split(lhs);
      return;
    }
    file << lhs;
  }

  void Insert(const Node<T>& value) {
    int curlen;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&curlen), sizeof(int));
    if (curlen == 0) {      // 文件为空，类似于初始化
      Block<T> firstBlock;  // 创建块
      firstBlock.insertNode(value);
      firstBlock.Info.before = 0;
      firstBlock.Info.after = 0;
      firstBlock.Info.position = header;  // 第一位置
      file << firstBlock;
      blockLen = 1;
      file.seekp(0);
      file.write(reinterpret_cast<char*>(&blockLen), sizeof(int));
      return;
    }
    // 此时curlen转义为current position
    curlen = header;
    // 第一个块特判
    file.getInfo(curlen, Tempout.Info);
    if (value < Tempout.Info.minNode) {
      file.getBlock(curlen, Tempout);
      Tempout.insertNode(value);
      if (Tempout.nodeLen > Tempout.Info.maxSize)
        Split(Tempout);
      else
        file << Tempout;
      return;
    }
    while (curlen) {
      file.getInfo(curlen, Tempout.Info);
      if (Tempout.Info.after == 0 || Tempout.inBlock(value)      // 到最后一个块，或者到合法的块
          || (tnode < value && Tempout.Info.minNode > value)) {  // 或者处于前夹缝
        file.getBlock(curlen, Tempout);
        Tempout.insertNode(value);
        if (Tempout.nodeLen > Tempout.Info.maxSize)
          Split(Tempout);
        else
          file << Tempout;
        return;
      }
      tnode = Tempout.Info.maxNode;
      curlen = Tempout.Info.after;
    }
  }

  void Delete(const Node<T>& value) {
    int curlen = 0;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&curlen), sizeof(int));
    if (curlen == 0)
      return;
    curlen = header;
    while (curlen) {
      file.getInfo(curlen, Tempout.Info);
      if (Tempout.inBlock(value)) {
        file.getBlock(curlen, Tempout);
        int pos = std::lower_bound(Tempout.nodes, Tempout.nodes + Tempout.nodeLen, value) - Tempout.nodes;
        if (value == Tempout.nodes[pos]) {
          Tempout.deleteNode(pos);
          if (Tempout.nodeLen < Tempout.Info.minSize) {
            Merge(Tempout);
            return;
          }
        }
        file << Tempout;
        return;
      }
      curlen = Tempout.Info.after;
    }
  }

  // 如果能保证能找到某个值，且修改后键不变，且值的变化不会影响排序，那么可以使用这个函数
  void sameModify(const Node<T>& value) {
    int curlen = 0;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&curlen), sizeof(int));
    if (curlen == 0)
      return;
    curlen = header;
    while (curlen) {
      file.getInfo(curlen, Tempout.Info);
      if (Tempout.inBlock(value)) {
        file.getBlock(curlen, Tempout);
        int pos = std::lower_bound(Tempout.nodes, Tempout.nodes + Tempout.nodeLen, value) - Tempout.nodes;
        Tempout.nodes[pos] = value;
        file << Tempout;
        return;
      }
      curlen = Tempout.Info.after;
    }
  }

  // 向array中覆盖写入所有索引为index的节点的信息
  void Find(const char index[], vector<T>& array) {
    array.clear();
    int curlen = 0;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&curlen), sizeof(int));
    if (curlen == 0)
      return;
    bool isIndex = false;
    curlen = header;
    while (curlen) {
      file.getInfo(curlen, Tempout.Info);
      if (Tempout.inBlock(index)) {
        file.getBlock(curlen, Tempout);
        for (int i = 0; i < Tempout.nodeLen; i++) {
          if (strcmp(index, Tempout.nodes[i].key) == 0) {
            isIndex = true;
            array.push_back(Tempout.nodes[i].info);
          } else if (isIndex)
            return;  // 找到就跑
        }
      }
      curlen = Tempout.Info.after;
    }
  }

  void findAll(vector<T>& array) {
    array.clear();
    int curlen = 0;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&curlen), sizeof(int));
    if (curlen == 0)
      return;
    curlen = header;
    while (curlen) {
      file.getBlock(curlen, Tempout);
      for (int i = 0; i < Tempout.nodeLen; i++)
        array.push_back(Tempout.nodes[i].info);
      curlen = Tempout.Info.after;
    }
    return;
  }

  void findBlock() {
    cout << blockLen << '\n';
    for (int i = 0; i <= resptr; i++) {
      cout << reserved[i] << ' ';
    }
    cout << '\n';
    int curlen = 0;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&curlen), sizeof(int));
    if (curlen == 0)
      return;
    curlen = header;
    while (curlen) {
      file.getBlock(curlen, Tempout);
      cout << Tempout;
      curlen = Tempout.Info.after;
    }
  }

  // 采集前count项
  void findFront(int count, vector<T>& array) {
    array.clear();
    if (!count)
      return;
    int curlen = 0;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&curlen), sizeof(int));
    if (curlen == 0)
      return;
    curlen = header;
    while (curlen) {
      file.getBlock(curlen, Tempout);
      if (array.size() + Tempout.nodeLen < count) {
        for (int i = 0; i < Tempout.nodeLen; i++) {
          array.push_back(Tempout.nodes[i].info);
        }
      } else {
        for (int i = 0; i < Tempout.nodeLen; i++) {
          array.push_back(Tempout.nodes[i].info);
          if (array.size() == count)
            return;
        }
      }
      curlen = Tempout.Info.after;
    }
    return;
  }

  int getLen() {
    vector<T> array;
    findAll(array);
    return array.size();
  }
};

#endif  // !_ROVER_H