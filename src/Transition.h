/*  Transition.h: a transition in an NFA and a path 

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

#ifndef TRANSITION_H
#define TRANSITION_H

#include "CharSet.h"
using namespace std;

typedef enum {
  CHARACTER_INPUT,
  CHAR_SET_INPUT,
  STRING_INPUT,
  CARET_INPUT,
  DOLLAR_INPUT,
  EPSILON,
  EMPTY
} TransitionType;

struct Transition
{
  TransitionType type;
  char character;
  CharSet *char_set;

  void print();
};


#endif // TRANSITION_H
