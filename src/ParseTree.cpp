/*  ParseTree.cpp: recursive descent parser

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

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <set>
#include "ParseTree.h"
#include "Scanner.h"
#include "CharSet.h"
#include "Stats.h"
#include "error.h"

using namespace std;

//=============================================================
// RD Parser
//=============================================================

// create parse tree using regex stored in scanner
//
void
ParseTree::create(Scanner &_scanner)
{
  scanner = _scanner;
  root = expr();

  if (scanner.get_type() != ERR) {
    stringstream s;
    s << "ERROR: Parse error - expected end of regex but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
}

// expr ::= concat '|' expr
//	|   concat '|'
//	|   '|' expr
//	|   '|'
//      |   concat
//
ParseNode *
ParseTree::expr()
{
  ParseNode *left, *right;

  // check for alternation without a "left"
  if (scanner.get_type() == ALTERNATION) {
    left = NULL;
  } else {
    left = concat();
  }

  // check for lack of alternation
  if (scanner.get_type() != ALTERNATION) {
    return left;
  }

  // advance past alternation token
  scanner.advance();

  // check for lacking right
  if (scanner.get_type() == RIGHT_PAREN || scanner.get_type() == ERR) {
    right = NULL;
  } else {
    right = expr();
  }

  // check for empty alternation clauses
  // both empty: abort with an error
  if (left == NULL && right == NULL) {
    throw EgretException("ERROR: pointless alternation (both clauses are empty)");
  }
  // left empty: return right?
  else if (left == NULL) {
    ParseNode *expr_node = new ParseNode(REPEAT_NODE, right, 0, 1);
    return expr_node;
  }
  // right empty: return left?
  else if (right == NULL) {
    ParseNode *expr_node = new ParseNode(REPEAT_NODE, left, 0, 1);
    return expr_node;
  }
  
  // otherwise return left | right
  ParseNode *expr_node = new ParseNode(ALTERNATION_NODE, left, right);
  return expr_node;
}

// concat ::= rep concat
//        |   rep
//
ParseNode *
ParseTree::concat()
{
  // always a repetition node to the left
  ParseNode *left = rep();

  // check for concatenation
  if (scanner.is_concat()) {
    ParseNode *right = concat();
    ParseNode *concat_node = new ParseNode(CONCAT_NODE, left, right);
    return concat_node;
  } else {
    return left;
  }
}

// rep  ::= atom '*'
//      |   atom '?'
//      |   atom '+'
//      |   atom '{n,m}'
//      |   atom '{n,}'
//      |   atom
//
ParseNode *
ParseTree::rep()
{
  // first is always atom node
  ParseNode *atom_node = atom();

  // then check for repetition character
  if (scanner.get_type() == STAR) {
    scanner.advance();
    ParseNode *rep_node = new ParseNode(REPEAT_NODE, atom_node, 0, -1);
    return rep_node;
  }
  else if (scanner.get_type() == PLUS) {
    scanner.advance();
    ParseNode *rep_node = new ParseNode(REPEAT_NODE, atom_node, 1, -1);
    return rep_node;
  }
  else if (scanner.get_type() == QUESTION) {
    scanner.advance();
    ParseNode *rep_node = new ParseNode(REPEAT_NODE, atom_node, 0, 1);
    return rep_node;
  }
  else if (scanner.get_type() == REPEAT) {
    int lower = scanner.get_repeat_lower();
    int upper = scanner.get_repeat_upper();
    scanner.advance();
    ParseNode *rep_node = new ParseNode(REPEAT_NODE, atom_node, lower, upper);
    return rep_node;
  }
  else {
    return atom_node;
  }
}

// atom	::= group
// 	|   character
//	|   char_class
// 	|   char_set
//
ParseNode *
ParseTree::atom()
{
  ParseNode *atom_node;

  // check for group
  if (scanner.get_type() == LEFT_PAREN) {
    atom_node = group();
  }

  // check for character set
  else if (scanner.get_type() == LEFT_BRACKET) {
    atom_node = char_set();
  }

  // check for character class
  else if (scanner.get_type() == CHAR_CLASS) {
    atom_node = char_class();
  }

  // otherwise atom node is character
  else {
    atom_node = character();
  }

  return atom_node;
}

// group ::= '(' expr ')'
//       | '(' NO_GROUP_EXT expr ')'             
//       | '(' NAMED_GROUP_EXT expr ')'
//       | '(' IGNORED_EXT expr ')'
//       | '(' IGNORED_EXT ')'
//
ParseNode *
ParseTree::group()
{
  ParseNode *group_node;
  ParseNode *left;
  bool ignored_group = false;

  if (scanner.get_type() != LEFT_PAREN) {
    stringstream s;
    s << "ERROR: Parse error - expected '(' but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();
 
  if (scanner.get_type() == NO_GROUP_EXT)
    scanner.advance();
  if (scanner.get_type() == NAMED_GROUP_EXT)
    scanner.advance();
  if (scanner.get_type() == IGNORED_EXT) {
    scanner.advance();
    ignored_group = true;
  }

  if (!ignored_group || scanner.get_type() != RIGHT_PAREN) {
    left = expr();
  }

  if (ignored_group) {
    group_node = new ParseNode(IGNORED_NODE, NULL, NULL);
  }
  else {
    group_node = new ParseNode(GROUP_NODE, left, NULL);
  }

  if (scanner.get_type() != RIGHT_PAREN) {
    stringstream s;
    s << "ERROR: Parse error - expected ')' but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  return group_node;
}

// character ::= CHARACTER
// 	     |   '^'
// 	     |   '$'
//	     |	 '-'
//	     |   WORD_BOUNDARY
//
ParseNode *
ParseTree::character()
{
  ParseNode *character_node;

  if (scanner.get_type() == CHARACTER) {
    char c = scanner.get_character();
    scanner.advance();
    character_node =  new ParseNode(CHARACTER_NODE, c);
  }
  else if (scanner.get_type() == CARET) {
    scanner.advance();
    return new ParseNode(CARET_NODE, NULL, NULL);
  }
  else if (scanner.get_type() == DOLLAR) {
    scanner.advance();
    return new ParseNode(DOLLAR_NODE, NULL, NULL);
  }
  else if (scanner.get_type() == HYPHEN) {
    addWarning("received HYPHEN outside char range - could be a bad range");
    scanner.advance();
    character_node =  new ParseNode(CHARACTER_NODE, '-');
  }
  else if (scanner.get_type() == WORD_BOUNDARY) {
    scanner.advance();
    return new ParseNode(IGNORED_NODE, NULL, NULL);
  }
  else {
    stringstream s;
    s << "ERROR: Parse error - expected character type but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char c = character_node->character;
  if (ispunct(c)) {
    if (punct_marks.find(c) == punct_marks.end()) {
      punct_marks.insert(c);
    }
  }
  return character_node;
}

// char_class ::= CHAR_CLASS
//
ParseNode *
ParseTree::char_class()
{
  char c = scanner.get_character();
  scanner.advance();

  ParseNode *char_set_node = new ParseNode(CHAR_SET_NODE, NULL, NULL);
  char_set_node->char_set = new CharSet();

  CharSetItem char_set_item;
  char_set_item.type = CHAR_CLASS_ITEM;
  char_set_item.character = c;
  char_set_node->char_set->items.push_back(char_set_item);

  return char_set_node;
}

// char_set ::= '[' char_list ']'
// 	    |   '[' '^' char_list ']'
//
ParseNode *
ParseTree::char_set()
{
  ParseNode *char_set_node;
  bool is_complement = false;

  if (scanner.get_type() != LEFT_BRACKET) {
    stringstream s;
    s << "ERROR: Parse error - expected '[' but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  if (scanner.get_type() == CARET) {
    is_complement = true;
    scanner.advance();
  }

  char_set_node = char_list();
  if (is_complement) char_set_node->char_set->complement = true;

  if (scanner.get_type() != RIGHT_BRACKET) {
    stringstream s;
    s << "ERROR: Parse error - expected ']' but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  return char_set_node;
}

// char_list ::= list_item charlist
// 	     |   list_item
//
ParseNode *
ParseTree::char_list()
{
  CharSetItem char_set_item = list_item();
  ParseNode *char_set_node;
  
  // Check for end of list
  if (scanner.get_type() == RIGHT_BRACKET) {
    char_set_node = new ParseNode(CHAR_SET_NODE, NULL, NULL);
    char_set_node->char_set = new CharSet();
  }
  else {
    char_set_node = char_list();
  }

  char_set_node->char_set->items.push_back(char_set_item);
  return char_set_node;
}


// list_item ::= character_item
//           |   char_class_item
//           |   char_range_item
//
CharSetItem
ParseTree::list_item()
{
  if (scanner.is_char_range()) {
    return char_range_item();
  }
  else if (scanner.get_type() == CHAR_CLASS) {
    return char_class_item();
  }
  else {
    return character_item();
  }
}

// character_item ::= CHARACTER
// 	          |   '^'
// 	          |   '$'
//	          |   '-'
//
CharSetItem
ParseTree::character_item()
{
  CharSetItem char_set_item;
  char_set_item.type = CHARACTER_ITEM;

  if (scanner.get_type() == CHARACTER) {
    char c = scanner.get_character();
    scanner.advance();
    char_set_item.character = c;
  }
  else if (scanner.get_type() == CARET) {
    scanner.advance();
    char_set_item.character = '^';
  }
  else if (scanner.get_type() == DOLLAR) {
    scanner.advance();
    char_set_item.character = '$';
  }
  else if (scanner.get_type() == HYPHEN) {
    addWarning("received HYPHEN outside char range - could be a bad range");
    scanner.advance();
    char_set_item.character = '-';
  }
  else {
    stringstream s;
    s << "ERROR: Parse error - expected character type but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char c = char_set_item.character;
  if (ispunct(c)) {
    if (punct_marks.find(c) == punct_marks.end()) {
      punct_marks.insert(c);
    }
  }
  return char_set_item;
}

// char_class_item ::= CHAR_CLASS
//
CharSetItem
ParseTree::char_class_item()
{
  CharSetItem char_set_item;
  char_set_item.type = CHAR_CLASS_ITEM;
  char_set_item.character = scanner.get_character();
  scanner.advance();
  return char_set_item;
}

// char_range_item ::= CHARACTER '-' CHARACTER
//
CharSetItem
ParseTree::char_range_item()
{
  CharSetItem char_set_item;
  char_set_item.type = CHAR_RANGE_ITEM;

  if (scanner.get_type() != CHARACTER) {
    stringstream s;
    s << "ERROR: Parse error - expected character type but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char_set_item.range_start = scanner.get_character();
  scanner.advance();

  if (scanner.get_type() != HYPHEN) {
    stringstream s;
    s << "ERROR: Parse error - expected hyphen but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  if (scanner.get_type() != CHARACTER) {
    stringstream s;
    s << "ERROR: Parse error - expected character type but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char_set_item.range_end = scanner.get_character();
  scanner.advance();

  return char_set_item;
}

//=============================================================
// Print functions
//=============================================================

// Prints the tree
void
ParseTree::print() {
  cout << "Tree:" << endl;
  print_tree(root, 0);
  cout << endl;
}

void
ParseTree::print_tree(ParseNode *node, unsigned offset)
{
  if (!node) return;

  for (unsigned int i = 0; i < offset; i++)
    cout << " ";

  switch (node->type) {
  case ALTERNATION_NODE:
    cout << "<alternation |>";
    break;
  case CONCAT_NODE:
    cout << "<concat>";
    break;
  case REPEAT_NODE:
    if (node->repeat_upper == -1)
      cout << "<repeat {" << node->repeat_lower << ",}>";
    else 
      cout << "<repeat {" << node->repeat_lower << "," << node->repeat_upper << "}>";
    break;
  case GROUP_NODE:
    cout << "<group ()>";
    break;
  case IGNORED_NODE:
    cout << "<ignored>";
    break;
  case CHARACTER_NODE:
    cout << "<character: " << node->character << ">";
    break;
  case CARET_NODE:
    cout << "<caret ^>";
    break;
  case DOLLAR_NODE:
    cout << "<dollar $>";
    break;
  case CHAR_SET_NODE:
    cout << "<charset [";
    node->char_set->print();
    cout << "]>";
    break;
  default:
    assert(false);
  }

  cout << endl;

  print_tree(node->left, offset + 2);
  print_tree(node->right, offset + 2);
}

//=============================================================
// Stat functions
//=============================================================

// Obtains stats
void
ParseTree::add_stats(Stats &stats)
{
  ParseTreeStats tree_stats = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  gather_stats(root, tree_stats);
  stats.add("PARSE_TREE", "Alternation nodes", tree_stats.alternationNodes);
  stats.add("PARSE_TREE", "Concat nodes", tree_stats.concatNodes);
  stats.add("PARSE_TREE", "Repeat nodes", tree_stats.repeatNodes);
  stats.add("PARSE_TREE", "Group nodes", tree_stats.groupNodes);
  stats.add("PARSE_TREE", "Caret nodes", tree_stats.caretNodes);
  stats.add("PARSE_TREE", "Dollar nodes", tree_stats.dollarNodes);
  stats.add("PARSE_TREE", "Character nodes", tree_stats.characterNodes);
  stats.add("PARSE_TREE", "Character set nodes (not ^)", tree_stats.normalCharSetNodes);
  stats.add("PARSE_TREE", "Character set nodes (^)", tree_stats.complementCharSetNodes);
  stats.add("PARSE_TREE", "Ignored nodes", tree_stats.ignoredNodes);
}

void
ParseTree::gather_stats(ParseNode *node, ParseTreeStats &tree_stats)
{
  if (!node) return;

  switch (node->type) {
  case ALTERNATION_NODE:
    tree_stats.alternationNodes++;
    break;
  case CONCAT_NODE:
    tree_stats.concatNodes++;
    break;
  case REPEAT_NODE:
    tree_stats.repeatNodes++;
    break;
  case GROUP_NODE:
    tree_stats.groupNodes++;
    break;
  case CHARACTER_NODE:
    tree_stats.characterNodes++;
    break;
  case CARET_NODE:
    tree_stats.caretNodes++;
    break;
  case DOLLAR_NODE:
    tree_stats.dollarNodes++;
    break;
  case CHAR_SET_NODE:
    if (node->char_set->complement)
      tree_stats.complementCharSetNodes++;
    else
      tree_stats.normalCharSetNodes++;
    break;
  case IGNORED_NODE:
    tree_stats.ignoredNodes++;
    break;
  default:
    assert(false);
  }

  gather_stats(node->left, tree_stats);
  gather_stats(node->right, tree_stats);
}
