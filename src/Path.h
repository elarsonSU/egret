/*  Path.h: Represents a path through the NFA

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

#ifndef PATH_H
#define PATH_H

#include <set>
#include <string>
#include <vector>
#include "Edge.h"
#include "TestString.h"
using namespace std;

class Path {

public:

  Path() {}
  Path(unsigned int initial) { states.push_back(initial); }
  TestString get_test_string() { return test_string; }

  // PATH CONSTRUCTION FUNCTIONS

  // adds an edge and the destination state to the path 
  void append(Edge *edge, unsigned int state);

  // removes the last edge and state
  void remove_last();

  // marks the states in the path as visited
  void mark_path_visited(bool *visited);

  // processes path: sets test string and evil edges
  void process_path(TestString base_substring); 

  // CHECKER FUNCTIONS

  // returns true if path has a leading caret
  bool has_leading_caret();

  // returns true if path has a trailing dollar
  bool has_trailing_dollar();

  // returns true and emits warning if there is an anchor (^ or $) in the
  // middle of the path 
  bool check_anchor_in_middle();

  // returns true and emits warning if a path contains duplicate character sets
  bool check_charsets();

  // TEST STRING GENERATION FUNCTIONS

  // generates a string with minimum iterations for repeating constructs
  TestString gen_min_iter_string();

  // generates evil strings for the path
  vector <TestString> gen_evil_strings(const set <char> &punct_marks);

  // PRINT FUNCTION
  
  // prints the path
  void print();

private:

  vector <unsigned int> states;		// list of states
  vector <Edge *> edges;		// list of edges
  TestString test_string;		// test string associated with path
  vector <unsigned int> evil_edges;	// list of evil edges that need processing
};

#endif // PATH_H
