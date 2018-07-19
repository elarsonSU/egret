/*  Scanner.h: scanner for regular expression, used by parser 

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

#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include "Stats.h"
#include "Util.h"
using namespace std;

// TODO: Separate token into a separate file?
// TODO: Have the scanner stop after producing a list of tokens?

// Types of tokens
typedef enum
{ 
  ALTERNATION,		// |
  STAR,			// *
  PLUS,			// +
  QUESTION,		// ?
  REPEAT,		// {n}, {n,m}, {n,}, or {,m}
  LEFT_PAREN,		// ( 
  RIGHT_PAREN,		// )
  CHARACTER,		// any "non-special" character
  CHAR_CLASS,		// \w, \d, etc.
  LEFT_BRACKET,		// [
  RIGHT_BRACKET,	// ]
  CARET,		// ^
  DOLLAR,		// $
  HYPHEN,		// - in character set
  WORD_BOUNDARY,	// \b
  NO_GROUP_EXT,		// (?:...)
  NAMED_GROUP_EXT,	// (?P<name>...)
  IGNORED_EXT,		// extensions that are ignored
  BACKREFERENCE,        // backreferences
  ERR			// error 
} TokenType;

struct Token
{
  TokenType type;
  Location loc;         // location in regular expression <start, end>
  int repeat_lower;	// for REPEAT
  int repeat_upper;	// for REPEAT (-1 for no limit)
  char character;	// for CHARACTER and CHAR_CLASS
  int group_num;        // for BACKREFERENCE
  string group_name;    // for BACKREFERENCE and NAMED_GROUP_EXT
};

// A scanner class, encapsulates the input stream as a set of tokens
//
class Scanner {

public:

  vector <Token> get_tokens() { return tokens; }

  // scans through input string and creates a vector of tokens
  void init(string in);

  // TODO: Consider returning a token instead of all these specialized functions
  // returns type for current token
  TokenType get_type();

  // returns type string for current token
  string get_type_str();

  // returns location for current token
  Location get_loc();

  // returns repeat lower bound of current token
  int get_repeat_lower();

  // returns repeat upper bound of current token
  int get_repeat_upper();

  // returns character associated with current token
  char get_character();

  // returns the group number for a backreference
  int get_group_num();

  // returns the group name for a backreference or named group
  string get_group_name();

  // advance to the next token
  void advance();

  // determines if concatentation between index-1 and index tokens
  bool is_concat();

  // determines if next three tokens form a character range
  bool is_char_range();

  // print list of scanner tokens
  void print();

  // add scanner stats
  void add_stats(Stats &stats);

private:

  vector <Token> tokens;	// stores the regular expression
  unsigned index;		// iterator

  // get next character from input string
  char get_next_char(string in, unsigned int &idx);

  // process octal character 
  Token process_octal(string in, unsigned int &idx, char first_digit);

  // process hexadecimal character 
  Token process_hex(string in, unsigned int &idx, int num_digits);

  // processes Python extensions for regular expressions
  Token process_extension(string in, unsigned int &idx);

  // process a repeat quantifier {}
  Token process_repeat(string in, unsigned int &idx);

  // returns string name of a token
  string token_type_to_str(TokenType type);
};

#endif // SCANNER_H
