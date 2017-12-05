/*  CharSet.h: Character set

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

#ifndef CHARSET_H
#define CHARSET_H

#include <set>
#include <string>
#include <vector>
#include "TestString.h"
using namespace std;

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
  void set_prefix(TestString p) { prefix = p; }
  void set_complement(bool c) { complement = c; }

  // getters
  bool is_complement() { return complement; }

  // CONSTRUCTION FUNCTIONS

  // add an item to the character set
  void add_item(CharSetItem item);

  // PROPERTY FUNCTIONS

  // returns true if character set is a string candidate
  bool is_string_candidate();

  // returns true if character set allows punctuation
  bool allows_punctuation();

  // returns true if character set only has characters
  bool only_has_characters();

  // returns the character set as a sorted string
  string get_charset_as_string();

  // TEST GENERATION FUNCTIONS

  // gets a single valid character
  char get_valid_character();

  // generate evil strings
  vector <TestString> gen_evil_strings(TestString test_string, const set <char> &punct_marks);

  // CHECKER FUNCTION

  // checks the character set, emits warnings if necessary
  void check();

  // PRINT FUNCTION

  // print the character set
  void print();


private:

  vector <CharSetItem> items;	// set of items comprising the set
  bool complement;		// true if set is complemented
  TestString prefix;		// path string up to visiting this node
  bool checked;			// true of charset has been checked

  // determines if a character is valid in a complemented character set
  bool is_valid_character(char character);

  // creates a set of test characters
  set <char> create_test_chars(const set <char> &punct_marks);

  // checker functions
  void check_single_punctuation();
  void check_only_digits_and_punctuation();
};

#endif // CHARSET_H
