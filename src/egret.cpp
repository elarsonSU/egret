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
#include <string>
#include <vector>
#include <algorithm>
#include "NFA.h"
#include "ParseTree.h"
#include "Path.h"
#include "Scanner.h"
#include "Stats.h"
#include "Checker.h"
#include "TestGenerator.h"
#include "TestString.h"
#include "error.h"

using namespace std;

vector <string>
run_engine(string regex, string base,
    bool check_only = false, bool debug_mode = false, bool stat_mode = false)
{
  Stats stats;
  vector <string> test_strings;

  // clear alerts
  clearAlerts();

  try {

    // check and convert base substring
    TestString base_substring;
    if (base.length() < 2) {
      throw EgretException("ERROR (bad arguments): Base substring must have at least two letters");
    }
    for (unsigned int i = 0; i < base.length(); i++) {
      if (!isalpha(base[i])) {
        throw EgretException("ERROR (bad arguments): Base substring can only contain letters");
      }
    }
    base_substring.append(base);

    // start debug mode
    if (debug_mode) cout << "RegEx: " << regex << endl;
     
    // initialize scanner with regex
    Scanner scanner;
    scanner.init(regex);
    if (debug_mode) scanner.print();
    if (stat_mode) scanner.add_stats(stats);
  
    // build parse tree
    ParseTree tree;
    tree.build(scanner);
    if (debug_mode) tree.print();
    if (stat_mode) tree.add_stats(stats);

    // build NFA
    NFA nfa;
    nfa.build(tree);
    if (debug_mode) nfa.print();
    if (stat_mode) nfa.add_stats(stats);

    // traverse NFA basis paths and process them
    vector <Path> paths = nfa.find_basis_paths();
    vector <Path>::iterator path_iter;
    for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
      path_iter->process_path(base_substring);
    }

    // run checker
    Checker checker(paths, base_substring, tree.get_punct_marks(), debug_mode);
    checker.check();

    // generate tests
    if (!check_only) {
      TestGenerator gen(paths, base_substring, tree.get_punct_marks(), debug_mode);
      test_strings = gen.gen_test_strings();
      if (stat_mode) gen.add_stats(stats);
    }
    
    // print stats
    if (stat_mode) stats.print();
  }
  catch (EgretException const &e) {
    vector <string> result;
    result.push_back(e.getError());
    return result;
  }

  // Add alerts to front of list.
  string alerts = getAlerts();
  if (alerts == "") alerts = "SUCCESS";
  test_strings.insert(test_strings.begin(), alerts);

  return test_strings;
}
