/*  RegexString.h: represents a regex string

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

#ifndef REGEX_STRING_H
#define REGEX_STRING_H

#include <set>
using namespace std;

struct RegexString
{
  CharSet *char_set;		// corresponding character set
  int repeat_lower;     	// lower bound for string
  int repeat_upper;     	// upper bound for string

  // traversal info
  int path_index;               // path that contains the char set
  string path_prefix;           // path string up to visiting this node
  string substring;             // substring corresponding to this string
};

#endif // REGEX_STRING_H

