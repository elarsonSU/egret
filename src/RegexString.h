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
#include <vector>
#include "CharSet.h"
#include "TestString.h"
using namespace std;

class RegexString {

public:

  RegexString(CharSet *c, int lower, int upper) {
    char_set = c;
    repeat_lower = lower;
    repeat_upper = upper;
  }

  // setters
  void set_prefix(TestString p) { prefix = p; }
  void set_substring(TestString s) { substring = s; }

  // getters
  TestString get_substring() { return substring; }

  // generate minimum iterations string
  void gen_min_iter_string(TestString &min_iter_string);

  // generate evil strings
  vector <TestString> gen_evil_strings(TestString test_string, const set <char> &punct_marks);

  // print the regex string
  void print();

private:
  CharSet *char_set;		// corresponding character set
  int repeat_lower;     	// lower bound for string
  int repeat_upper;     	// upper bound for string

  TestString prefix;            // prefix of test string before the loop
  TestString substring;         // substring corresponding to this string

  // add evil substrings to the set 
  void add_evil_substring(vector <TestString> &evil_substrings, string s);
  void add_evil_substring(vector <TestString> &evil_substrings,
    TestString before, string s, TestString after);
};

#endif // REGEX_STRING_H

