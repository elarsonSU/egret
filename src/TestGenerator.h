/*  TestGenerator.h: Generates paths and strings

    Copyright (C) 2016  Eric Larson and Anna Kirk
    elarson@seattleu.edu

    Some code in this file was derived from a RE->NFA converter
    developed by Eli Bendersky.

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

#ifndef TEST_GENERATOR_H
#define TEST_GENERATOR_H

#include <set>
#include <string>
#include <vector>
#include "NFA.h"
#include "Path.h"
using namespace std;

class TestGenerator {

public:

  TestGenerator(NFA n, string b, set <char> p) { nfa = n; base_substring = b; punct_marks = p;}

  // generate test strings
  vector <string> gen_test_strings();

  // add test generation stats
  void add_stats(Stats &stats);

private:

  NFA nfa;				// NFA to traverse
  string base_substring;		// base string for regex strings
  set <char> punct_marks;		// set of punct marks
  vector <Path> paths;			// list of paths
  vector <string> test_strings;		// list of test strings
  
  // generates initial set of strings
  void gen_initial_strings();

  // adds a string to test string vector (unless it is already there)
  void add_to_test_strings(string s);

  // adds a set of strings to test string vector
  void add_to_test_strings(set <string> strs);

  // generates additional evil strings
  void gen_evil_strings();
};

#endif // TEST_GENERATOR_H
