/*  TestGenerator.cpp: Generates paths and strings

    Copyright (C) 2016  Eric Larson and Anna Kirk
    elarson@seattleu.edu

    Some code in this file was derived from a RE->NFA converter
    developed by Eli Bendersky.

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

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>
#include "NFA.h"
#include "TestGenerator.h"
#include "Path.h"
#include "error.h"
using namespace std;

vector <string>
TestGenerator::gen_test_strings()
{
  paths = nfa.find_basis_paths();
  gen_initial_strings();
  gen_evil_strings();
  return test_strings;
}

void
TestGenerator::gen_initial_strings()
{
  bool all_start_with_caret = false;
  bool all_end_with_dollar = false;
  bool warn_anchor_middle = false;
  bool warn_caret_start = false;
  bool warn_dollar_end = false;

  vector <Path>::iterator path_iter;
  string first_string = "";
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {

    // check for leading carets and trailing dollars
    bool start_with_caret = path_iter->has_leading_caret();
    bool end_with_dollar = path_iter->has_trailing_dollar();

    // go through each state in the path
    string path_string = path_iter->gen_initial_string(base_substring);
    add_to_test_strings(path_string);

    // for first path, record whether the path starts with ^ and/or ends with $
    if (first_string == "") {
      all_start_with_caret = start_with_caret;
      all_end_with_dollar = end_with_dollar;
      first_string = path_string;
    }

    // check for anchors in the middle
    string anchor_err = path_iter->check_anchor_middle();
	  
    // process anchor warnings
    if (!warn_anchor_middle && anchor_err != "") {
      addWarning(anchor_err);
      warn_anchor_middle = true;
    }
    if (!warn_caret_start) {
      if (all_start_with_caret && !start_with_caret) {
        stringstream s;
        s << "Some but not all strings start with a ^ anchor\n";
        s << "...String with ^ anchor:    " << first_string << "\n";
        s << "...String with no ^ anchor: " << path_string;
        addWarning(s.str());
        warn_caret_start = true;
      }
      if (!all_start_with_caret && start_with_caret) {
        stringstream s;
        s << "Some but not all strings start with a ^ anchor\n";
        s << "...String with ^ anchor:    " << path_string << "\n";
        s << "...String with no ^ anchor: " << first_string;
        addWarning(s.str());
        warn_caret_start = true;
      }
    }
    if (!warn_dollar_end) {
      if (all_end_with_dollar && !end_with_dollar) {
        stringstream s;
        s << "Some but not all strings end with a $ anchor\n";
        s << "...String with $ anchor:    " << first_string << "\n";
        s << "...String with no $ anchor: " << path_string;
        addWarning(s.str());
        warn_dollar_end = true;
      }
      if (!all_end_with_dollar && end_with_dollar) {
        stringstream s;
        s << "Some but not all strings end with a $ anchor\n";
        s << "...String with $ anchor:    " << path_string << "\n";
        s << "...String with no $ anchor: " << first_string;
        addWarning(s.str());
        warn_dollar_end = true;
      }
    }
  }
}

void
TestGenerator::add_to_test_strings(string s)
{
  if (find(test_strings.begin(), test_strings.end(), s) == test_strings.end()) {
    test_strings.push_back(s);
  }
}

void
TestGenerator::add_to_test_strings(set <string> strs)
{
  set <string>::iterator it;
  for (it = strs.begin(); it != strs.end(); it++)
    add_to_test_strings(*it);
}

void
TestGenerator::gen_evil_strings()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    set <string> evil_strings = path_iter->gen_evil_strings(punct_marks);
    add_to_test_strings(evil_strings);
  }
}

void
TestGenerator::add_stats(Stats &stats)
{
  stats.add("PATHS", "Paths", paths.size());
  stats.add("PATHS", "Strings", test_strings.size());
}
