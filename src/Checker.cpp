/*  Checker.cpp: Regular Expression Checker

    Copyright (C) 2016-18  Eric Larson and Tyler Hartje
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

#include <set>
#include <sstream>
#include <vector>
#include <iostream>

#include "Checker.h"
#include "Path.h"
#include "error.h"
using namespace std;

// Checker

void 
Checker::check()
{
  // perform various checks
  check_anchor_usage();
  check_anchor_in_middle();
  check_charsets();
}

// CHECKER FUNCTIONS

void
Checker::check_anchor_usage()
{
  bool all_start_with_caret = false;
  bool all_end_with_dollar = false;
  bool warn_caret_start = false;
  bool warn_dollar_end = false;
  
  bool is_first_string = true;
  string first_string;
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {

    // check for leading carets and trailing dollars
    bool start_with_caret = path_iter->has_leading_caret();
    bool end_with_dollar = path_iter->has_trailing_dollar();

    // for first path, record whether the path starts with ^ and/or ends with $
    if (is_first_string) {
      all_start_with_caret = start_with_caret;
      all_end_with_dollar = end_with_dollar;
      is_first_string = false;
      first_string = path_iter->get_test_string().get_string();
    }

    // print warning (but only for first occurrence of each anchor)
    if (!warn_caret_start) {
      if (all_start_with_caret && !start_with_caret) {
	string curr_string = path_iter->get_test_string().get_string();

        stringstream s;
        s << "Some but not all strings start with a ^ anchor\n";
        s << "...String with ^ anchor:    " << first_string << "\n";
        s << "...String with no ^ anchor: " << curr_string;
        addViolation("anchor usage", s.str());
        warn_caret_start = true;
      }
      if (!all_start_with_caret && start_with_caret) {
	string curr_string = path_iter->get_test_string().get_string();

        stringstream s;
        s << "Some but not all strings start with a ^ anchor\n";
        s << "...String with ^ anchor:    " << curr_string << "\n";
        s << "...String with no ^ anchor: " << first_string;
        addViolation("anchor usage", s.str());
        warn_caret_start = true;
      }
    }
    if (!warn_dollar_end) {
      if (all_end_with_dollar && !end_with_dollar) {
	string curr_string = path_iter->get_test_string().get_string();

        stringstream s;
        s << "Some but not all strings end with a $ anchor\n";
        s << "...String with $ anchor:    " << first_string << "\n";
        s << "...String with no $ anchor: " << curr_string;
        addViolation("anchor usage", s.str());
        warn_dollar_end = true;
      }
      if (!all_end_with_dollar && end_with_dollar) {
	string curr_string = path_iter->get_test_string().get_string();

        stringstream s;
        s << "Some but not all strings end with a $ anchor\n";
        s << "...String with $ anchor:    " << curr_string << "\n";
        s << "...String with no $ anchor: " << first_string;
        addViolation("anchor usage", s.str());
        warn_dollar_end = true;
      }
    }
  }
}

void
Checker::check_anchor_in_middle()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    if (path_iter->check_anchor_in_middle()) return;
  }
}

void
Checker::check_charsets()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    if (path_iter->check_charsets()) return;
  }
}
