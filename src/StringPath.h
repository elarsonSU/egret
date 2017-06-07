#ifndef STRING_PATH_H
#define STRING_PATH_H

#include <string>
#include <vector>
using namespace std;
// types are : CHAR - character, BG - begin group, EG - end group, BR -backreference
typedef enum {
  CHAR,
  BG,
  EG,
  BR
} ItemType;

struct StringPathItem
{
  ItemType type;
  char item;
  int num;
  int id;
};

struct spcompare {
    template<typename T>
    bool operator()(T const& left, T const& right) { return true; }
};

class StringPath {
 public:
  StringPath(){}

  StringPath path_from_string(string s);
  string get_string();
  vector<string> gen_evil_backreference_strings(vector <int> &backrefs_done);
  void add_string(string s);
  void add_char(char c);
  void add_path(StringPath path2);
  void add_path_item(StringPathItem);
  void add_backreference(int _num, int _id);
  void add_begin_group(int _num);
  void add_end_group(int _num);
  void clear() { path.clear(); }

  vector <StringPathItem> path;

 //private:
  
};

#endif // STRING_PATH_H
