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
#include "TestString.h"
using namespace std;

class TestGenerator {

public:

  TestGenerator(NFA n, string b, set <char> p, bool d) {
    paths = n.find_basis_paths(); 
    base_substring.append(b); 
    punct_marks = p;
    debug_mode = d;
  }

  // generate test strings
  vector <string> gen_test_strings();

  // add test generation stats
  void add_stats(Stats &stats);

private:

  vector <Path> paths;		// list of paths
  TestString base_substring;    // base string for regex strings
  set <char> punct_marks;	// set of punct marks
  bool debug_mode;		// set if debug mode is on

  vector <TestString> test_strings;     // list of test strings

  int num_gen_strings;          // number of generated strings (for stats)

  // TEST STRING GENERATION FUNCTIONS

  // generates initial set of strings
  void gen_initial_strings();

  // generate backreference strings
  vector <string> gen_evil_backreference_strings();

  // generate minimum iteration strings
  void gen_min_iter_strings();

  // generates evil strings
  void gen_evil_strings();

  // adds a string to test string vector (unless it is already there)
  void add_to_test_strings(TestString s);

  // adds a set of strings to test string vector
  void add_to_test_strings(vector <TestString> strs);

  // add a string to return strings (unless it is already there)
  void add_to_return_strings(vector <string> &return_strs, string s);

  // CHECKER FUNCTIONS

  // check anchor usage
  void check_anchor_usage();

  // check anchor in middle
  void check_anchor_in_middle();

  // check chararacter sets
  void check_charsets();
};

#endif // TEST_GENERATOR_H
