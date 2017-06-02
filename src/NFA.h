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
#include "Edge.h"
#include "CharSet.h"
#include "ParseTree.h"
#include "Path.h"
#include "Stats.h"
using namespace std;

class NFA {

public:

  NFA() {}
  NFA(unsigned int _size, unsigned int _initial, unsigned int _final);
  NFA(const NFA &other);
  NFA &operator= (const NFA &other);

  // build an NFA from the parse tree
  void build(ParseTree &tree);

  // create a set of basis paths
  vector <Path> find_basis_paths();

  // print out the NFA
  void print();

  // add NFA stats
  void add_stats(Stats &stats);

private:

  unsigned int size;			// number of states
  unsigned int initial;			// initial state
  unsigned int final;			// final state
  vector <vector <Edge *> > edge_table;	// edge table
  
  // builds an NFA from tree
  NFA build_nfa_from_tree(ParseNode *tree);

  // builds an alternation of nfa1 and nfa2 (nfa1|nfa2)
  NFA build_nfa_alternation(NFA nfa1, NFA nfa2);

  // builds a concatenation of nfa1 and nfa2 (nfa1nfa2)
  NFA build_nfa_concat (NFA nfa1, NFA nfa2);

  // builds nfa{m,n}
  NFA build_nfa_repeat(NFA nfa, int repeat_lower, int repeat_upper);

  // builds special node for regex strings such as .+ or \w*
  NFA build_nfa_string(ParseNode *tree, int repeat_lower, int repeat_upper);

  // builds (nfa)
  NFA build_nfa_group(NFA nfa, string name, int num);

  // builds nfa with character
  NFA build_nfa_character(char character);

  // builds nfa with caret
  NFA build_nfa_caret();

  // builds nfa with dollar
  NFA build_nfa_dollar();

  // builds nfa with backreference
  NFA build_nfa_backreference(string name, int num, int id);

  // builds nfa with ignored element
  NFA build_nfa_ignored();

  // builds nfa with char set as input
  NFA build_nfa_char_set(CharSet *char_set);

  // adds an edge to edge table
  void add_edge(unsigned int from, unsigned int to, Edge *edge);

  // shift (renames) all the states in the NFA according to some (positive) shift factor
  void shift_states(unsigned int shift);

  // fills states from other's states
  void fill_states(const NFA &other);

  // appends a new empty state to the NFA
  void append_empty_state();

  // returns true if repeat quantifier represents a string
  bool is_regex_string(ParseNode *node, int repeat_lower, int repeat_upper);

  // utility function to find all paths through the NFA
  void traverse(unsigned int curr_state, Path path, vector <Path> &paths, bool *visited);
};

#endif // NFA_H

