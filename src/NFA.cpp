/*  NFA.cpp: Nondeterminstic Finite State Automaton

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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

#include "NFA.h"
#include "Transition.h"
#include "ParseTree.h"
#include "error.h"

using namespace std;

// constants
static const Transition EPSILON_TRANS = { EPSILON, '\0' };
static const Transition EMPTY_TRANS = { EMPTY };

// constructor
NFA::NFA(unsigned int _size, unsigned int _initial, unsigned int  _final)
{
  size = _size;
  initial = _initial;
  final = _final;

  assert(initial < size);
  assert(final < size);

  // Initialize trans_table with an "empty graph" (no transitions)
  vector <Transition> empty_row(size, EMPTY_TRANS);
  for (unsigned int i = 0; i < size; i++) {
    trans_table.push_back(empty_row);
  }
}

// copy constructor
NFA::NFA(const NFA &other)
{
  size = other.size;
  initial = other.initial;
  final = other.final;
  trans_table = other.trans_table;
  loops = other.loops;
}

// overloaded assignment operator
NFA &
NFA::operator=(const NFA & other)
{
  if (this == &other)
    return *this;

  initial = other.initial;
  final = other.final;
  size = other.size;
  trans_table = other.trans_table;
  loops = other.loops;

  return *this;
}

//
// NFA BUILDING FUNCTIONS
//
// Using Thompson Construction, build NFAs from basic inputs or
// compositions of other NFAs.
//

// Initialize an NFA using a parse tree.
void
NFA::init(ParseTree &tree)
{
  // Initialize members
  punct_marks = tree.get_punct_marks();

  // Build NFA
  NFA nfa = build_nfa_from_tree(tree.get_root());

  // Copy NFA
  initial = nfa.initial;
  final = nfa.final;
  size = nfa.size;
  trans_table = nfa.trans_table;
  loops = nfa.loops;
}

//
// Builds an NFA from tree
//
NFA
NFA::build_nfa_from_tree(ParseNode *tree)
{
  assert(tree);

  switch (tree->type) {

  case ALTERNATION_NODE:
    return build_nfa_alternation(build_nfa_from_tree(tree->left),
	build_nfa_from_tree(tree->right));

  case CONCAT_NODE:
    return build_nfa_concat(build_nfa_from_tree(tree->left),
	build_nfa_from_tree(tree->right));

  case REPEAT_NODE:
#if 0
    if (is_regex_string(tree->left, tree->repeat_lower, tree->repeat_upper))
      return build_nfa_string(tree->left, tree->repeat_lower, tree->repeat_upper);
    else
#endif
      return build_nfa_repeat(build_nfa_from_tree(tree->left),
	tree->repeat_lower, tree->repeat_upper);

  case GROUP_NODE:
    return build_nfa_group(build_nfa_from_tree(tree->left));

  case CHARACTER_NODE:
    return build_nfa_character(tree->character);

  case CARET_NODE:
    return build_nfa_caret();

  case DOLLAR_NODE:
    return build_nfa_dollar();

  case CHAR_SET_NODE:
    return build_nfa_char_set(tree->char_set);

  case IGNORED_NODE:
    return build_nfa_ignored();

  default:
    throw EgretException("ERROR (internal): Invalid node type in parse tree");
  }
}

// Builds an alternation of nfa1 and nfa2 (nfa1|nfa2)
//
NFA
NFA::build_nfa_alternation(NFA nfa1, NFA nfa2)
{
  // How this is done: the new nfa must contain all the states in
  // nfa1 and nfa2, plus new initial and final states.
  // First will come the new initial state, then nfa1's states, then
  // nfa2's states, then the new final state.

  // make room for the new initial state
  nfa1.shift_states(1);

  // make room for nfa1
  nfa2.shift_states(nfa1.size);

  // create a new nfa and initialize it with (the shifted) nfa2
  NFA new_nfa(nfa2);

  // nfa1's states take their places in new_nfa
  new_nfa.fill_states(nfa1);

  // Set new initial state and the transitions from it
  new_nfa.add_transition(0, nfa1.initial, EPSILON_TRANS);
  new_nfa.add_transition(0, nfa2.initial, EPSILON_TRANS);
  new_nfa.initial = 0;

  // Make up space for the new final state
  new_nfa.append_empty_state();

  // Set new final state
  new_nfa.final = new_nfa.size - 1;
  new_nfa.add_transition(nfa1.final, new_nfa.final, EPSILON_TRANS);
  new_nfa.add_transition(nfa2.final, new_nfa.final, EPSILON_TRANS);

  return new_nfa;
}

// Builds a concatenation of nfa1 and nfa2 (nfa1nfa2)
//
NFA
NFA::build_nfa_concat(NFA nfa1, NFA nfa2)
{
  // How this is done: First will come nfa1, then nfa2 (its
  // initial state replaced with nfa1's final state)

  // make room for nfa1
  nfa2.shift_states(nfa1.size);

  // create a new nfa and initialize it with (the shifted) nfa2
  NFA new_nfa(nfa2);

  // nfa1's states take their places in new_nfa
  new_nfa.fill_states(nfa1);

  // add transition from nfa1 to nfa2
  new_nfa.add_transition(nfa1.final, new_nfa.initial, EPSILON_TRANS);

  // set the new initial state (the final state stays nfa2's final state,
  // and was already copied)
  new_nfa.initial = nfa1.initial;

  return new_nfa;
}

// Builds nfa{m,n}
//
NFA
NFA::build_nfa_repeat(NFA nfa, int repeat_lower, int repeat_upper)
{
  // make room for the new initial state
  nfa.shift_states(1);

  // make room for the new final state
  nfa.append_empty_state();

  // add new transitions
  nfa.add_transition(0, nfa.initial, EPSILON_TRANS);	   // new initial to old initial
  nfa.add_transition(nfa.final, nfa.size - 1, EPSILON_TRANS); // old final to new final

  // update states
  nfa.initial = 0;
  nfa.final = nfa.size - 1;

  // add loop to list
  RegexLoop loop;
  loop.begin_state = nfa.initial;
  loop.end_state = nfa.final;
  loop.repeat_lower = repeat_lower;
  loop.repeat_upper = repeat_upper;
  loops.push_back(loop);

  return nfa;
}

// Builds special node for regex strings such as .+ or \w*
NFA
NFA::build_nfa_string(ParseNode *node, int repeat_lower, int repeat_upper)
{
  // Initialize the NFA
  NFA nfa(2, 0, 1);
  Transition trans;
  trans.type = STRING_INPUT;
  trans.regex_str = new RegexString;
  trans.regex_str->char_set = node->char_set;
  trans.regex_str->repeat_lower = repeat_lower;
  trans.regex_str->repeat_upper = repeat_upper;
  nfa.add_transition(0, 1, trans);

  return nfa;
}

// Builds (nfa)
//
NFA
NFA::build_nfa_group(NFA nfa)
{
  return nfa;
}

// Builds nfa with character
//
NFA
NFA::build_nfa_character(char character)
{
  NFA nfa(2, 0, 1);	// size = 2, initial = 0 , final = 1
  Transition trans;
  trans.type = CHARACTER_INPUT;
  trans.character = character;
  nfa.add_transition(0, 1, trans);
  return nfa;
}

// Builds nfa with caret
//
NFA
NFA::build_nfa_caret()
{
  NFA nfa(2, 0, 1);	// size = 2, initial = 0 , final = 1
  Transition trans;
  trans.type = CARET_INPUT;
  nfa.add_transition(0, 1, trans);
  return nfa;
}

// Builds nfa with dollar
//
NFA
NFA::build_nfa_dollar()
{
  NFA nfa(2, 0, 1);	// size = 2, initial = 0 , final = 1
  Transition trans;
  trans.type = DOLLAR_INPUT;
  nfa.add_transition(0, 1, trans);
  return nfa;
}

// Builds nfa with ignored element - simply consists of two nodes and an epsilon
// transition
//
NFA
NFA::build_nfa_ignored()
{
  NFA nfa(2, 0, 1);	// size = 2, initial = 0 , final = 1
  nfa.add_transition(0, 1, EPSILON_TRANS);
  return nfa;
}

// Builds nfa with char set as input
//
NFA
NFA::build_nfa_char_set(CharSet *char_set)
{
  // Initialize the NFA
  NFA nfa(2, 0, 1);
  Transition trans;
  trans.type = CHAR_SET_INPUT;
  trans.char_set = char_set;
  nfa.add_transition(0, 1, trans);
  return nfa;
}

// Adds a transition to trans_table that has regular character input
void
NFA::add_transition(unsigned int from, unsigned int to, Transition trans)
{
  assert(from < size);
  assert(to < size);

  trans_table[from][to] = trans;
}

// Shift all the states in the NFA according to some (positive) shift factor
//
// For each NFA state: number += shift
//
// Functionally, this doesn't affect the NFA, it only makes it larger and renames its states
void
NFA::shift_states(unsigned int shift)
{
  unsigned int new_size = size + shift;

  if (shift < 1) return;

  // create a new, empty transition table (of the new size)
  vector <Transition> empty_row(new_size, EMPTY_TRANS);
  vector <vector <Transition> > new_trans_table(new_size, empty_row);

  // copy all the transitions to the new table, at their new locations
  for (unsigned int i = 0; i < size; i++) {
    for (unsigned int j = 0; j < size; j++) {
      new_trans_table[i + shift][j + shift] = trans_table[i][j];
    }
  }

  // update indices in the loops list
  vector <RegexLoop>::iterator it;
  for (it = loops.begin(); it != loops.end(); it++) {
    it->begin_state += shift;
    it->end_state += shift;
  }

  // update the NFA members
  size = new_size;
  initial += shift;
  final += shift;
  trans_table = new_trans_table;
}

// Fills states 0 up to other.size from other.
// Requires the use of shift_states first.
void
NFA::fill_states(const NFA &other)
{
  for (unsigned int i = 0; i < other.size; i++) {
    for (unsigned int j = 0; j < other.size; j++) {
      trans_table[i][j] = other.trans_table[i][j];
    }
  }
}

// Appends a new empty state to the NFA
void
NFA::append_empty_state()
{
  // append a new row (already with a larger size)
  vector <Transition> empty_row(size + 1, EMPTY_TRANS);
  trans_table.push_back(empty_row);

  // append a new column
  for (unsigned int i = 0; i < size; i++)
    trans_table[i].push_back(EMPTY_TRANS);

  size += 1;
}

// Returns true if repeat quantifier represents a string
bool
NFA::is_regex_string(ParseNode *node, int repeat_lower, int repeat_upper)
{
  // Conditions for a string:
  // - Must be a repeated character set node
  // - Must be a * or + meaning that lower is 0 or 1, upper is -1 (no limit)
  // - Character set node must contain sufficient selections for a string
  //
  if (node->type != CHAR_SET_NODE) return false;
  if (repeat_upper != -1) return false;
  if (repeat_lower != 0 && repeat_lower != 1) return false;
  if (!(node->char_set->is_string_candidate())) return false;

  return true;
}

//
// PRINT FUNCTION
//

// Print out the NFA
void
NFA::print()
{
  cout << "NFA: " << endl;
  cout << "Number of states: " << size << " ";
  cout << "Initial state: " << initial << " ";
  cout << "Final state: " << final << endl;
  
  cout << "Transition table: " << endl;
  for (unsigned int from = 0; from < size; from++) {
    cout << "State " << from << ": ";
    cout << endl;
    for (unsigned int to = 0; to < size; to++) {
      Transition trans = trans_table[from][to];
      if (trans.type != EMPTY) {
        cout << "  To state " << to << " on ";
	trans.print();
      }
    }
  }

  cout << endl;
}

// Starting function to generate all strings
vector <Path>
NFA::find_basis_paths()
{
  Path path;
  path.states.push_back(nfa.initial);

  vector <Path> paths;

  bool *visited = new bool[size];
  for (unsigned int i = 0; i < size; i++)
    visited[i] = false;

  traverse(nfa.initial, path, paths, visited);

  path_count = paths.size();
  return paths;
}

// Utility function to find all paths through the NFA.
void
NFA::traverse(unsigned int curr_state, Path path, vector <Path> &paths, bool *visited)
{
  // stop if you already have been here
  bool been_here = visited[curr_state];

  // final state --> process the path and stop the traversal
  if (curr_state == nfa.final) {
    for (unsigned int i = 1; i < path.size(); i++) {
      visited[curr_state] = true;
    }
    paths.push_back(path);
    return;
  }

  // for each adjacent state, find all paths 
  for (unsigned int next_state = 0; next_state < nfa.size; next_state++) {
    Transition trans = nfa.trans_table[curr_state][next_state];
    if (trans.type == EMPTY) continue;
    path.transition.push_back(trans);
    path.states.push_back(next_state);
    find_all_paths(next_state, path);
    path.transitions.pop_back();
    path.states.pop_back();
    if (been_here) break;
  }
}

//
// STAT FUNCTION
//

// Add NFA stats
void
NFA::add_stats(Stats &stats)
{
  int edge_count = 0;
  int epsilon_count = 0;

  for (unsigned int from = 0; from < size; from++) {
    for (unsigned int to = 0; to < size; to++) {
      Transition trans = trans_table[from][to];
      if (trans.type != EMPTY) {
        edge_count++;
	if (trans.type == EPSILON) epsilon_count++;
      }
    }
  }

  stats.add("NFA", "NFA states", size);
  stats.add("NFA", "NFA edges", edge_count);
  stats.add("NFA", "NFA epsilon edges", epsilon_count);
  stats.add("PATHS", "Paths", path_count);
}
