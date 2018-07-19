/*  CharSet.h: Character set

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

#ifndef CHARSET_H
#define CHARSET_H

#include <set>
#include <string>
#include <vector>
#include "Util.h"
using namespace std;

class Path;             // breaks a circular dependency CharSet --> Path --> Edge --> CharSet

typedef enum
{
  CHARACTER_ITEM,
  CHAR_CLASS_ITEM,
  CHAR_RANGE_ITEM
} CharSetItemType;

struct CharSetItem
{
  CharSetItemType type;
  char character;	// for CHARACTER_ITEM and CHAR_CLASS_ITEM
  char range_start;	// for CHAR_RANGE_ITEM
  char range_end;	// for CHAR_RANGE_ITEM
};

class CharSet {

public:

  CharSet() { complement = false; checked = false; }

  // setters
  void set_prefix(string p) { prefix = p; }
  void set_complement(bool c) { complement = c; }

  // getters
  bool is_complement() { return complement; }

  // CONSTRUCTION FUNCTIONS

  // add an item to the character set
  void add_item(CharSetItem item);

  // PROPERTY FUNCTIONS

  // returns true if character set is a single character
  bool is_single_char();

  // returns true if character set is a wildcard
  bool is_wildcard();

  // returns true if character set is a string candidate
  bool is_string_candidate();

  // returns true if character set allows punctuation
  bool allows_punctuation();

  // returns true if character set only has punctuation and spaces
  bool only_has_punc_and_spaces();

  // determines if a character is valid 
  bool is_valid_character(char character);

  // returns true if character is explicitly part of non-negated character set
  bool has_character_item(char character);

  // returns the character set as a sorted string
  string get_charset_as_string();

  // gets a single valid character
  char get_valid_character(char except = '\0');

  // CHECKER FUNCTIONS

  // checks the character set, emits warnings if necessary
  void check(Path *path, Location loc);

  // repeat puncutation checking functions
  bool is_repeat_punc_candidate();
  char get_repeat_punc_char();

  // digit too optional checking functions
  bool is_digit_too_optional_candidate();

  // TEST GENERATION FUNCTIONS

  // generate evil strings
  vector <string> gen_evil_strings(string test_string, const set <char> &punct_marks);

  // PRINT FUNCTION

  // print the character set
  void print();

private:

  vector <CharSetItem> items;	// set of items comprising the set
  bool complement;		// true if set is complemented
  string prefix;		// path string up to visiting this node
  bool checked;			// true of charset has been checked

  // checker functions
  bool only_has_punc(bool allow_spaces = false);
  bool is_good_range(char start, char end);
  bool has_upper_range();
  bool has_lower_range();
  bool has_digit_range();
  string fix_bad_range(Location loc);
  bool has_range(Location loc);
  string fix_comma_bar_charset(Location loc, char elim);
  void replace(string &str, string from, string to);
  string replace_charset_with_parens(Location loc);
  
  // creates a set of test characters
  set <char> create_test_chars(const set <char> &punct_marks);
};

#endif // CHARSET_H
