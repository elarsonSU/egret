/*  State.h: a state in an NFA and a path

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

#ifndef STATE_H
#define STATE_H

#include <set>
using namespace std;

struct State
{
  bool begin_state;		// Set if state marks the beginning of repeat quantifier
 				// or character set
  bool end_repeat_state;	// Set if state marks the end of a repeating quantifier
  bool end_charset_state;	// Set if state marks the end of a character set

  int begin_index;		// Corresponding begin state index for end states
  int repeat_lower;     	// Lower bound for repeat quantifiers 
  int repeat_upper;     	// Upper bound for repeat quantifiers (-1 if no bound)

  // Traversal information
  bool visited;			// set when visited
  int path_index;		// path that contains this state
  string path_prefix;		// path string up to visiting this node (for paths[path_index])
  string path_element;		// substring corresponding to the RE element (for END states only)

  // Prints the state.
  void print();
};

#endif // STATE

