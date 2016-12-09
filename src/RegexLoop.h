/*  RegexLoop.h: represents a regex repeat quantifier

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

#ifndef REGEX_LOOP_H
#define REGEX_LOOP_H

#include <set>
#include <string>
using namespace std;

class RegexLoop {

public:

  RegexLoop(int lower, int upper) {
    repeat_lower = lower;
    repeat_upper = upper;
  }

  // get substring - additional iterations for lower bounds greater than 1
  string get_substring();

  // process minimum iterations string
  void process_min_iter_string(string &min_iter_string);

  // process begin loop edge
  void process_begin_loop(string prefix, bool processed);

  // process end loop edge
  void process_end_loop(string prefix, bool processed);

  // generate evil strings
  set <string> gen_evil_strings(string path_string);

  // print the regex loop
  void print();

private:

  int repeat_lower;     	// lower bound for repeat quantifiers 
  int repeat_upper;     	// upper bound for repeat quantifiers (-1 if no bound)
  string path_prefix;           // path string up to visiting this node
  string path_substring;        // substring corresponding to this string
  string curr_prefix;           // current path string up to visiting this node
  string curr_substring;        // current substring corresponding to this string
};

#endif // REGEX_LOOP_H

