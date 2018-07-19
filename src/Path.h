/*  Path.h: Represents a path through the NFA

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
  string get_test_string() { return test_string; }

  // PATH CONSTRUCTION FUNCTIONS

  // adds an edge and the destination state to the path 
  void append(Edge *edge, unsigned int state);

  // removes the last edge and state
  void remove_last();

  // marks the states in the path as visited
  void mark_path_visited(bool *visited);

  // processes path: sets test string and evil edges
  void process_path();

  // CHECKER FUNCTIONS

  // returns true if path has a leading caret
  bool has_leading_caret();

  // returns true if path has a trailing dollar
  bool has_trailing_dollar();

  // returns true and emits violation if there is an anchor (^ or $) in the
  // middle of the path 
  bool check_anchor_in_middle();

  // emits violation if a path contains a charset error
  void check_charsets();

  // emits violation if a path contains optional braces
  void check_optional_braces();

  // emits violation if wildcard is just before/after punctuation mark
  void check_wild_punctuation();

  // emits violation if punctuation can be repeated in certain situations
  void check_repeat_punctuation();

  // emits violation if digits are too optional
  void check_digit_too_optional();

  // STRING GENERATION FUNCTIONS

  // generates example string 
  string gen_example_string(Location loc, char c);
  string gen_example_string(Location loc, char c, char except);
  string gen_example_string(Location loc, char c, Location omit);
  string gen_example_string(Location loc1, char c1, Location loc2, char c2);
  string gen_example_string(Location loc, string replace);

  // generates string based on location
  string gen_backref_string(Location loc);

  // generates a string with minimum iterations for repeating constructs
  string gen_min_iter_string();

  // generates evil strings for the path
  vector <string> gen_evil_strings(const set <char> &punct_marks);

  // PRINT FUNCTION
  
  // prints the path
  void print();

private:

  vector <unsigned int> states;		// list of states
  vector <Edge *> edges;		// list of edges
  string test_string;		// test string associated with path
  vector <unsigned int> evil_edges;	// list of evil edges that need processing

};

#endif // PATH_H
