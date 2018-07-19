/*  Checker.cpp: Regular Expression Checker

    Copyright (C) 2016-2018  Eric Larson and Tyler Hartje
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
#include <sstream>
#include <vector>
#include "Checker.h"
#include "Path.h"
#include "Util.h"
using namespace std;

// Checker

void 
Checker::check()
{
  check_anchor_usage();
  check_anchor_in_middle();
  check_charsets();
  check_optional_braces();
  check_wild_punctuation();
  check_repeat_punctuation();
  check_digit_too_optional();
}

// CHECKER FUNCTIONS

void
Checker::check_anchor_usage()
{
  bool all_start_with_caret = false;
  bool all_end_with_dollar = false;
  bool warn_caret_start = false;
  bool warn_dollar_end = false;
  
  // get end of line marker
  string eol = Util::get()->is_web_mode() ? "<br>" : "\n";

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
      first_string = path_iter->get_test_string();
    }

    // print warning (but only for first occurrence of each anchor)
    if (!warn_caret_start) {
      if (all_start_with_caret && !start_with_caret) {
	string curr_string = path_iter->get_test_string();

        stringstream s;
        s << "Some but not all strings start with a ^ anchor" << eol;
        s << "...String with ^ anchor: " << first_string << eol;
        s << "...String with no ^ anchor: " << curr_string;
        Alert a("anchor usage", s.str(), fix_anchors());
        Util::get()->add_alert(a);
        warn_caret_start = true;
      }
      if (!all_start_with_caret && start_with_caret) {
	string curr_string = path_iter->get_test_string();

        stringstream s;
        s << "Some but not all strings start with a ^ anchor" << eol;
        s << "...String with ^ anchor: " << curr_string << eol;
        s << "...String with no ^ anchor: " << first_string;
        Alert a("anchor usage", s.str(), fix_anchors());
        Util::get()->add_alert(a);
        warn_caret_start = true;
      }
    }
    if (!warn_dollar_end) {
      if (all_end_with_dollar && !end_with_dollar) {
	string curr_string = path_iter->get_test_string();

        stringstream s;
        s << "Some but not all strings end with a $ anchor" << eol;
        s << "...String with $ anchor: " << first_string << eol;
        s << "...String with no $ anchor: " << curr_string;
        Alert a("anchor usage", s.str(), fix_anchors());
        Util::get()->add_alert(a);
        warn_dollar_end = true;
      }
      if (!all_end_with_dollar && end_with_dollar) {
	string curr_string = path_iter->get_test_string();

        stringstream s;
        s << "Some but not all strings end with a $ anchor" << eol;
        s << "...String with $ anchor: " << curr_string << eol;
        s << "...String with no $ anchor: " << first_string;
        Alert a("anchor usage", s.str(), fix_anchors());
        Util::get()->add_alert(a);
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
    path_iter->check_charsets();
  }
}

void
Checker::check_optional_braces()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    path_iter->check_optional_braces();
  }
}

void
Checker::check_wild_punctuation()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    path_iter->check_wild_punctuation();
  }
}

void
Checker::check_repeat_punctuation()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    path_iter->check_repeat_punctuation();
  }
}

void
Checker::check_digit_too_optional()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    path_iter->check_digit_too_optional();
  }
}

string
Checker::fix_anchors()
{
  string new_regex = "^(";
  string regex = Util::get()->get_regex();

  vector <Token>::iterator vi;
  for (vi = tokens.begin(); vi != tokens.end(); vi++) {
    TokenType type = vi->type;
    if (type == CARET || type == DOLLAR) continue;
    for (int i = vi->loc.first; i <= vi->loc.second; i++) {
      new_regex += regex[i];
    }
  }
  
  new_regex += ")$";

  return new_regex;
}
