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

#include <vector>
#include <map>
#include <set>
#include "NFA.h"
#include "State.h"
#include "Transition.h"
#include "CharSet.h"
#include "ParseTree.h"
#include "Stats.h"

using namespace std;

class TestGenerator
{
public:

  // constructors
  TestGenerator(NFA _nfa) { nfa = _nfa;}

  // Find all paths through NFA.
  vector <string> gen_test_strings();

  // Add generation stats.
  void add_stats(Stats &stats);

private:

  NFA nfa;	// NFA to traverse
  bool *visited;			// array of visited states
  vector <vector <unsigned int> > paths;	// list of paths
  vector <string> path_strings;		// list of paths strings
  vector <unsigned int> end_states;	// list of end states that need further attention
  vector <string> test_strings;		// list of test strings
  
  // HELPER NFA TRAVERSAL FUNCTIONS

  // Utility function to find all paths through the NFA.
  void find_all_paths(unsigned int curr_state, vector <unsigned int> path);
  
  // Determines if the path should be added to the list of paths.  If so, a path
  // string is generated for that path.
  void process_path(vector <unsigned int> path);

  // Initialize set of strings by copying path strings, removing duplicates.
  void init_test_strings();

  // Adds a string to test string vector (unless it is already there).
  void add_to_test_strings(string s);

  // Generate additional strings by visiting end nodes.
  void visit_end_nodes();

  // Process loop constructs by adding one less and one more iteration.
  void process_test_string_loop(State state, bool skip_one_less = false, bool skip_one_more = false);

};

#endif // TEST_GENERATOR_H
