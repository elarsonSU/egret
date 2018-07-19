/*  NFA.cpp: Nondeterminstic Finite State Automaton

    Copyright (C) 2016-2018  Eric Larson and Anna Kirk
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

#include <cassert>
#include <iostream>
#include <vector>
#include "Edge.h"
#include "NFA.h"
#include "ParseTree.h"
#include "Util.h"
using namespace std;

// TODO: No location information for epsilon edge.  OK?
static Edge EPSILON = Edge(EPSILON_EDGE);

NFA::NFA(unsigned int _size, unsigned int _initial, unsigned int  _final)
{
  size = _size;
  initial = _initial;
  final = _final;

  assert(initial < size);
  assert(final < size);

  // initialize edge table with an "empty graph"
  vector <Edge *> empty_row(size, NULL);
  for (unsigned int i = 0; i < size; i++) {
    edge_table.push_back(empty_row);
  }
}

NFA::NFA(const NFA &other)
{
  size = other.size;
  initial = other.initial;
  final = other.final;
  edge_table = other.edge_table;
}

NFA &
NFA::operator=(const NFA & other)
{
  if (this == &other)
    return *this;

  initial = other.initial;
  final = other.final;
  size = other.size;
  edge_table = other.edge_table;

  return *this;
}

void
NFA::build(ParseTree &tree)
{
  // Build NFA
  NFA nfa = build_nfa_from_tree(tree.get_root());

  // Copy NFA
  initial = nfa.initial;
  final = nfa.final;
  size = nfa.size;
  edge_table = nfa.edge_table;
}

NFA
NFA::build_nfa_from_tree(ParseNode *tree)
{
  assert(tree);

  switch (tree->type) {

  case ALTERNATION_NODE:
    return build_nfa_alternation(tree);

  case CONCAT_NODE:
    return build_nfa_concat(tree);

  case REPEAT_NODE:
    return build_nfa_repeat(tree);

  case GROUP_NODE:
    return build_nfa_group(tree);

  case CHARACTER_NODE:
    return build_nfa_character(tree);

  case CARET_NODE:
    return build_nfa_caret(tree);

  case DOLLAR_NODE:
    return build_nfa_dollar(tree);

  case CHAR_SET_NODE:
    return build_nfa_char_set(tree);

  case IGNORED_NODE:
    return build_nfa_ignored(tree);

  case BACKREFERENCE_NODE:
    return build_nfa_backreference(tree);

  default:
    throw EgretException("ERROR (internal): Invalid node type in parse tree");
  }
}

NFA
NFA::build_nfa_alternation(ParseNode *tree)
{
  NFA nfa1 = build_nfa_from_tree(tree->left);
  NFA nfa2 = build_nfa_from_tree(tree->right);

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

  // Set new initial state and the edges from it
  new_nfa.add_edge(0, nfa1.initial, &EPSILON);
  new_nfa.add_edge(0, nfa2.initial, &EPSILON);
  new_nfa.initial = 0;

  // Make up space for the new final state
  new_nfa.append_empty_state();

  // Set new final state
  new_nfa.final = new_nfa.size - 1;
  new_nfa.add_edge(nfa1.final, new_nfa.final, &EPSILON);
  new_nfa.add_edge(nfa2.final, new_nfa.final, &EPSILON);

  return new_nfa;
}

NFA
NFA::build_nfa_concat(ParseNode *tree)
{
  NFA nfa1 = build_nfa_from_tree(tree->left);
  NFA nfa2 = build_nfa_from_tree(tree->right);
  return concat_nfa(nfa1, nfa2);
}

NFA
NFA::build_nfa_repeat(ParseNode *tree)
{
  int repeat_lower = tree->repeat_lower;
  int repeat_upper = tree->repeat_upper;

  // if repeat represents a string, build a regex string instead
  if (is_regex_string(tree->left, repeat_lower, repeat_upper))
    return build_nfa_string(tree);

  // create NFA for repeated segment
  NFA nfa = build_nfa_from_tree(tree->left);

  // make room for the new initial state
  nfa.shift_states(1);

  // make room for the new final state
  nfa.append_empty_state();

  // create new loop
  RegexLoop *regex_loop = new RegexLoop(repeat_lower, repeat_upper);

  // Util new edges
  Edge *edge = new Edge(BEGIN_LOOP_EDGE, tree->loc, regex_loop);
  nfa.add_edge(0, nfa.initial, edge);	   // new initial to old initial
  edge = new Edge(END_LOOP_EDGE, tree->loc, regex_loop);
  nfa.add_edge(nfa.final, nfa.size - 1, edge); // old final to new final

  // update states
  nfa.initial = 0;
  nfa.final = nfa.size - 1;

  return nfa;
}

NFA
NFA::build_nfa_string(ParseNode *tree)
{
  NFA nfa(2, 0, 1);
  RegexString *regex_str =
    new RegexString(tree->left->char_set, tree->repeat_lower, tree->repeat_upper);
  Location loc = make_pair(tree->left->loc.first, tree->loc.second);
  Edge *edge = new Edge(STRING_EDGE, loc, regex_str);
  nfa.add_edge(0, 1, edge);

  return nfa;
}

NFA
NFA::build_nfa_group(ParseNode *tree)
{
  return build_nfa_from_tree(tree->left);
}


NFA
NFA::build_nfa_character(ParseNode *tree)
{
  NFA nfa(2, 0, 1);	// size = 2, initial = 0 , final = 1
  Edge *edge = new Edge(CHARACTER_EDGE, tree->loc, tree->character);
  nfa.add_edge(0, 1, edge);
  return nfa;
}

NFA
NFA::build_nfa_caret(ParseNode *tree)
{
  NFA nfa(2, 0, 1);	// size = 2, initial = 0 , final = 1
  Edge *edge = new Edge(CARET_EDGE, tree->loc);
  nfa.add_edge(0, 1, edge);
  return nfa;
}

NFA
NFA::build_nfa_dollar(ParseNode *tree)
{
  NFA nfa(2, 0, 1);	// size = 2, initial = 0 , final = 1
  Edge *edge = new Edge(DOLLAR_EDGE, tree->loc);
  nfa.add_edge(0, 1, edge);
  return nfa;
}

NFA
NFA::build_nfa_char_set(ParseNode *tree)
{
  NFA nfa(2, 0, 1);     // size = 2, initial = 0, final = 1
  Edge *edge = new Edge(CHAR_SET_EDGE, tree->loc, tree->char_set);
  nfa.add_edge(0, 1, edge);
  return nfa;
}

NFA
NFA::build_nfa_ignored(ParseNode *tree)
{
  NFA nfa(2, 0, 1);	// size = 2, initial = 0 , final = 1
  nfa.add_edge(0, 1, &EPSILON);
  return nfa;
}

NFA
NFA::build_nfa_backreference(ParseNode *tree)
{
  NFA nfa(2, 0, 1);     // size = 2, initial = 0, final = 1
  Edge *edge = new Edge(BACKREFERENCE_EDGE, tree->loc, tree->backref);
  nfa.add_edge(0, 1, edge);
  return nfa;
}

void
NFA::add_edge(unsigned int from, unsigned int to, Edge *edge)
{
  assert(from < size);
  assert(to < size);

  edge_table[from][to] = edge;
}

NFA
NFA::concat_nfa(NFA nfa1, NFA nfa2)
{
  // How this is done: First will come nfa1, then nfa2 (its
  // initial state replaced with nfa1's final state)

  // make room for nfa1
  nfa2.shift_states(nfa1.size);

  // create a new nfa and initialize it with (the shifted) nfa2
  NFA new_nfa(nfa2);

  // nfa1's states take their places in new_nfa
  new_nfa.fill_states(nfa1);

  // add edge from nfa1 to nfa2
  new_nfa.add_edge(nfa1.final, new_nfa.initial, &EPSILON);

  // set the new initial state (the final state stays nfa2's final state,
  // and was already copied)
  new_nfa.initial = nfa1.initial;

  return new_nfa;
}

void
NFA::shift_states(unsigned int shift)
{
  unsigned int new_size = size + shift;

  if (shift < 1) return;

  // create a new, empty edge table (of the new size)
  vector <Edge *> empty_row(new_size, NULL);
  vector <vector <Edge *> > new_edge_table(new_size, empty_row);

  // copy all the edges to the new table, at their new locations
  for (unsigned int i = 0; i < size; i++) {
    for (unsigned int j = 0; j < size; j++) {
      new_edge_table[i + shift][j + shift] = edge_table[i][j];
    }
  }

  // update the NFA members
  size = new_size;
  initial += shift;
  final += shift;
  edge_table = new_edge_table;
}

// fills states from other's states
// (requires the use of shift_states first)
void
NFA::fill_states(const NFA &other)
{
  for (unsigned int i = 0; i < other.size; i++) {
    for (unsigned int j = 0; j < other.size; j++) {
      edge_table[i][j] = other.edge_table[i][j];
    }
  }
}

void
NFA::append_empty_state()
{
  // append a new row (already with a larger size)
  vector <Edge *> empty_row(size + 1, NULL);
  edge_table.push_back(empty_row);

  // append a new column
  for (unsigned int i = 0; i < size; i++)
    edge_table[i].push_back(NULL);

  size += 1;
}

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

vector <Path>
NFA::find_basis_paths()
{
  Path path(initial);
  vector <Path> paths;
  bool *visited = new bool[size];
  for (unsigned int i = 0; i < size; i++)
    visited[i] = false;

  traverse(initial, path, paths, visited);

  delete visited;

  return paths;
}

void
NFA::traverse(unsigned int curr_state, Path path, vector <Path> &paths,
    bool *visited)
{
  // stop if you already have been here
  bool been_here = visited[curr_state];

  // final state --> process the path and stop the traversal
  if (curr_state == final) {
    path.mark_path_visited(visited);
    paths.push_back(path);
    return;
  }

  // for each adjacent state, find all paths 
  for (unsigned int next_state = 0; next_state < size; next_state++) {
    Edge *edge = edge_table[curr_state][next_state];
    if (edge == NULL) continue;
    path.append(edge, next_state);
    traverse(next_state, path, paths, visited);
    path.remove_last();
    if (been_here) break;
  }
}

void
NFA::print()
{
  cout << "NFA: " << endl;
  cout << "Number of states: " << size << " ";
  cout << "Initial state: " << initial << " ";
  cout << "Final state: " << final << endl;
  
  cout << "Edge table: " << endl;
  for (unsigned int from = 0; from < size; from++) {
    cout << "State " << from << ": ";
    cout << endl;
    for (unsigned int to = 0; to < size; to++) {
      Edge *edge = edge_table[from][to];
      if (edge != NULL) {
        cout << "  To state " << to << " on ";
	edge->print();
      }
    }
  }

  cout << endl;
}

void
NFA::add_stats(Stats &stats)
{
  int edge_count = 0;
  int char_count = 0;
  int charset_count = 0;
  int string_count = 0;
  int begin_loop_count = 0;
  int end_loop_count = 0;
  int caret_count = 0;
  int dollar_count = 0;
  int backreference_count = 0;
  int epsilon_count = 0;

  for (unsigned int from = 0; from < size; from++) {
    for (unsigned int to = 0; to < size; to++) {
      Edge *edge = edge_table[from][to];
      if (edge != NULL) {
        edge_count++;
	switch (edge->get_type()) {
	  case CHARACTER_EDGE:
	    char_count++;
	    break;
	  case CHAR_SET_EDGE:
	    charset_count++;
	    break;
	  case STRING_EDGE:
	    string_count++;
	    break;
	  case BEGIN_LOOP_EDGE:
	    begin_loop_count++;
	    break;
	  case END_LOOP_EDGE:
	    end_loop_count++;
	    break;
	  case CARET_EDGE:
	    caret_count++;
	    break;
	  case DOLLAR_EDGE:
	    dollar_count++;
	    break;
	  case BACKREFERENCE_EDGE:
	    backreference_count++;
	    break;
	  case EPSILON_EDGE:
	    epsilon_count++;
	    break;
	}
      }
    }
  }

  stats.add("NFA", "NFA states", size);
  stats.add("NFA", "NFA edges", edge_count);
  stats.add("NFA", "NFA character edges", char_count);
  stats.add("NFA", "NFA char set edges", charset_count);
  stats.add("NFA", "NFA string edges", string_count);
  stats.add("NFA", "NFA begin loop edges", begin_loop_count);
  stats.add("NFA", "NFA end loop edges", end_loop_count);
  stats.add("NFA", "NFA caret edges", caret_count);
  stats.add("NFA", "NFA dollar edges", dollar_count);
  stats.add("NFA", "NFA backreference edges", backreference_count);
  stats.add("NFA", "NFA epsilon edges", epsilon_count);
}
