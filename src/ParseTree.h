/*  ParseTree.h: recursive descent parser

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

// The BNF for our simple regexes is:
//
// expr		::= concat '|' expr		(expression)
// 		|   concat '|'
//		|   '|' expr
//		|   '|'
//		|   concat
//
// concat	::= rep concat			(concatenation)
// 		|   rep
//
// rep		::= atom '*'			(repetition)
// 		|   atom '+'
// 		|   atom '?'
// 		|   atom '{n,m}'
// 		|   atom '{n,}'
// 		|   atom
//
// atom		::= group			(atomic)
// 		|   character
// 		|   char_class
// 		|   char_set
//
// group	::= '(' expr ')'		(group)
// 		| '(' NO_GROUP_EXT expr ')'		
// 		| '(' NAMED_GROUP_EXT expr ')'		
// 		| '(' IGNORED_EXT expr ')'		
// 		| '(' IGNORED_EXT ')'		
//
// character	::= CHARACTER 			(character)
//		|   '^'
//		|   '$'
//		|   '-'
//		|   WORD_BOUNDARY
//
// char_class	::= CHAR_CLASS			(character class)
//
// char_set	::= '[' char_list ']'		(character set)
// 		|   '[' '^' char_list ']'
//
// char_list	::= list_item char_list		(character list)
// 		|   list_item
//
// list_item	::= character_item		(list item)
//		|   char_class_item
// 		|   char_range_item
//
// character_item ::= CHARACTER			(character item)
//		|   '^'
//		|   '$'
//		|   '-'
//
// char_class_item ::= CHAR_CLASS		(character class item)
//
// char_range_item ::= CHARACTER '-' CHARACTER	(character range item)
//

#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include <set>
#include <cassert>
#include <unordered_map>
#include "Scanner.h"
#include "Backref.h"
#include "CharSet.h"
#include "Stats.h"
using namespace std;

typedef enum
{
  ALTERNATION_NODE,
  CONCAT_NODE,
  REPEAT_NODE,
  GROUP_NODE,
  BACKREFERENCE_NODE,
  CHARACTER_NODE,
  CHAR_SET_NODE,
  CARET_NODE,
  DOLLAR_NODE,
  IGNORED_NODE
} NodeType;

struct ParseNode
{
  ParseNode(NodeType t, Location _loc, ParseNode *l, ParseNode *r) {
    type = t;
    loc = _loc;
    left = l;
    right = r;
    char_set = NULL;
  }

  ParseNode(NodeType t, Location _loc, string _name, ParseNode *l, ParseNode *r) {
    assert(t == GROUP_NODE);
    type = t;
    loc = _loc;
    group_name = _name;
    left = l;
    right = r;
    char_set = NULL;
  }

  ParseNode(NodeType t, Location _loc, CharSet *c) {
    assert(t == CHAR_SET_NODE);
    type = t;
    loc = _loc;
    left = NULL;
    right = NULL;
    char_set = c;
  }

  ParseNode(NodeType t, Location _loc, char c) {
    assert(t == CHARACTER_NODE);
    type = t;
    loc = _loc;
    left = NULL;
    right = NULL;
    char_set = NULL;
    character = c;
  }

  ParseNode(NodeType t, Location _loc, Backref *b) {
    assert(t == BACKREFERENCE_NODE);
    type = t;
    loc = _loc;
    left = NULL;
    right = NULL;
    backref = b;
  }

  ParseNode(NodeType t, Location _loc, ParseNode *l, int lower, int upper) {
    assert(t == REPEAT_NODE);
    type = t;
    loc = _loc;
    left = l;
    right = NULL;
    char_set = NULL;
    repeat_lower = lower;
    repeat_upper = upper;
  }

  NodeType type;
  Location loc;
  ParseNode *left;
  ParseNode *right;
  char character;	// For CHARACTER_NODE
  CharSet *char_set;	// For CHAR_SET_NODE
  int repeat_lower;	// For REPEAT_NODE
  int repeat_upper;	// For REPEAT_NODE (-1 for no limit)
  Backref *backref;     // For BACKREFERENCE_NODE
  string group_name;    // For GROUP_NODE
};

class ParseTree {

public:

  // build parse tree using regex stored in scanner
  void build(Scanner &_scanner);

  // get root of the tree
  ParseNode *get_root() { return root; }

  // get set of punctuation marks
  set<char> get_punct_marks() { return punct_marks; }

  // prints the tree
  void print();

  // get tree stats
  void add_stats(Stats &stats);

private:

  ParseNode *root;		// root of parse tree
  Scanner scanner;		// scanner
  set<char> punct_marks;	// set of punctuation marks
  unordered_map<int, Location> group_locs;
  unordered_map<string, Location> named_group_locs;
  int group_count;

  // creation functions
  ParseNode *expr();
  ParseNode *concat();
  ParseNode *rep();
  ParseNode *atom();
  ParseNode *group();
  ParseNode *character();
  ParseNode *char_class();
  ParseNode *char_set();
  ParseNode *char_list(int start_loc);
  CharSetItem list_item();
  CharSetItem character_item();
  CharSetItem char_class_item();
  CharSetItem char_range_item();

  // print the tree
  void print_tree(ParseNode *node, unsigned offset);

  // gather stats
  struct ParseTreeStats {
    int alternation_nodes;
    int concat_nodes;
    int repeat_nodes;
    int unnamed_group_nodes;
    int named_group_nodes;
    int backreference_nodes;
    int character_nodes;
    int caret_nodes;
    int dollar_nodes;
    int normal_char_set_nodes;
    int complement_char_set_nodes;
    int ignored_nodes;
  };
  void gather_stats(ParseNode *node, ParseTreeStats &tree_stats);

};

#endif // PARSE_TREE_H

