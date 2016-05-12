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
#include "TestGenerator.h"
#include "Transition.h"
#include "ParseTree.h"
#include "error.h"

using namespace std;

// Starting function to generate all strings
vector <string>
TestGenerator::gen_test_strings()
{
  // Find all the paths
  visited = new bool[nfa.size];
  for (unsigned int i = 0; i < nfa.size; i++)
    visited[i] = false;

  vector <unsigned int> path;
  path.push_back(nfa.initial);
  find_all_paths(nfa.initial, path);

  gen_initial_strings();

  // process_regex_loops();
  // process_regex_strings();
  // process_char_sets();

  // Initialize variables

  return test_strings;
}

// Utility function to find all paths through the NFA.
void
TestGenerator::find_all_paths(unsigned int curr_state, vector <unsigned int> path)
{
  // stop if you already have been here
  bool been_here = visited[curr_state];

  // final state --> process the path and stop the traversal
  if (curr_state == nfa.final) {
    for (unsigned int i = 1; i < path.size(); i++) {
      visited[curr_state] = true;
    }
    return;
  }

  // for each adjacent state, find all paths 
  for (unsigned int next_state = 0; next_state < nfa.size; next_state++) {
    Transition trans = nfa.trans_table[curr_state][next_state];
    if (trans.type == EMPTY) continue;

    path.push_back(next_state);
    find_all_paths(next_state, path);
    path.pop_back();
    if (been_here) break;
  }
}

// Generates initial strings
void
TestGenerator::gen_initial_strings(vector <unsigned int> path)
{
  bool all_start_with_caret = false;
  bool all_end_with_dollar = false;
  bool warn_caret_middle = false;
  bool warn_dollar_middle = false;
  bool warn_caret_start = false;
  bool warn_dollar_end = false;

  vector <vector <unsigned int> >::iterator it;
  bool first_path = true;
  for (it = paths.begin(); it != paths.end(); it++) {

    // local variables for carets and dollars
    bool start_with_caret = false;
    bool end_with_dollar = false;
    bool caret_in_middle = false;
    bool dollar_in_middle = false;
    unsigned int first_char_state = nfa.final;
    unsigned int last_char_state = nfa.initial;
    unsigned int caret_index;
    unsigned int dollar_index;

    // check for leading caret(s)
    for (unsigned int i = 1; i < path.size(); i++) {
      int from = path[i-1];
      int to = path[i];
      Transition trans = nfa.trans_table[from][to];
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
      Transition trans = nfa.trans_table[from][to];
      if (trans.type == DOLLAR_INPUT) {
        end_with_dollar = true;
      }
      else if (trans.type != EPSILON) {
        last_char_state = from;
        break;
      }
    }

    // for first path, record whether the path starts with ^ and/or ends with $
    if (first_path) {
      all_start_with_caret = start_with_caret;
      all_end_with_dollar = end_with_dollar;
    }
    else {
      first_path = false;
    }

    // go through each state in the path
    string path_string = "";
    map <unsigned int, string> prefix_map;
    for (unsigned int i = 1; i < path.size(); i++) {
      unsigned int prev_state = path[i-1];
      unsigned int curr_state = path[i];

      // check for caret and dollar transitions
      if (nfa.trans_table[prev_state][curr_state].type == CARET_INPUT &&
	  curr_state > first_char_state)
      {
        caret_in_middle = true;
        caret_index = path_string.length();
      }
      if (nfa.trans_table[prev_state][curr_state].type == DOLLAR_INPUT &&
	  prev_state < last_char_state)
      {
        dollar_in_middle = true;
        dollar_index = path_string.length();
      }

      // add character to path string
      string substring = nfa.trans_table[prev_state][curr_state].get_substring();
      path_string += substring;
      
#if 0
      // add to prefix map for begin states
      if (nfa.states[curr_state].begin_state) {
        prefix_map[curr_state] = path_string;
      }

      // extract the path element for end states
      string path_element;
      if (nfa.states[curr_state].end_repeat_state) {
        int begin_index = nfa.states[curr_state].begin_index;
        size_t begin_len = prefix_map[begin_index].length();
        path_element = path_string.substr(begin_len);

        for (int i = 2; i <= nfa.states[curr_state].repeat_lower; i++) {
          path_string += path_element;
        }
      }

      // if it is not visited, update this state with information from this path
      if (!nfa.states[curr_state].visited) {
        nfa.states[curr_state].path_index = paths.size();
        nfa.states[curr_state].path_prefix = path_string;
        if (nfa.states[curr_state].end_repeat_state || nfa.states[curr_state].end_charset_state)
          end_states.push_back(curr_state);
        if (nfa.states[curr_state].end_repeat_state)
          nfa.states[curr_state].path_element = path_element;
      }
#endif
    }

    // add path string to list
    add_to_test_string(path_string);
	  
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
  }
}

// Adds a string to test string vector (unless it is already there).
void
TestGenerator::add_to_test_strings(string s)
{
  if (find(test_strings.begin(), test_strings.end(), s) == test_strings.end()) {
    test_strings.push_back(s);
  }
}

#if 0
// Generate additional strings by visiting end nodes.
void
TestGenerator::visit_end_nodes()
{
  vector <unsigned int>::iterator it;
  for (it = end_states.begin(); it != end_states.end(); it++) {
    State state = nfa.states[*it];
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
          string before_path = nfa.states[state.begin_index].path_prefix;
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
      Transition trans = nfa.trans_table[state.begin_index][*it];
      assert(trans.type == CHAR_SET_INPUT || trans.type == STRING_INPUT);

      string orig_path = path_strings[state.path_index];
      string path_prefix = nfa.states[state.begin_index].path_prefix;
      string path_suffix = orig_path.substr(path_prefix.length() + 1);

      set<char> test_chars = trans.char_set->create_test_chars(nfa.punct_marks);
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
TestGenerator::process_test_string_loop(State state, bool skip_one_less, bool skip_one_more)
{
  string orig_path = path_strings[state.path_index];
  string path_prefix = nfa.states[state.begin_index].path_prefix;
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
#endif

//
// STAT FUNCTION
//

// Add tests generation stats
void
TestGenerator::add_stats(Stats &stats)
{
  stats.add("PATHS", "Paths", paths.size());
  stats.add("PATHS", "Strings", test_strings.size());
}


