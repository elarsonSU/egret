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

// coonstants
static const State DEFAULT_STATE = { false, false, false };
static const Transition EPSILON_TRANS = { EPSILON, '\0' };
static const Transition EMPTY_TRANS = { EMPTY };
static const unsigned int PATH_LIMIT = 10000;

// constructor
NFA::NFA(unsigned int _size, unsigned int _initial, unsigned int  _final)
{
  size = _size;
  initial = _initial;
  final = _final;

  assert(initial < size);
  assert(final < size);

  // Initialize trans_table with an "empty graph" (no transitions) and
  // the states list to all normal states
  vector <Transition> empty_row(size, EMPTY_TRANS);
  for (unsigned int i = 0; i < size; i++) {
    trans_table.push_back(empty_row);
    states.push_back(DEFAULT_STATE);
  }
}

// copy constructor
NFA::NFA(const NFA &other)
{
  size = other.size;
  initial = other.initial;
  final = other.final;
  trans_table = other.trans_table;
  states = other.states;
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
  states = other.states;

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
  states = nfa.states;
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
  nfa.states[nfa.initial].begin_state = true;
  nfa.states[nfa.final].end_repeat_state = true;
  nfa.states[nfa.final].begin_index = nfa.initial;
  nfa.states[nfa.final].repeat_lower = repeat_lower;
  nfa.states[nfa.final].repeat_upper = repeat_upper;

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
  trans.character = node->char_set->find_good_character(punct_marks);
  trans.char_set = node->char_set;
  nfa.add_transition(0, 1, trans);

  nfa.states[0].begin_state = true;
  nfa.states[1].end_charset_state  = true;
  nfa.states[1].begin_index = 0;
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
  trans.character = '\0';
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
  trans.character = '\0';
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
  trans.character = char_set->find_good_character(punct_marks);
  trans.char_set = char_set;
  nfa.add_transition(0, 1, trans);

  nfa.states[0].begin_state = true;
  nfa.states[1].end_charset_state  = true;
  nfa.states[1].begin_index = 0;
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

  // update indices in the states list
  vector <State>::iterator it;
  for (it = states.begin(); it != states.end(); it++) {
    if (it->end_repeat_state || it->end_charset_state) {
      it->begin_index += shift;
    }
  }

  // update the NFA members
  size = new_size;
  initial += shift;
  final += shift;
  trans_table = new_trans_table;
  states.insert(states.begin(), shift, DEFAULT_STATE);
}

// Fills states 0 up to other.size with other's states
// Requires the use of shift_states first.
void
NFA::fill_states(const NFA &other)
{
  for (unsigned int i = 0; i < other.size; i++) {
    for (unsigned int j = 0; j < other.size; j++) {
      trans_table[i][j] = other.trans_table[i][j];
    }
    states[i] = other.states[i];
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

  // append a new state
  states.push_back(DEFAULT_STATE);

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
// NFA TRAVERSAL FUNCTIONS
//

// Starting function to find all paths (with limitations on cycles) through NFA
vector <string>
NFA::nfa_traverse()
{
  vector <unsigned int> path;
  path.push_back(initial);
  string path_string = "";

  // Initialize variables
  for (unsigned int i = 0; i < size; i++)
    states[i].visited = false;
  all_start_with_caret = false;
  all_end_with_dollar = false;
  warn_caret_middle = false;
  warn_dollar_middle = false;
  warn_caret_start = false;
  warn_dollar_end = false;

  find_all_paths(initial, path);
  init_test_strings();
  visit_end_nodes();
  return test_strings;
}

// Utility function to find all paths through the NFA.
void
NFA::find_all_paths(unsigned int curr_state, vector <unsigned int> path)
{
  // stop if you already have been here
  bool been_here = states[curr_state].visited;

  // final state --> process the path and stop the traversal
  if (curr_state == final) {
    process_path(path);
    return;
  }

  // for each adjacent state, find all paths 
  for (unsigned int next_state = 0; next_state < size; next_state++) {
    Transition trans = trans_table[curr_state][next_state];
    if (trans.type == EMPTY) continue;

    path.push_back(next_state);
    find_all_paths(next_state, path);
    path.pop_back();
    if (been_here) break;
  }
}

// Determines if the path should be added to the list of paths.  If so, a path
// string is generated for that path.
void
NFA::process_path(vector <unsigned int> path)
{
  // local variables for carets and dollars
  bool start_with_caret = false;
  bool end_with_dollar = false;
  bool caret_in_middle = false;
  bool dollar_in_middle = false;
  unsigned int first_char_state = final;
  unsigned int last_char_state = initial;
  unsigned int caret_index;
  unsigned int dollar_index;

  // check for leading caret(s)
  for (unsigned int i = 1; i < path.size(); i++) {
    int from = path[i-1];
    int to = path[i];
    Transition trans = trans_table[from][to];
    if (trans.type == CARET_INPUT) {
      start_with_caret = true;
    }
    else if (trans.type != EPSILON) {
      first_char_state = to;
      break;
    }
  }

  // check for trailing dollar(s)
  for (unsigned int i = path.size() - 1; i > 0; i--) {
    int from = path[i-1];
    int to = path[i];
    Transition trans = trans_table[from][to];
    if (trans.type == DOLLAR_INPUT) {
      end_with_dollar = true;
    }
    else if (trans.type != EPSILON) {
      last_char_state = from;
      break;
    }
  }

  // for first path, record whether the path starts with ^ and/or ends with $
  if (paths.size() == 0) {
    all_start_with_caret = start_with_caret;
    all_end_with_dollar = end_with_dollar;
  }

  // go through each state in the path
  string path_string = "";
  map <unsigned int, string> prefix_map;
  for (unsigned int i = 1; i < path.size(); i++) {
    unsigned int prev_state = path[i-1];
    unsigned int curr_state = path[i];

    // check for caret and dollar transitions
    if (trans_table[prev_state][curr_state].type == CARET_INPUT &&
	curr_state > first_char_state)
    {
      caret_in_middle = true;
      caret_index = path_string.length();
    }
    if (trans_table[prev_state][curr_state].type == DOLLAR_INPUT &&
	prev_state < last_char_state)
    {
      dollar_in_middle = true;
      dollar_index = path_string.length();
    }

    // add character to path string
    char c = trans_table[prev_state][curr_state].character;
    if (c != '\0') path_string += c;
      
    // TODO: Check if this can be simplified.

    // add to prefix map for begin states
    if (states[curr_state].begin_state) {
      prefix_map[curr_state] = path_string;
    }

    // extract the path element for end states
    string path_element;
    if (states[curr_state].end_repeat_state) {
      int begin_index = states[curr_state].begin_index;
      size_t begin_len = prefix_map[begin_index].length();
      path_element = path_string.substr(begin_len);

      for (int i = 2; i <= states[curr_state].repeat_lower; i++) {
        path_string += path_element;
      }
    }

    // if it is not visited, update this state with information from this path
    if (!states[curr_state].visited) {
      states[curr_state].visited = true;
      states[curr_state].path_index = paths.size();
      states[curr_state].path_prefix = path_string;
      if (states[curr_state].end_repeat_state || states[curr_state].end_charset_state)
        end_states.push_back(curr_state);
      if (states[curr_state].end_repeat_state)
        states[curr_state].path_element = path_element;
    }
  }

  // add path and path string to respective list
  paths.push_back(path);
  path_strings.push_back(path_string);
	  
  // Process ^ and $ warnings
  if (!warn_caret_middle && caret_in_middle) {
    stringstream s;
    s << "Generated string has ^ anchor in middle\n";
    s << "...Characters before ^ anchor: " << path_string.substr(0, caret_index) << "\n";
    s << "...Characters after ^ anchor:  " << path_string.substr(caret_index);
    addWarning(s.str());
    warn_caret_middle = true;
  }
  if (!warn_dollar_middle && dollar_in_middle) {
    stringstream s;
    s << "Generated string has $ anchor in middle\n";
    s << "...Characters before $ anchor: " << path_string.substr(0, dollar_index) << "\n";
    s << "...Characters after $ anchor:  " << path_string.substr(dollar_index);
    addWarning(s.str());
    warn_dollar_middle = true;
  }
  if (!warn_caret_start) {
    if (all_start_with_caret && !start_with_caret) {
      stringstream s;
      s << "Some but not all strings start with a ^ anchor\n";
      s << "...String with ^ anchor:    " << path_strings[0] << "\n";
      s << "...String with no ^ anchor: " << path_string << endl;
      addWarning(s.str());
      warn_caret_start = true;
    }
    if (!all_start_with_caret && start_with_caret) {
      stringstream s;
      s << "Some but not all strings start with a ^ anchor\n";
      s << "...String with ^ anchor:    " << path_string << "\n";
      s << "...String with no ^ anchor: " << path_strings[0] << endl;
      addWarning(s.str());
      warn_caret_start = true;
    }
  }
  if (!warn_dollar_end) {
    if (all_end_with_dollar && !end_with_dollar) {
      stringstream s;
      s << "Some but not all strings end with a $ anchor\n";
      s << "...String with $ anchor:    " << path_strings[0] << "\n";
      s << "...String with no $ anchor: " << path_string << endl;
      addWarning(s.str());
      warn_dollar_end = true;
    }
    if (!all_end_with_dollar && end_with_dollar) {
      stringstream s;
      s << "Some but not all strings end with a $ anchor\n";
      s << "...String with $ anchor:    " << path_string << "\n";
      s << "...String with no $ anchor: " << path_strings[0];
      addWarning(s.str());
      warn_dollar_end = true;
    }
  }

  return;
}

// Initialize set of strings by copying path strings, removing duplicates.
void
NFA::init_test_strings()
{
  vector <string>::iterator it;
  for (it = path_strings.begin(); it != path_strings.end(); it++) {
    add_to_test_strings(*it);
  }
}
  
// Adds a string to test string vector (unless it is already there).
void
NFA::add_to_test_strings(string s)
{
  if (find(test_strings.begin(), test_strings.end(), s) == test_strings.end()) {
    test_strings.push_back(s);
  }
}

// Generate additional strings by visiting end nodes.
void
NFA::visit_end_nodes()
{
  vector <unsigned int>::iterator it;
  for (it = end_states.begin(); it != end_states.end(); it++) {
    State state = states[*it];
    if (state.end_repeat_state) {
      if (state.repeat_upper != -1) {

        // For cases like {n}, add strings for one less (n-1) and one more (n+1).
        if (state.repeat_lower == state.repeat_upper) {
	  process_test_string_loop(state);
	}
	else {
	  // Handle one less on lower bound (note if lower bound is zero, the path
	  // has one iteration so one less iteration will get us to zero iterations)
	  process_test_string_loop(state, false, true);

	  // Extract the path elements.
          string orig_path = path_strings[state.path_index];
          string before_path = states[state.begin_index].path_prefix;
          string after_path = orig_path.substr(before_path.length());

	  // Add enough path elements to get to the upper bound (note if lower bound
	  // is zero, the path has one iteration so the starting point is bumped to one).
	  int base_iterations = state.repeat_lower;
	  if (base_iterations == 0) base_iterations = 1;
	  string path_elements;
	  for (int i = base_iterations; i < state.repeat_upper; i++) {
	    path_elements += state.path_element;
	  }

	  // Add the upper bound string.
	  string upper_bound_string = before_path + path_elements + after_path;
	  add_to_test_strings(upper_bound_string);

	  // Add the string with one more iteration past the upper bound.
	  string past_bound_string = before_path + path_elements + state.path_element + after_path;
	  add_to_test_strings(past_bound_string);
	}
      }
      else {	// state.repeat_upper == -1 (no limit)
	// If lower bound is 0 or 1, add one less (zero) and add one more (two).  Want
	// to have one case that has repeated (two) elements.
	if (state.repeat_lower == 0 || state.repeat_lower == 1) {
	process_test_string_loop(state);
	}
	// Otherwise, only add the string with one less iteration than the lower bound.
	else {
	  process_test_string_loop(state, false, true);
	}
      }
    }
    else if (state.end_charset_state) {
      Transition trans = trans_table[state.begin_index][*it];
      assert(trans.type == CHAR_SET_INPUT || trans.type == STRING_INPUT);

      string orig_path = path_strings[state.path_index];
      string path_prefix = states[state.begin_index].path_prefix;
      string path_suffix = orig_path.substr(path_prefix.length() + 1);

      set<char> test_chars = trans.char_set->create_test_chars(punct_marks);
      set<char>::iterator cs;
      for (cs = test_chars.begin(); cs != test_chars.end(); cs++) {
        string new_path;
        new_path = path_prefix + *cs + path_suffix;
        add_to_test_strings(new_path);
      }
    }
  }
}

// Process loop constructs by adding one less and one more iteration.
void
NFA::process_test_string_loop(State state, bool skip_one_less, bool skip_one_more)
{
  string orig_path = path_strings[state.path_index];
  string path_prefix = states[state.begin_index].path_prefix;
  string path_element = state.path_element;
  string path_suffix = orig_path.substr(path_prefix.length() + path_element.length());
	  
  if (!skip_one_less) {
    string one_less_string = path_prefix + path_suffix;
    add_to_test_strings(one_less_string);
  }

  if (!skip_one_more) {
    string one_more_string = path_prefix + path_element + path_element + path_suffix; 
    add_to_test_strings(one_more_string);
  }
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
    states[from].print();
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
  stats.add("PATHS", "Paths", paths.size());
  stats.add("PATHS", "Strings", test_strings.size());
}

