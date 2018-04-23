/*  Checker.h: Regular Expression Checker

    Copyright (C) 2016-18  Eric Larson and Tyler Hartje
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

#ifndef CHECKER_H
#define CHECKER_H

#include <set>
#include <string>
#include <vector>
#include "Path.h"
#include "TestString.h"
using namespace std;

class Checker {

public:

  // TODO: Need all four of these arguments?
  Checker(vector <Path> p, TestString b, set <char> m, bool d) {
    paths = p;
    base_substring = b;
    punct_marks = m;
    debug_mode = d;
  }

  // checker entry point
  void check();

private:

  vector <Path> paths;		// list of paths
  TestString base_substring;    // base string for regex strings
  set <char> punct_marks;	// set of punct marks
  bool debug_mode;		// set if debug mode is on

  // CHECKER FUNCTIONS

  // check anchor usage
  void check_anchor_usage();

  // check anchor in middle
  void check_anchor_in_middle();

  // check chararacter sets
  void check_charsets();
};

#endif // CHECKER_H
