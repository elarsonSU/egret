/*  RegexString.h: represents a regex string

    Copyright (C) 2016-2018  Eric Larson and Anna Kirk
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

#ifndef REGEX_STRING_H
#define REGEX_STRING_H

#include <set>
#include <string>
#include <vector>
#include "CharSet.h"
#include "Util.h"
using namespace std;

class RegexString {

public:

  RegexString(CharSet *c, int lower, int upper) {
    char_set = c;
    repeat_lower = lower;
    repeat_upper = upper;
  }

  // setters
  void set_prefix(string p) { prefix = p; }
  void set_substring(string s) { substring = s; }

  // getters
  string get_substring() { return substring; }
  int get_repeat_lower() { return repeat_lower; }
  int get_repeat_upper() { return repeat_upper; }
  CharSet *get_charset() { return char_set; }

  // property function - used by checker
  // TODO: These functions could be refactored
  bool is_wild_candidate() { return char_set->is_wildcard() || char_set->is_complement(); }
  bool is_valid_character(char c) { return char_set->is_wildcard() || char_set->is_valid_character(c); }

  // repeat punctuation check functions
  bool is_repeat_punc_candidate() { return char_set->is_repeat_punc_candidate(); }
  char get_repeat_punc_char() { return char_set->get_repeat_punc_char(); }

  // generate minimum iterations string
  void gen_min_iter_string(string &min_iter_string);

  // generate evil strings
  vector <string> gen_evil_strings(string test_string, const set <char> &punct_marks);

  // print the regex string
  void print();

private:
  CharSet *char_set;		// corresponding character set
  int repeat_lower;     	// lower bound for string
  int repeat_upper;     	// upper bound for string

  string prefix;            // prefix of test string before the loop
  string substring;         // substring corresponding to this string
};

#endif // REGEX_STRING_H

