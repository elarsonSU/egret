/*  TestString.h: Generated Test String

    Copyright (C) 2016-17  Eric Larson, Anna Kirk, and Tyler Hartje
    elarson@seattleu.edu

    This file is part of EGRET.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TEST_STRING_H
#define TEST_STRING_H

#include <string>
#include <vector>
using namespace std;

typedef enum {
  TEST_STR_CHAR,
  TEST_STR_BEGIN_GROUP,
  TEST_STR_END_GROUP,
  TEST_STR_BACKREFERENCE
} TestStringItemType;

struct TestStringItem
{
  TestStringItemType type;
  char character;	// character in string (for characters)
  int group;		// group number (for groups and backreferences)
  int id;		// unique id for each backreference (for backreferences)
};

class TestString {

public:

  TestString() {}
  void clear() { test_string.clear(); }
  bool empty() { return test_string.empty(); }
  unsigned int size() { return test_string.size(); }

  // append functions
  void append(char c);
  void append(string s);
  void append(TestStringItem item);
  void append(TestString test);
  void append_begin_group(int group);
  void append_end_group(int group);
  void append_backreference(int group, int id);

  // create and return substring
  TestString create_substr(unsigned int start);
  TestString create_substr(unsigned int start, unsigned int end);

  // get actual string corresponding to the test string
  string get_string();

  // generate strings for backreference
  vector <string> gen_evil_backreference_strings(vector <int> &backrefs_done);

private:
  vector <TestStringItem> test_string;
  
};

#endif // TEST_STRING_H
