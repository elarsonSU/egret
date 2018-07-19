/*  Backref.cpp: represents a regex repeat quantifier

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

#include <iostream>
#include <set>
#include <string>
#include "Backref.h"
using namespace std;

void
Backref::gen_min_iter_string(string &min_iter_string)
{
  min_iter_string.append(get_substring());
}

vector <string>
Backref::gen_evil_strings(string test_string)
{
  vector <string> evil_strings;
  return evil_strings;

#if 0 // TODO: Reimplement backreference evil strings
  // Create suffix: substring after the loop
  int start = prefix.size() + substring.size();
  string suffix = test_string.create_substr(start);

  // Create string with one less iteration
  string one_less_string = prefix;
  one_less_string.append(suffix);

  // Create string with one more iteration
  string one_more_string = prefix;
  one_more_string.append(substring);
  one_more_string.append(substring);
  one_more_string.append(suffix);

  if (repeat_upper != -1) {

    // For cases like {n}, add strings for one less (n-1) and one more (n+1).
    if (repeat_lower == repeat_upper) {
      evil_strings.push_back(one_less_string);
      evil_strings.push_back(one_more_string);
    }
    else {
      // Handle one less on lower bound (note if lower bound is zero, the path
      // has one iteration so one less iteration will get us to zero iterations)
      evil_strings.push_back(one_less_string);

      // Add enough path elements to get to the upper bound (note if lower bound
      // is zero, the path has one iteration so the starting point is bumped to one).
      // The variable path_elements is initialized to substring since suffix
      // has one substring less than lower bound.
      int base_iterations = repeat_lower;
      if (base_iterations == 0) base_iterations = 1;
      string path_elements = substring;
      for (int i = base_iterations; i < repeat_upper; i++) {
        path_elements.append(substring);
      }

      // Add the upper bound string.
      string upper_bound_string = prefix;
      upper_bound_string.append(path_elements);
      upper_bound_string.append(suffix);
      evil_strings.push_back(upper_bound_string);

      // Add the string with one more iteration past the upper bound.
      string past_bound_string = prefix;
      past_bound_string.append(path_elements);
      past_bound_string.append(substring);
      past_bound_string.append(suffix);
      evil_strings.push_back(past_bound_string);
    } 
  }

  else {    // repeat_upper == -1 (no limit)
    // If lower bound is 0 or 1, add one less (zero) and add one more (two).  Want
    // to have one case that has repeated (two) elements.
    if (repeat_lower == 0 || repeat_lower == 1) {
      evil_strings.push_back(one_less_string);
      evil_strings.push_back(one_more_string);
    }
    // Otherwise, only add the string with one less iteration than the lower bound.
    else {
      evil_strings.push_back(one_less_string);
    }
  }

  return evil_strings;
#endif

}

#if 0 // TODO: Reimplement backreference evil strings
vector <string>
gen_evil_backreference_strings(vector <int> &backrefs_done)
{
  // create evil strings for each backreference
  vector <int>::iterator bit;
  for (bit = backrefs.begin(); bit != backrefs.end(); bit++) {

    string evil_add = "";
    string evil_remove = "";
    string evil_modify = "";

    for (it = test_string.begin(); it != test_string.end(); it++) {
      if (it->type == TEST_STR_CHAR) {
	evil_add += it->character;
	evil_remove += it->character;
	evil_modify += it->character;
      }
      else if (it->type == TEST_STR_BACKREFERENCE) {
	string temp;

	// if current backreference, generate evil strings
	if (it->id == *bit) {

	  // evil_add
	  temp = group_strings[it->group];
	  temp.push_back(temp.back());
	  evil_add += temp;

	  // evil_remove
	  temp = group_strings[it->group];
	  if(temp.length() > 0) {
	    temp.pop_back();
	  }
	  evil_remove += temp;

	  // evil_modify
	  temp = group_strings[it->group];
	  int edit = temp.length()/2;
	  char c = temp[edit];
	  c += 1;
	  temp[edit] = c;
	  evil_modify += temp;
	}

	// not current backreference --> normal backref substitution
	else {
	  temp = group_strings[it->group];
	  evil_add += temp;
	  evil_remove += temp;
	  evil_modify += temp;
	}
      }
    }
    evil_strings.push_back(evil_add);
    evil_strings.push_back(evil_remove);
    evil_strings.push_back(evil_modify);
  }
  
  return evil_strings;
}
#endif

void
Backref::print()
{
  cout << "Group " << group_number;
  if (group_name != "") {
    cout << "(" << group_name << ")";
  }
  cout << " @ (" << group_loc.first << "," << group_loc.second << ")";
}

