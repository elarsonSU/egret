/*  RegexLoop.cpp: represents a regex repeat quantifier

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
#include "RegexLoop.h"
using namespace std;

string
RegexLoop::get_substring()
{
  // Adds the loop substring a sufficient number of times if the lower
  // bound is greater than 1.
  string substring;
  for (int j = 1; j < repeat_lower; j++) {
    substring += curr_substring;
  }

  return substring;
}

void
RegexLoop::process_begin_loop(string prefix, bool processed)
{
  curr_prefix = prefix;
  if (!processed) path_prefix = prefix;
}

void
RegexLoop::process_end_loop(string prefix, bool processed)
{
  curr_substring = prefix.substr(curr_prefix.length());
  if (!processed) path_substring = curr_substring;
}

set <string>
RegexLoop::gen_evil_strings(string path_string)
{
  set <string> evil_strings;
  string path_suffix = path_string.substr(path_prefix.length() + path_substring.length());
  string one_less_string = path_prefix + path_suffix;
  string one_more_string = path_prefix + path_substring + path_substring + path_suffix;

  if (repeat_upper != -1) {

    // For cases like {n}, add strings for one less (n-1) and one more (n+1).
    if (repeat_lower == repeat_upper) {
      evil_strings.insert(one_less_string);
      evil_strings.insert(one_more_string);
    }
    else {
      // Handle one less on lower bound (note if lower bound is zero, the path
      // has one iteration so one less iteration will get us to zero iterations)
      evil_strings.insert(one_less_string);

      // Add enough path elements to get to the upper bound (note if lower bound
      // is zero, the path has one iteration so the starting point is bumped to one).
      // The variable path_elements is initialized to path_substring since path_suffix
      // has one substring less than lower bound.
      int base_iterations = repeat_lower;
      if (base_iterations == 0) base_iterations = 1;
      string path_elements = path_substring;
      for (int i = base_iterations; i < repeat_upper; i++) {
        path_elements += path_substring;
      }

      // Add the upper bound string.
      string upper_bound_string = path_prefix + path_elements + path_suffix;
      evil_strings.insert(upper_bound_string);

      // Add the string with one more iteration past the upper bound.
      string past_bound_string = path_prefix + path_elements + path_substring + path_suffix;
      evil_strings.insert(past_bound_string);
    } 
  }

  else {    // repeat_upper == -1 (no limit)
    // If lower bound is 0 or 1, add one less (zero) and add one more (two).  Want
    // to have one case that has repeated (two) elements.
    if (repeat_lower == 0 || repeat_lower == 1) {
      evil_strings.insert(one_less_string);
      evil_strings.insert(one_more_string);
    }
    // Otherwise, only add the string with one less iteration than the lower bound.
    else {
      evil_strings.insert(one_less_string);
    }
  }

  return evil_strings;
}

void
RegexLoop::print()
{
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
