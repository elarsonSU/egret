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
using namespace std;

class Path {

public:

  Path() {}
  Path(unsigned int initial) { states.push_back(initial); }

  // adds an edge and the destination state to the path 
  void append(Edge *edge, unsigned int state);

  // removes the last edge and state
  void remove_last();

  // marks the states in the path as visited
  void mark_path_visited(bool *visited);

  // generates the initial test string for the path
  string gen_initial_string(string base_substring);

  // generates a string with minimum iterations for repeating constructs
  string gen_min_iter_string();

  // returns true if path has a leading caret
  bool has_leading_caret();

  // returns true if path has a trailing dollar
  bool has_trailing_dollar();

  // returns an error message if there is an anchor (^ or $) in the
  // middle of the path, returns an empty string otherwise
  string check_anchor_middle();

  // generates evil strings for the path
  set <string> gen_evil_strings(const set <char> &punct_marks);

private:

  vector <unsigned int> states;		// list of states
  vector <Edge *> edges;		// list of edges
  vector <unsigned int> evil_edges;	// list of evil edges that need processing
  string path_string;			// test string associated with path
};

#endif // PATH_H
