/*  RegexString.h: represents a regex string

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

#ifndef REGEX_STRING_H
#define REGEX_STRING_H

#include <set>
#include <string>
#include "CharSet.h"
using namespace std;

class RegexString {

public:

  RegexString(CharSet *c, int lower, int upper) {
    char_set = c;
    repeat_lower = lower;
    repeat_upper = upper;
  }

  void set_path_prefix(string p) { path_prefix = p; }
  void set_substring(string s) { substring = s; }
  string get_substring() { return substring; }

  // generate evil strings
  set <string> gen_evil_strings(string path_string, const set <char> &punct_marks);

  // print the regex string
  void print();

private:
  CharSet *char_set;		// corresponding character set
  int repeat_lower;     	// lower bound for string
  int repeat_upper;     	// upper bound for string
  string path_prefix;           // path string up to visiting this node
  string substring;             // substring corresponding to this string
};

#endif // REGEX_STRING_H

