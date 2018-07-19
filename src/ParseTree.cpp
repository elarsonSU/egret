/*  ParseTree.cpp: recursive descent parser

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

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include "Backref.h"
#include "CharSet.h"
#include "ParseTree.h"
#include "Scanner.h"
#include "Stats.h"
#include "Util.h"
using namespace std;

//=============================================================
// RD Parser
//=============================================================

void
ParseTree::build(Scanner &_scanner)
{
  group_count = 1;

  scanner = _scanner;
  root = expr();
  
  if (scanner.get_type() != ERR) {
    stringstream s;
    s << "ERROR (parse error): expected end of regex but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  // count_groups();
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
  Location loc = scanner.get_loc();
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
    throw EgretException("ERROR (pointless alternation): both clauses are empty");
  }
  // left empty: return right?
  else if (left == NULL) {
    ParseNode *expr_node = new ParseNode(REPEAT_NODE, loc, right, 0, 1);
    return expr_node;
  }
  // right empty: return left?
  else if (right == NULL) {
    ParseNode *expr_node = new ParseNode(REPEAT_NODE, loc, left, 0, 1);
    return expr_node;
  }
  
  // otherwise return left | right
  ParseNode *expr_node = new ParseNode(ALTERNATION_NODE, loc, left, right);
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
    int left_loc = left->loc.second;
    Location loc = make_pair(left_loc, left_loc + 1);
    ParseNode *concat_node = new ParseNode(CONCAT_NODE, loc, left, right);
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
  Location loc = scanner.get_loc();

  // then check for repetition character
  if (scanner.get_type() == STAR) {
    scanner.advance();
    ParseNode *rep_node = new ParseNode(REPEAT_NODE, loc, atom_node, 0, -1);
    return rep_node;
  }
  else if (scanner.get_type() == PLUS) {
    scanner.advance();
    ParseNode *rep_node = new ParseNode(REPEAT_NODE, loc, atom_node, 1, -1);
    return rep_node;
  }
  else if (scanner.get_type() == QUESTION) {
    scanner.advance();
    ParseNode *rep_node = new ParseNode(REPEAT_NODE, loc, atom_node, 0, 1);
    return rep_node;
  }
  else if (scanner.get_type() == REPEAT) {
    int lower = scanner.get_repeat_lower();
    int upper = scanner.get_repeat_upper();
    scanner.advance();
    ParseNode *rep_node = new ParseNode(REPEAT_NODE, loc, atom_node, lower, upper);
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
  bool ignored_group = false;
  bool normal_group = true;
  string name = "";
  int start_loc = scanner.get_loc().second;

  if (scanner.get_type() != LEFT_PAREN) {
    stringstream s;
    s << "ERROR (parse error): expected '(' but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  // Determine if it a special use of parentheses
  if (scanner.get_type() == NO_GROUP_EXT) {
    normal_group = false;
    scanner.advance();
  }
  if (scanner.get_type() == NAMED_GROUP_EXT) {
    name = scanner.get_group_name();
    scanner.advance();
  }
  if (scanner.get_type() == IGNORED_EXT) {
    normal_group = false;
    ignored_group = true;
    scanner.advance();
  }

  // Assign the group number now before advancing scanner
  int group_num;
  if (normal_group) {
    group_num = group_count;
    group_count++;
  }
  else {
    group_num = -1;
  }

  // Get the group expression
  ParseNode *left;
  if (!ignored_group || scanner.get_type() != RIGHT_PAREN) {
    left = expr();
  }
  else {
    left = NULL;
  }

  // Create the group node
  ParseNode *group_node;
  int end_loc = scanner.get_loc().first;
  Location loc = make_pair(start_loc, end_loc);
  if (ignored_group) {
    group_node = new ParseNode(IGNORED_NODE, loc, NULL, NULL);
  }
  else {
    group_node = new ParseNode(GROUP_NODE, loc, name, left, NULL);
  }

  // Store group information
  if (normal_group) {
    group_locs[group_num] = loc;
    if (name != "") {
      named_group_locs[name] = loc;
    }
  }

  if (scanner.get_type() != RIGHT_PAREN) {
    stringstream s;
    s << "ERROR (parse error): expected ')' but received " << scanner.get_type_str();
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
  Location loc = scanner.get_loc();
  TokenType type = scanner.get_type();

  if (type == CHARACTER) {
    char c = scanner.get_character();
    scanner.advance();
    character_node =  new ParseNode(CHARACTER_NODE, loc, c);
    if (ispunct(c)) {
      if (punct_marks.find(c) == punct_marks.end()) {
        punct_marks.insert(c);
      }
    }
  }
  else if (type == CARET) {
    scanner.advance();
    return new ParseNode(CARET_NODE, loc, NULL, NULL);
  }
  else if (type == DOLLAR) {
    scanner.advance();
    return new ParseNode(DOLLAR_NODE, loc, NULL, NULL);
  }
  else if (type == HYPHEN) {
    scanner.advance();
    character_node =  new ParseNode(CHARACTER_NODE, loc, '-');
    if (punct_marks.find('-') == punct_marks.end()) {
      punct_marks.insert('-');
    }
  }
  else if (type == WORD_BOUNDARY) {
    scanner.advance();
    return new ParseNode(IGNORED_NODE, loc, NULL, NULL);
  }
  else if (type == BACKREFERENCE) {
    int group_num = scanner.get_group_num();
    string group_name = scanner.get_group_name();
    Location group_loc;
    if (group_name != "") {
      group_loc = named_group_locs[group_name];
    }
    else {
      group_loc = group_locs[group_num];
    }

    Backref *backref = new Backref(group_name, group_num, group_loc);
    character_node = new ParseNode(BACKREFERENCE_NODE, loc, backref);
    scanner.advance();
  }
  else {
    stringstream s;
    s << "ERROR (parse error): expected character type but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }

  return character_node;
}

// char_class ::= CHAR_CLASS
//
ParseNode *
ParseTree::char_class()
{
  Location loc = scanner.get_loc();
  char c = scanner.get_character();
  scanner.advance();

  CharSet *char_set = new CharSet();

  CharSetItem char_set_item;
  char_set_item.type = CHAR_CLASS_ITEM;
  char_set_item.character = c;
  char_set->add_item(char_set_item);

  ParseNode *char_set_node = new ParseNode(CHAR_SET_NODE, loc, char_set);
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
  int start_loc = scanner.get_loc().second;

  if (scanner.get_type() != LEFT_BRACKET) {
    stringstream s;
    s << "ERROR (parse error): expected '[' but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  if (scanner.get_type() == CARET) {
    is_complement = true;
    scanner.advance();
  }

  char_set_node = char_list(start_loc);
  if (is_complement) char_set_node->char_set->set_complement(true);
  if (char_set_node->char_set->is_single_char() && !is_complement) {
    char c = char_set_node->char_set->get_valid_character();
    delete char_set_node;
    int end_loc = scanner.get_loc().first;
    Location loc = make_pair(start_loc, end_loc);
    char_set_node = new ParseNode(CHARACTER_NODE, loc, c);
  }

  if (scanner.get_type() != RIGHT_BRACKET) {
    stringstream s;
    s << "ERROR (parse error): expected ']' but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  return char_set_node;
}

// char_list ::= list_item charlist
// 	     |   list_item
//
ParseNode *
ParseTree::char_list(int start_loc)
{
  CharSetItem char_set_item = list_item();
  ParseNode *char_set_node;
  
  // Check for end of list
  if (scanner.get_type() == RIGHT_BRACKET) {
    int end_loc = scanner.get_loc().first;
    Location loc = make_pair(start_loc, end_loc);
    char_set_node = new ParseNode(CHAR_SET_NODE, loc, new CharSet());
  }
  else {
    char_set_node = char_list(start_loc);
  }

  char_set_node->char_set->add_item(char_set_item);
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
    scanner.advance();
    char_set_item.character = '-';
  }
  else {
    stringstream s;
    s << "ERROR (parse error): expected character type but received " << scanner.get_type_str();
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

  //TODO:  These seem like sanity checks - maybe assertions instead?
  if (scanner.get_type() != CHARACTER) {
    stringstream s;
    s << "ERROR (parse error): expected character type but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char start = scanner.get_character();
  scanner.advance();

  if (scanner.get_type() != HYPHEN) {
    stringstream s;
    s << "ERROR (parse error): expected hyphen but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  if (scanner.get_type() != CHARACTER) {
    stringstream s;
    s << "ERROR (parse error): expected character type but received " << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char end = scanner.get_character();
  scanner.advance();

  char_set_item.range_start = start;
  char_set_item.range_end = end;
  return char_set_item;
}

void
ParseTree::print()
{
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

  cout << "<";
  switch (node->type) {
  case ALTERNATION_NODE:
    cout << "alternation |";
    break;
  case CONCAT_NODE:
    cout << "concat";
    break;
  case REPEAT_NODE:
    if (node->repeat_upper == -1)
      cout << "repeat {" << node->repeat_lower << ",}";
    else 
      cout << "repeat {" << node->repeat_lower << "," << node->repeat_upper << "}";
    break;
  case GROUP_NODE:
    cout << "group"; 
    break;
  case BACKREFERENCE_NODE:
    cout << "backreference ";
    node->backref->print();
    break;
  case IGNORED_NODE:
    cout << "ignored";
    break;
  case CHARACTER_NODE:
    cout << "character: " << node->character;
    break;
  case CARET_NODE:
    cout << "caret ^";
    break;
  case DOLLAR_NODE:
    cout << "dollar $";
    break;
  case CHAR_SET_NODE:
    cout << "charset [";
    node->char_set->print();
    cout << "]";
    break;
  default:
    assert(false);
  }

  cout << " @ (" << node->loc.first << "," << node->loc.second <<  ")>" << endl;

  print_tree(node->left, offset + 2);
  print_tree(node->right, offset + 2);
}

void
ParseTree::add_stats(Stats &stats)
{
  ParseTreeStats tree_stats = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  gather_stats(root, tree_stats);
  stats.add("PARSE_TREE", "Alternation nodes", tree_stats.alternation_nodes);
  stats.add("PARSE_TREE", "Concat nodes", tree_stats.concat_nodes);
  stats.add("PARSE_TREE", "Repeat nodes", tree_stats.repeat_nodes);
  stats.add("PARSE_TREE", "Group nodes (unnamed)", tree_stats.unnamed_group_nodes);
  stats.add("PARSE_TREE", "Group nodes (named)", tree_stats.named_group_nodes);
  stats.add("PARSE_TREE", "Backreference nodes", tree_stats.backreference_nodes);
  stats.add("PARSE_TREE", "Caret nodes", tree_stats.caret_nodes);
  stats.add("PARSE_TREE", "Dollar nodes", tree_stats.dollar_nodes);
  stats.add("PARSE_TREE", "Character nodes", tree_stats.character_nodes);
  stats.add("PARSE_TREE", "Character set nodes (not ^)", tree_stats.normal_char_set_nodes);
  stats.add("PARSE_TREE", "Character set nodes (^)", tree_stats.complement_char_set_nodes);
  stats.add("PARSE_TREE", "Ignored nodes", tree_stats.ignored_nodes);
}

void
ParseTree::gather_stats(ParseNode *node, ParseTreeStats &tree_stats)
{
  if (!node) return;

  switch (node->type) {
  case ALTERNATION_NODE:
    tree_stats.alternation_nodes++;
    break;
  case CONCAT_NODE:
    tree_stats.concat_nodes++;
    break;
  case REPEAT_NODE:
    tree_stats.repeat_nodes++;
    break;
  case GROUP_NODE:
    if (node->group_name == "")
      tree_stats.unnamed_group_nodes++;
    else
      tree_stats.named_group_nodes++;
    break;
  case BACKREFERENCE_NODE:
    tree_stats.backreference_nodes++;
    break;
  case CHARACTER_NODE:
    tree_stats.character_nodes++;
    break;
  case CARET_NODE:
    tree_stats.caret_nodes++;
    break;
  case DOLLAR_NODE:
    tree_stats.dollar_nodes++;
    break;
  case CHAR_SET_NODE:
    if (node->char_set->is_complement())
      tree_stats.complement_char_set_nodes++;
    else
      tree_stats.normal_char_set_nodes++;
    break;
  case IGNORED_NODE:
    tree_stats.ignored_nodes++;
    break;
  default:
    assert(false);
  }

  gather_stats(node->left, tree_stats);
  gather_stats(node->right, tree_stats);
}
