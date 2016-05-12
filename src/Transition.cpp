/*  Transition.cpp: a transition in an NFA and a path 

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

#include <iostream>
#include <vector>
#include "Transition.h"

using namespace std;

void
Transition::print()
{
  switch (type) {
  case CHARACTER_INPUT:
    cout << "CHARACTER(" << character << ")" << endl;
    break;
  case CHAR_SET_INPUT:
    cout << "CHAR_SET(" << character << "):" << endl;
    break;
  case STRING_INPUT:
    cout << "STRING_INPUT(" << character << "):" << endl;
    break;
  case CARET_INPUT:
    cout << "CARET" << endl;
    break;
  case DOLLAR_INPUT:
    cout << "DOLLAR" << endl;
    break;
  case EPSILON:
    cout << "EPSILON" << endl;
    break;
  case EMPTY:
    cout << "EMPTY" << endl;
    break;
  }
}
