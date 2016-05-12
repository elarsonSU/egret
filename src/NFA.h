/*  NFA.h: Nondeterminstic Finite State Automaton

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

#ifndef NFA_H
#define NFA_H

#include <vector>
#include <map>
#include <set>
#include "State.h"
#include "Transition.h"
#include "CharSet.h"
#include "ParseTree.h"
#include "Stats.h"

using namespace std;

class NFA
{
public:

  // constructors
  NFA() {}
  NFA(unsigned int _size, unsigned int _initial, unsigned int _final);
  NFA(const NFA &other);
  NFA &operator= (const NFA &other);

  // Build an NFA from a parse tree.
  void init(ParseTree &tree);

  // Find all paths through NFA.
  vector <string> nfa_traverse();

  // Print out the NFA.
  void print();

  // Add NFA stats.
  void add_stats(Stats &stats);

private:

  unsigned int size;			// number of states
  unsigned int initial;			// initial state
  unsigned int final;			// final state
  vector <vector <Transition> > trans_table;	// transition table
  vector <State> states;		// list of states  
  set<char> punct_marks;		// set of punctuation marks
  vector <vector <unsigned int> > paths;	// list of paths
  vector <string> path_strings;		// list of paths strings
  vector <unsigned int> end_states;	// list of end states that need further attention
  vector <string> test_strings;		// list of test strings
  
  // variables for processing carets and dollars
  bool all_start_with_caret;
  bool all_end_with_dollar;
  bool warn_caret_middle;
  bool warn_dollar_middle;
  bool warn_caret_start;
  bool warn_dollar_end;

  //
  // HELPER NFA BUILDING FUNCTIONS
  //

  // Builds an NFA from tree
  NFA build_nfa_from_tree(ParseNode *tree);

  // Builds an alternation of nfa1 and nfa2 (nfa1|nfa2)
  NFA build_nfa_alternation(NFA nfa1, NFA nfa2);

  // Builds a concatenation of nfa1 and nfa2 (nfa1nfa2)
  NFA build_nfa_concat (NFA nfa1, NFA nfa2);

  // Builds nfa{m,n}
  NFA build_nfa_repeat(NFA nfa, int repeat_lower, int repeat_upper);

  // Builds special node for regex strings such as .+ or \w*
  NFA build_nfa_string(ParseNode *tree, int repeat_lower, int repeat_upper);

  // Builds (nfa)
  NFA build_nfa_group(NFA nfa);

  // Builds nfa with character
  NFA build_nfa_character(char character);

  // Builds nfa with caret
  NFA build_nfa_caret();

  // Builds nfa with dollar
  NFA build_nfa_dollar();

  // Builds nfa with ignored element - simply consists of two nodes and an epsilon
  // transition
  NFA build_nfa_ignored();

  // Builds nfa with char set as input
  NFA build_nfa_char_set(CharSet *char_set);

  // Adds a transition between two states - regular character input
  void add_transition(unsigned int from, unsigned int to, Transition trans);

  // Shift (renames) all the states in the NFA according to some (positive) shift factor
  void shift_states(unsigned int shift);

  // Fills states 0 up to other.size with other's states
  void fill_states(const NFA &other);

  // Appends a new empty state to the NFA
  void append_empty_state();

  // Returns true if repeat quantifier represents a string
  bool is_regex_string(ParseNode *node, int repeat_lower, int repeat_upper);

  // HELPER NFA TRAVERSAL FUNCTIONS
  //

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

#endif // NFA_H

