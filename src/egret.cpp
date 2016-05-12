/*  egret.cpp: entry point into EGRET engine

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
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "Stats.h"
#include "Scanner.h"
#include "ParseTree.h"
#include "NFA.h"
#include "Transition.h"
#include "error.h"

using namespace std;

// Global variables / options
static bool debug_mode = false;
static bool stat_mode = false;

// run_engine: entry point into EGRET engine
vector <string>
run_engine(string regex, bool debug = false, bool stat = false)
{
  vector <string> test_strings;

  // process arguments
  debug_mode = debug;
  stat_mode = stat;
  
  // clear warnings
  clearWarnings();

  try {
    // initialize scanner with regex
    Scanner scanner;
    scanner.init(regex);
  
    // build parse tree
    ParseTree tree;
    tree.create(scanner);

    // build NFA
    NFA nfa;
    nfa.init(tree);

    // traverse NFA
    test_strings = nfa.nfa_traverse();

    // print debug info
    if (debug_mode) {
      cout << "RegEx: " << regex << endl;
      scanner.print();
      tree.print();
      nfa.print();
    }

    // print stats
    Stats stats;
    if (stat_mode) {
      scanner.add_stats(stats);
      tree.add_stats(stats);
      nfa.add_stats(stats);
      stats.print();
    }
  }
  catch (EgretException const &e) {
    vector <string> result;
    result.push_back(e.getError());
    return result;
  }

  // Add warnings to front of list.
  string warnings = getWarnings();
  if (warnings == "") warnings = "SUCCESS";

  test_strings.insert(test_strings.begin(), warnings);

  return test_strings;
}
