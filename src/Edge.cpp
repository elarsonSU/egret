/*  Edge.cpp: an edge in an NFA and a path 

    Copyright (C) 2016  Eric Larson and Anna Kirk
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
#include <set>
#include <string>
#include <vector>
#include "Edge.h"
#include "TestString.h"
using namespace std;

bool
Edge::process_edge(TestString test_string, TestString base_substring)
{
  // loops need to be processed for each path
  if (type == BEGIN_LOOP_EDGE) {
    regex_loop->set_curr_prefix(test_string);
  }
  if (type == END_LOOP_EDGE) {
    regex_loop->set_curr_substring(test_string);
  }

  // no further work needed if edge already processed
  if (processed) return false;
  processed = true;

  // set the path prefix and substring for nodes that need it
  // and return true for evil edges
  switch (type) {
    case CHAR_SET_EDGE:
      char_set->set_prefix(test_string);
      return true;
    case STRING_EDGE:
      regex_str->set_prefix(test_string);
      regex_str->set_substring(base_substring);
      return true;
    case BEGIN_LOOP_EDGE:
      regex_loop->set_prefix(test_string);
      return false;
    case END_LOOP_EDGE:
      regex_loop->set_substring_from_curr();
      return true;
    default:
      return false;
  }
}

TestString
Edge::get_substring()
{
  TestString s;
  
  switch (type) {
    case CHARACTER_EDGE:
      s.append(character);
      return s;
    case CHAR_SET_EDGE:
      s.append(char_set->get_valid_character());
      return s;
    case STRING_EDGE:
      return regex_str->get_substring();
    case END_LOOP_EDGE:
      return regex_loop->get_substring();
    case BEGIN_GROUP_EDGE:
      s.append_begin_group(group);
      return s;
    case END_GROUP_EDGE:
      s.append_end_group(group);
      return s;
    case BACKREFERENCE_EDGE:
      s.append_backreference(group, id);
      return s;
    default:
      return s;
  }
}

void
Edge::gen_min_iter_string(TestString &min_iter_string)
{
  switch (type) {
    case STRING_EDGE:
      regex_str->gen_min_iter_string(min_iter_string);
      break;
    case BEGIN_LOOP_EDGE:
      regex_loop->set_curr_prefix(min_iter_string);
      break;
    case END_LOOP_EDGE:
      regex_loop->gen_min_iter_string(min_iter_string);
      break;
    default:
      min_iter_string.append(get_substring());
      break;
  }
}

vector <TestString>
Edge::gen_evil_strings(TestString path_string, const set <char> &punct_marks)
{
  switch (type) {
    case CHAR_SET_EDGE:
      return char_set->gen_evil_strings(path_string, punct_marks);
    case STRING_EDGE:
      return regex_str->gen_evil_strings(path_string, punct_marks);
    case END_LOOP_EDGE:
      return regex_loop->gen_evil_strings(path_string);
    default:
    {
      vector <TestString> empty;
      return empty;
    }
  }
}

void
Edge::print()
{
  switch (type) {
    case CHARACTER_EDGE:
      cout << "CHARACTER " << character << endl;
      break;
    case CHAR_SET_EDGE:
      cout << "CHAR_SET ";
      char_set->print();
      cout << endl;
      break;
    case STRING_EDGE:
      cout << "STRING ";
      regex_str->print();
      cout << endl;
      break;
    case BEGIN_LOOP_EDGE:
      cout << "BEGIN_LOOP ";
      regex_loop->print();
      cout << endl;
      break;
    case END_LOOP_EDGE:
      cout << "END_LOOP ";
      regex_loop->print();
      cout << endl;
      break;
    case CARET_EDGE:
      cout << "CARET" << endl;
      break;
    case DOLLAR_EDGE:
      cout << "DOLLAR" << endl;
      break;
    case EPSILON_EDGE:
      cout << "EPSILON" << endl;
      break;
    case BACKREFERENCE_EDGE:
      cout << "BACKREFERENCE id:" << id << " " << group_name << " " << group << endl;
      break;
    case BEGIN_GROUP_EDGE:
      cout << "BEGIN GROUP " << group << endl;
      break;
    case END_GROUP_EDGE:
      cout << "END GROUP " << group << endl;
      break;
  } 
}
