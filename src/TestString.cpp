/*  TestString.cpp: Generated Test String

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

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include "TestString.h"
using namespace std;

void
TestString::append(char c)
{
  TestStringItem item;
  item.character = c;
  item.type = TEST_STR_CHAR;
  test_string.push_back(item);
}

void
TestString::append(string s)
{
  TestStringItem item;
  for (unsigned int i = 0; i < s.length(); i++) {
    item.character = s[i];
    item.type = TEST_STR_CHAR;
    test_string.push_back(item);
  }
}

void
TestString::append(TestStringItem item)
{
  test_string.push_back(item);
}

void
TestString::append(TestString test)
{
  test_string.insert(test_string.end(), test.test_string.begin(), test.test_string.end());
}

void
TestString::append_begin_group(int group)
{
  TestStringItem item;
  item.type = TEST_STR_BEGIN_GROUP;
  item.group = group;
  test_string.push_back(item);
}

void
TestString::append_end_group(int group)
{
  TestStringItem item;
  item.type = TEST_STR_END_GROUP;
  item.group = group;
  test_string.push_back(item);
}

void
TestString::append_backreference(int group, int id)
{
  TestStringItem item;
  item.type = TEST_STR_BACKREFERENCE;
  item.group = group;
  item.id = id;
  test_string.push_back(item);
}

TestString
TestString::create_substr(unsigned int start)
{
  TestString s;
  for (unsigned int i = start; i < size(); i++) {
    s.append(test_string[i]);
  }
  return s;
}

TestString
TestString::create_substr(unsigned int start, unsigned int end)
{
  TestString s;
  for (unsigned int i = start; i < end; i++) {
    s.append(test_string[i]);
  }
  return s;
}

string
TestString::get_string()
{
  string actual_str = "";

  // list of group strings and of local backreferences
  vector <int> groups;
  vector <int>::iterator git;
  unordered_map <int, string> group_strings {};  

  vector <TestStringItem>::iterator it;
  for (it = test_string.begin(); it != test_string.end(); it++) {
    switch (it->type) {
      case TEST_STR_CHAR:
        actual_str += it->character;
        for (git = groups.begin(); git != groups.end(); git++) {
          group_strings[*git] += it->character;
        }
        break;
      case TEST_STR_BEGIN_GROUP:
        groups.push_back(it->group);
        break;
      case TEST_STR_END_GROUP:
        assert(!groups.empty());
        groups.pop_back();
	break;
      case TEST_STR_BACKREFERENCE:
        string backref = group_strings[it->group];
        actual_str += backref;
        for (git = groups.begin(); git != groups.end(); git++) {
	  group_strings[*git] += group_strings[it->group];
        }
	break;
    }
  }
  return actual_str;
}

vector <string>
TestString::gen_evil_backreference_strings(vector <int> &backrefs_done)
{
  vector <int> backrefs; 	// backreferences to process
  vector <string> evil_strings; // set of evil strings
  
  // list of group strings and of local backreferences
  vector <int> groups;
  vector <int>::iterator git;
  unordered_map <int, string> group_strings {};
  
  vector <TestStringItem>::iterator it;
  for (it = test_string.begin(); it != test_string.end(); it++) {
    switch (it->type) {
      case TEST_STR_CHAR:
        for (git = groups.begin(); git != groups.end(); git++) {
	  group_strings[*git] += it->character;
        }
        break;
      case TEST_STR_BEGIN_GROUP:
        groups.push_back(it->group);
        break;
      case TEST_STR_END_GROUP:
        assert(!groups.empty());
        groups.pop_back();
	break;
      case TEST_STR_BACKREFERENCE:
        if (!(find(backrefs_done.begin(), backrefs_done.end(), it->id)
	      != backrefs_done.end())) {
	  backrefs.push_back(it->id);
	  backrefs_done.push_back(it->id);
        }
        for (git = groups.begin(); git != groups.end(); git++) {
	  group_strings[*git] += group_strings[it->group];
        }
	break;
    }
  }

  // create evil strings for each backreference
  vector <int>::iterator bit;
  for (bit = backrefs.begin(); bit != backrefs.end(); bit++) {

    string evil_add = "";
    string evil_remove = "";
    string evil_modify = "";

    for (it = test_string.begin(); it != test_string.end(); it++) {
      if (it->type == TEST_STR_CHAR) {
	evil_add += it->character;
	evil_remove += it->character;
	evil_modify += it->character;
      }
      else if (it->type == TEST_STR_BACKREFERENCE) {
	string temp;

	// if current backreference, generate evil strings
	if (it->id == *bit) {

	  // evil_add
	  temp = group_strings[it->group];
	  temp.push_back(temp.back());
	  evil_add += temp;

	  // evil_remove
	  temp = group_strings[it->group];
	  if(temp.length() > 0) {
	    temp.pop_back();
	  }
	  evil_remove += temp;

	  // evil_modify
	  temp = group_strings[it->group];
	  int edit = temp.length()/2;
	  char c = temp[edit];
	  c += 1;
	  temp[edit] = c;
	  evil_modify += temp;
	}

	// not current backreference --> normal backref substitution
	else {
	  temp = group_strings[it->group];
	  evil_add += temp;
	  evil_remove += temp;
	  evil_modify += temp;
	}
      }
    }
    evil_strings.push_back(evil_add);
    evil_strings.push_back(evil_remove);
    evil_strings.push_back(evil_modify);
  }
  
  return evil_strings;
}
