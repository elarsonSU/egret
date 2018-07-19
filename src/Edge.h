/*  Edge.h: an edge in an NFA and a path 

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

#ifndef EDGE_H
#define EDGE_H

#include <set>
#include <string>
#include "Backref.h"
#include "CharSet.h"
#include "RegexLoop.h"
#include "RegexString.h"
#include "Util.h"
using namespace std;

class Path;     // used to resolve a circular dependency

typedef enum {
  CHARACTER_EDGE,
  CHAR_SET_EDGE,
  STRING_EDGE,
  BEGIN_LOOP_EDGE,
  END_LOOP_EDGE,
  CARET_EDGE,
  DOLLAR_EDGE,
  BACKREFERENCE_EDGE,
  EPSILON_EDGE
} EdgeType;

class Edge {

public:

  // constructors
  Edge() { processed = false; }
  Edge(EdgeType t) { type = t; loc = make_pair(-1, -1); processed = false; }
  Edge(EdgeType t, Location l) { type = t; loc = l; processed = false; }
  Edge(EdgeType t, Location l, char c) { type = t; loc = l; character = c; processed = false; }
  Edge(EdgeType t, Location l, CharSet *c) { type = t; loc = l; char_set = c; processed = false; }
  Edge(EdgeType t, Location l, RegexString *r) { type = t; loc = l; regex_str = r; processed = false; }
  Edge(EdgeType t, Location l, RegexLoop *r) { type = t; loc = l; regex_loop = r; processed = false; }
  Edge(EdgeType t, Location l, Backref *b) { type = t; loc = l; backref = b; processed = false; }

  // accessors
  EdgeType get_type() 		{ return type; }
  Location get_loc() 		{ return loc; }
  char get_character()          { return character; }
  CharSet *get_charset() {
    if (type == STRING_EDGE) return regex_str->get_charset();
    return char_set;
  }

  // process an edge, returns true if edge should be used in creating evil strings
  bool process_edge(string test_string, Path *path);

  // get substring associated with edge
  string get_substring();

  // edge property functions - used by checker
  bool is_opt_repeat_begin();
  bool is_opt_repeat_end();
  bool is_wild_candidate();
  bool is_valid_character(char c);
  bool is_repeat_begin();
  bool is_repeat_end();
  bool is_repeat_punc_candidate();
  bool is_str_repeat_punc_candidate();
  char get_repeat_punc_char();
  int get_repeat_lower_limit();
  int get_repeat_upper_limit();
  bool is_zero_repeat_begin();
  bool is_zero_repeat_end();
  bool is_digit_too_optional_candidate();

  // creates a regex due to a wild punctuation error
  string fix_wild_punctuation(char c);

  // generate minimum iteration string
  void gen_min_iter_string(string &min_iter_string);

  // generate evil strings
  vector <string> gen_evil_strings(string path_string, const set <char> &punct_marks);

  // print the edge
  void print();

private:
  EdgeType type;		// type of edge
  Location loc;                 // location within original regex
  bool processed;		// set if edge is processed
  char character;		// character (for CHARACTER_EDGE)
  CharSet *char_set;		// character set (for CHAR_SET_EDGE)
  RegexString *regex_str;	// regex string (for STRING_EDGE)
  RegexLoop *regex_loop;	// regex loop (for BEGIN_LOOP_EDGE and END_LOOP_EDGE)
  Backref *backref;             // backreference (for BACKREFERENCE_EDGE)
};

#endif // EDGE_H
