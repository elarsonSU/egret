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
RegexString::process_min_iter_string(StringPath *min_iter_string)
{
  if (repeat_lower != 0) {
    min_iter_string->add_path(get_substring());
  }
}

set <StringPath, spcompare>
RegexString::gen_evil_strings(StringPath path_string, const set <char> &punct_marks)
{
  set <StringPath, spcompare> evil_substrings;
  set <StringPath, spcompare> evil_strings;

  // insert one letter strings
  StringPath a;
  a.add_string("");
  evil_substrings.insert(a);
  StringPath b;
  b.add_string("_");
  evil_substrings.insert(b);
  StringPath c;
  c.add_string("6");
  evil_substrings.insert(c);
  StringPath d;
  d.add_string(" ");
  evil_substrings.insert(d);
  StringPath e;
  e.add_path_item(substring.path[0]);
  evil_substrings.insert(e);

  // insert strings with added digit, space, and underscore
  int half = substring.path.size() / 2;
  StringPath first_half;
  for(int i = 0; i < half; i++) first_half.add_path_item(substring.path[i]);
  StringPath second_half;
  int max = substring.path.size();
  for(int i = half; i < max; i++) second_half.add_path_item(substring.path[i]);

  StringPath f;
  f.add_path(first_half);
  f.add_string("4");
  f.add_path(second_half);
  evil_substrings.insert(f);
  StringPath g;
  g.add_path(first_half);
  g.add_string(" ");
  g.add_path(second_half);
  evil_substrings.insert(g);
  StringPath h;
  h.add_path(first_half);
  h.add_string("_");
  h.add_path(second_half);
  evil_substrings.insert(h);

  // insert all uppercase and all lowercase
  StringPath all_upper;
  StringPath all_lower;
  for (unsigned int i = 0; i < substring.path.size(); i++) {
    all_upper.add_path_item(substring.path[i]);
    all_lower.add_path_item(substring.path[i]);
    char s = substring.path[i].item;
    s = std::toupper(s, std::locale());
    all_upper.path[i].item = s;
    s = std::tolower(s, std::locale());
    all_lower.path[i].item = s;
  }
  evil_substrings.insert(all_upper);
  evil_substrings.insert(all_lower);

  // insert mixed case where first character is lowercase and second character
  // is uppercase
  StringPath first;
  first.add_path_item(substring.path[0]);
  char s1 = first.path[0].item;
  s1 = std::tolower(s1, std::locale());
  first.path[0].item = s1;
  StringPath second;
  second.add_path_item(substring.path[1]);
  char s2 = second.path[0].item;
  s2 = std::toupper(s2, std::locale());
  second.path[0].item = s2;
  StringPath mixed;
  mixed.add_path(first);
  mixed.add_path(second);
  for(unsigned int i = 2; i < substring.path.size(); i++) mixed.add_path_item(substring.path[i]);
  evil_substrings.insert(mixed);  
  
  if (char_set->allows_punctuation()) {
    set <char>::iterator it;
    for (it = punct_marks.begin(); it != punct_marks.end(); it++) {
      StringPath p;
      string s = string(1, *it);
      p.add_string(s);
      evil_substrings.insert(p);
    }
  }

  // generate the new full strings
  StringPath path_suffix;
  int start = path_prefix.path.size() + substring.path.size();
  int end = path_string.path.size();
  for(int i = start; i < end; i++) path_suffix.add_path_item(path_string.path[i]);
  set <StringPath, spcompare>::iterator it;
  
  for (it = evil_substrings.begin(); it != evil_substrings.end(); it++) {
    StringPath new_string;
    new_string.add_path(path_prefix);
    StringPath ptr = *it;
    new_string.add_path(ptr);
    new_string.add_path(path_suffix);
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
