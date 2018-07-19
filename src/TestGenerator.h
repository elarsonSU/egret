/*  TestGenerator.h: Generates paths and strings

    Copyright (C) 2016-2018  Eric Larson and Anna Kirk
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
#include "Path.h"
using namespace std;

class TestGenerator {

public:

  TestGenerator(vector <Path> p, set <char> m, bool d) {
    paths = p;
    punct_marks = m;
    debug_mode = d;
  }

  // generate test strings
  vector <string> gen_test_strings();

  // add test generation stats
  void add_stats(Stats &stats);

private:

  vector <Path> paths;		// list of paths
  set <char> punct_marks;	// set of punct marks
  bool debug_mode;		// set if debug mode is on

  vector <string> test_strings;     // list of test strings

  int num_gen_strings;          // number of generated strings (for stats)

  // TEST STRING GENERATION FUNCTIONS

  // get initial set of strings
  void get_initial_strings();

  // generate minimum iteration strings
  void gen_min_iter_strings();

  // generates evil strings
  void gen_evil_strings();
};
#endif // TEST_GENERATOR_H
