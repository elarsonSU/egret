/*  ParseTree.h: recursive descent parser

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

#include "Scanner.h"
#include "CharSet.h"
#include "Stats.h"

#include <set>
#include <cassert>
using namespace std;

typedef enum
{
  ALTERNATION_NODE,
  CONCAT_NODE,
  REPEAT_NODE,
  GROUP_NODE,
  CHARACTER_NODE,
  CARET_NODE,
  DOLLAR_NODE,
  CHAR_SET_NODE,
  IGNORED_NODE
} NodeType;

struct ParseNode
{
  ParseNode(NodeType _type, ParseNode *_left, ParseNode *_right)
    :type(_type), left(_left), right(_right), char_set(NULL) {}

  ParseNode(NodeType _type, ParseNode *_left, int lower, int upper)
    :type(_type), left(_left), right(NULL), repeat_lower(lower),
    repeat_upper(upper), char_set(NULL) { assert(type == REPEAT_NODE); }

  ParseNode(NodeType _type, char c)
    :type(_type), left(NULL), right(NULL), character(c),
     char_set(NULL) { assert(type == CHARACTER_NODE); }

  NodeType type;
  ParseNode *left;
  ParseNode *right;
  int repeat_lower;	// For REPEAT_NODE
  int repeat_upper;	// For REPEAT_NODE (-1 for no limit)
  char character;	// For CHARACTER_NODE
  CharSet *char_set;	// For CHAR_SET_NODE
};

class ParseTree {

public:

  // Create parse tree using regex stored in scanner
  void create(Scanner &_scanner);

  // Get root of the tree
  ParseNode *get_root() { return root; }

  // Get set of punctuation marks
  set<char> get_punct_marks() { return punct_marks; }

  // Prints the tree
  void print();

  // Get tree stats
  void add_stats(Stats &stats);

private:

  ParseNode *root;		// root of parse tree
  Scanner scanner;		// scanner
  set<char> punct_marks;	// set of punctuation marks

  // Creation functions
  ParseNode *expr();
  ParseNode *concat();
  ParseNode *rep();
  ParseNode *atom();
  ParseNode *group();
  ParseNode *character();
  ParseNode *char_class();
  ParseNode *char_set();
  ParseNode *char_list();
  CharSetItem list_item();
  CharSetItem character_item();
  CharSetItem char_class_item();
  CharSetItem char_range_item();

  // Printing functions
  void print_tree(ParseNode *node, unsigned offset);

  // Stats
  struct ParseTreeStats {
    int alternationNodes;
    int concatNodes;
    int repeatNodes;
    int groupNodes;
    int characterNodes;
    int caretNodes;
    int dollarNodes;
    int normalCharSetNodes;
    int complementCharSetNodes;
    int ignoredNodes;
  };
  void gather_stats(ParseNode *node, ParseTreeStats &tree_stats);

};

#endif // PARSE_TREE_H

