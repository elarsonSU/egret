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
#include "RegexString.h"
using namespace std;

void
RegexString::process_min_iter_string(string &min_iter_string)
{
  if (repeat_lower != 0) {
    min_iter_string += get_substring();
  }
  else {
    min_iter_string = min_iter_string.substr(0, min_iter_string.length() - substring.length());
  }
}

set <string>
RegexString::gen_evil_strings(string path_string, const set <char> &punct_marks)
{
  set <string> evil_substrings;

  // insert one letter strings
  evil_substrings.insert("");
  evil_substrings.insert("_");
  evil_substrings.insert("6");
  evil_substrings.insert(" ");
  evil_substrings.insert(substring.substr(0, 1));

  // insert strings with added digit, space, and underscore
  unsigned int half = substring.length() / 2;
  string first_half = substring.substr(0, half);
  string second_half = substring.substr(half);
  evil_substrings.insert(first_half + "4" + second_half);
  evil_substrings.insert(first_half + " " + second_half);
  evil_substrings.insert(first_half + "_" + second_half);

  // insert all uppercase and all lowercase
  string all_upper;
  string all_lower;
  for (unsigned int i = 0; i < substring.length(); i++) {
    all_upper += toupper(substring[i]);
    all_lower += tolower(substring[i]);
  }
  evil_substrings.insert(all_upper);
  evil_substrings.insert(all_lower);

  // insert mixed case where first character is lowercase and second character
  // is uppercase
  string first = string(1, tolower(substring[0]));
  string second = string(1, toupper(substring[1]));
  string mixed = first + second + substring.substr(2);
  evil_substrings.insert(mixed);

  if (char_set->allows_punctuation()) {
    set <char>::iterator it;
    for (it = punct_marks.begin(); it != punct_marks.end(); it++) {
      evil_substrings.insert(string(1, *it));
    }
  }

  // generate the new full strings
  string path_suffix = path_string.substr(path_prefix.length() + substring.length());
  set <string> evil_strings;
  set <string>::iterator it;
  for (it = evil_substrings.begin(); it != evil_substrings.end(); it++) {
    string new_string;
    new_string = path_prefix + *it + path_suffix;
    evil_strings.insert(new_string);
  }

  return evil_strings;
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
