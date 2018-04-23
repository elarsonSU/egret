/*  TestGenerator.cpp: Generates paths and strings

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

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>
#include <iostream>

#include "NFA.h"
#include "TestGenerator.h"
#include "Path.h"
#include "error.h"
#include "TestString.h"
using namespace std;

// TEST STRING GENERATION FUNCTIONS

vector <string>
TestGenerator::gen_test_strings()
{
  vector <TestString>::iterator tsi;

  // get initial strings
  get_initial_strings();

  // TODO: Move to egret.cpp after path processing?
  // debug - print initial strings from basis paths
  if (debug_mode) {
    cout << "Initial Test Strings: " << endl;
    for (tsi = test_strings.begin(); tsi != test_strings.end(); tsi++) {
      cout << tsi->get_string() << endl;
    }
  }

  // generate backreference strings
  vector <string> backref_strs = gen_evil_backreference_strings();

  // gen minimum iteration strings
  gen_min_iter_strings();

  // gen evil strings
  gen_evil_strings();

  vector <string> return_strs; // set of strings to return

  // convert evil strings into actual strings and add to list
  for (tsi = test_strings.begin(); tsi != test_strings.end(); tsi++) {
    add_to_return_strings(return_strs, tsi->get_string());
  }

  // add backreference strings (already actual strings)
  vector <string>::iterator si;
  for (si = backref_strs.begin(); si != backref_strs.end(); si++) {
    add_to_return_strings(return_strs, *si);
  }

  // record number of generated strings for stats
  num_gen_strings = return_strs.size();

  return return_strs;
}

void
TestGenerator::get_initial_strings()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    add_to_test_strings(path_iter->get_test_string());
  }
}


vector <string>
TestGenerator::gen_evil_backreference_strings()
{
  vector <int> backrefs_done; // used to keep track of which backreferences have been processed.
  vector <string> backref_strs;

  vector <TestString>::iterator tsi;
  vector <string>::iterator si;
  for (tsi = test_strings.begin(); tsi != test_strings.end(); tsi++) {
    vector <string> new_strs = tsi->gen_evil_backreference_strings(backrefs_done);
    for (si = new_strs.begin(); si != new_strs.end(); si++) {
      backref_strs.push_back(*si);
    }
  }

  return backref_strs;
}

void
TestGenerator::gen_min_iter_strings()
{
  if (debug_mode) {
    cout << "Minimum Iteration Test Strings: " << endl;
  }

  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    TestString min_iter_string = path_iter->gen_min_iter_string();
    add_to_test_strings(min_iter_string);
    if (debug_mode)
      cout << min_iter_string.get_string() << endl;
  }
}

void
TestGenerator::gen_evil_strings()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    vector <TestString> evil_strings = path_iter->gen_evil_strings(punct_marks);
    add_to_test_strings(evil_strings);
  }
}

void
TestGenerator::add_to_test_strings(TestString s)
{
  test_strings.push_back(s);
}

void
TestGenerator::add_to_test_strings(vector <TestString> strs)
{
  vector <TestString>::iterator tsi;
  for (tsi = strs.begin(); tsi != strs.end(); tsi++) {
    add_to_test_strings(*tsi);
  }
}

void
TestGenerator::add_to_return_strings(vector <string> &return_strs, string s)
{
  if (!(std::find(return_strs.begin(), return_strs.end(), s) != return_strs.end()))
    return_strs.insert(return_strs.begin(), s);
}

// STAT FUNCTION

void
TestGenerator::add_stats(Stats &stats)
{
  // TODO: Should paths be included here?
  stats.add("PATHS", "Paths", paths.size());
  stats.add("PATHS", "Strings", num_gen_strings);
}
