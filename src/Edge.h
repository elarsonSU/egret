/*  Edge.h: an edge in an NFA and a path 

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

#ifndef EDGE_H
#define EDGE_H

#include <set>
#include <string>
#include "CharSet.h"
#include "RegexString.h"
#include "RegexLoop.h"
using namespace std;

typedef enum {
  CHARACTER_EDGE,
  CHAR_SET_EDGE,
  STRING_EDGE,
  BEGIN_LOOP_EDGE,
  END_LOOP_EDGE,
  CARET_EDGE,
  DOLLAR_EDGE,
  EPSILON_EDGE
} EdgeType;

class Edge {

public:

  Edge() { processed = false; }
  Edge(EdgeType t) { type = t; processed = false; }
  Edge(EdgeType t, char c) { type = t; character = c; processed = false; }
  Edge(EdgeType t, CharSet *c) { type = t; char_set = c; processed = false; }
  Edge(EdgeType t, RegexString *r) { type = t; regex_str = r; processed = false; }
  Edge(EdgeType t, RegexLoop *r) { type = t; regex_loop = r; processed = false; }

  EdgeType getType() { return type; }

  // get valid substring associated with edge
  string get_substring();

  // perform path processing on the edge, returns true if edge should be used in
  // creating evil strings
  bool process_edge_in_path(string path_prefix, string base_substring);

  // generate evil strings
  set <string> gen_evil_strings(string path_string, const set <char> &punct_marks);

  // print the edge
  void print();

private:
  EdgeType type;		// type of edge
  bool processed;		// set if processed in a path
  char character;		// character (for CHARACTER_EDGE)
  CharSet *char_set;		// character set (for CHAR_SET_EDGE)
  RegexString *regex_str;	// regex string (for STRING_EDGE)
  RegexLoop *regex_loop;	// regex loop (for BEGIN_LOOP_EDGE and END_LOOP_EDGE)
};

#endif // EDGE_H
