/*  egret.cpp: entry point into EGRET engine

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

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "Checker.h"
#include "NFA.h"
#include "ParseTree.h"
#include "Path.h"
#include "Scanner.h"
#include "Stats.h"
#include "TestGenerator.h"
#include "Util.h"

using namespace std;

vector <string>
run_engine(string regex, string base_substring, bool check_mode = false, bool web_mode = false,
    bool debug_mode = false, bool stat_mode = false)
{
  Stats stats;
  vector <string> test_strings;

  try {

    // check and convert base substring
    if (base_substring.length() < 2) {
      throw EgretException("ERROR (bad arguments): Base substring must have at least two letters");
    }
    for (unsigned int i = 0; i < base_substring.length(); i++) {
      if (!isalpha(base_substring[i])) {
        throw EgretException("ERROR (bad arguments): Base substring can only contain letters");
      }
    }

    // set global options
    Util::get()->init(regex, check_mode, web_mode, base_substring);

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
      path_iter->process_path();
    }

    // run checker
    if (check_mode) {
      Checker checker(paths, scanner.get_tokens());
      checker.check();
    }

    // generate tests
    if (!check_mode) {
      TestGenerator gen(paths, tree.get_punct_marks(), debug_mode);
      test_strings = gen.gen_test_strings();
      if (stat_mode) gen.add_stats(stats);
    }
    
    // print stats
    if (stat_mode) stats.print();
  }
  catch (EgretException const &e) {
    vector <string> result;
    result.push_back(e.get_error());
    return result;
  }

  // Add alerts to front of list.
  vector <string> alerts = Util::get()->get_alerts();
  if (check_mode) {
    if (alerts.size() == 0) {
      alerts.insert(alerts.begin(), "No violations detected.");
    }
    return alerts;
  }
  test_strings.insert(test_strings.begin(),"BEGIN");
  test_strings.insert(test_strings.begin(), alerts.begin(), alerts.end());

  return test_strings;
}
