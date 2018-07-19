/*  Checker.h: Regular Expression Checker

    Copyright (C) 2016-2018  Eric Larson and Tyler Hartje
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

#ifndef CHECKER_H
#define CHECKER_H

#include <set>
#include <string>
#include <vector>
#include "Scanner.h"
#include "Path.h"
using namespace std;

class Checker {

public:

  Checker(vector <Path> p, vector <Token> t) {
    paths = p;
    tokens = t;
  }

  // checker entry point
  void check();

private:

  vector <Path> paths;		// list of paths
  vector <Token> tokens;        // set of tokens - used for generated fixes

  // CHECKER FUNCTIONS

  // check anchor usage
  void check_anchor_usage();

  // check anchor in middle
  void check_anchor_in_middle();

  // check chararacter sets
  void check_charsets();

  // check optional braces
  void check_optional_braces();

  // checks if wildcard is just before/after punctuation mark
  void check_wild_punctuation();

  // checks if punctuation can be repeated in certain situations
  void check_repeat_punctuation();

   // checks if digits are too optional
   void check_digit_too_optional();

  // fix anchors
  string fix_anchors();
};

#endif // CHECKER_H
