/*  State.cpp: a state in an NFA and a path

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
#include "State.h"

using namespace std;

void
State::print()
{
  if (end_repeat_state) {
    cout << "REPEAT END {" << repeat_lower << "," << repeat_upper
         << "} (BEGIN: " << begin_index << ")";
  }
  else if (end_charset_state) {
    cout << "CHAR SET END (BEGIN: " << begin_index << ")";
  }
  else {
    cout << "NORMAL";
  }
}
