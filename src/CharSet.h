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

#include <vector>
#include <set>
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

struct CharSet
{
  vector <CharSetItem> items;	// set of items comprising the set
  bool complement;		// true if set is complemented

  // constructor
  CharSet() { complement = false; }

  // determines if character set is a string candidate
  bool is_string_candidate();

  // find a good character
  char find_good_character(set<char> punct_marks);

  // determines if a character is valid in a complemented character set
  bool is_good_character(char character);

  // creates a set of test characters
  set<char> create_test_chars(set<char> punct_marks);
    
  // print functions
  void print();
};

#endif // CHARSET_H
