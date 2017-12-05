/*  RegexString.cpp: represents a regex string

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

#include <set>
#include <string>
#include <iostream>
#include <algorithm>
#include "RegexString.h"
using namespace std;

void
RegexString::gen_min_iter_string(TestString &min_iter_string)
{
  if (repeat_lower != 0) {
    min_iter_string.append(get_substring());
  }
}

vector <TestString>
RegexString::gen_evil_strings(TestString test_string, const set <char> &punct_marks)
{
  vector <TestString> evil_substrings; // set of evil substrings

  // create suffix: substring after the loop
  int start = prefix.size() + substring.size();
  TestString suffix = test_string.create_substr(start);

  // insert one letter strings
  add_evil_substring(evil_substrings, "");
  add_evil_substring(evil_substrings, "_");
  add_evil_substring(evil_substrings, "6");
  add_evil_substring(evil_substrings, " ");

  // insert string with just first character of substring
  evil_substrings.push_back(substring.create_substr(0, 1));

  // split test string into two halves
  unsigned int half = substring.size() / 2;
  TestString before = substring.create_substr(0, half);
  TestString after = substring.create_substr(half);

  // insert strings with added digit, space, and underscore
  add_evil_substring(evil_substrings, before, "4", after);
  add_evil_substring(evil_substrings, before, " ", after);
  add_evil_substring(evil_substrings, before, "_", after);

  // insert all uppercase, all lowercase, and mixed case where
  // the first char is lowercase and the second char is uppercase
  string all_upper = substring.get_string();
  string all_lower = all_upper;
  string mixed = all_upper;

  for (unsigned int i = 0; i < substring.size(); i++) {
    all_upper[i] = toupper(all_upper[i], locale());
    all_lower[i] = tolower(all_lower[i], locale());
    if (i == 0) {
      mixed[i] = tolower(mixed[i], locale());
    } else if (i == 1) {
      mixed[i] = toupper(mixed[i], locale());
    }
  }
  add_evil_substring(evil_substrings, all_upper);
  add_evil_substring(evil_substrings, all_lower);
  add_evil_substring(evil_substrings, mixed);
  
  // insert strings for each punctation mark
  if (char_set->allows_punctuation()) {
    set <char>::iterator it;
    for (it = punct_marks.begin(); it != punct_marks.end(); it++) {
      add_evil_substring(evil_substrings, string(1, *it));
    }
  }

  // generate the new full strings
  vector <TestString> evil_strings;
  vector <TestString>::iterator tsi;
  for (tsi = evil_substrings.begin(); tsi != evil_substrings.end(); tsi++) {
    TestString new_string;
    new_string.append(prefix);
    new_string.append(*tsi);
    new_string.append(suffix);
    evil_strings.push_back(new_string);
  }

  return evil_strings;
}

void
RegexString::add_evil_substring(vector <TestString> &evil_substrings, string s)
{
  TestString t;
  t.append(s);
  evil_substrings.push_back(t);
}

void
RegexString::add_evil_substring(vector <TestString> &evil_substrings,
   TestString before, string s, TestString after)
{
  TestString t;
  t.append(before);
  t.append(s);
  t.append(after);
  evil_substrings.push_back(t);
}

void
RegexString::print()
{
  char_set->print();

  if (repeat_lower == 0 && repeat_upper == -1)
    cout << "*";
  else if (repeat_lower == 1 && repeat_upper == -1)
    cout << "+";
  else if (repeat_lower == 0 && repeat_upper == 1)
    cout << "?";
  else if (repeat_upper == -1)
    cout << "{" << repeat_lower << ",}";
  else if (repeat_lower == repeat_upper)
    cout << "{" << repeat_lower << "}";
  else
    cout << "{" << repeat_lower << "," << repeat_upper << "}";
}
