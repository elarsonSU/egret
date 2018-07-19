/*  egret.h: entry point into EGRET engine

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

#ifndef EGRET_H
#define EGRET_H

#include <string>
#include <vector>
using namespace std;

// run_engine: entry point into EGRET engine
vector <string>
run_engine(string regex, string base_substring,
    bool check_mode = false, bool web_mode = false, bool debug_mode = false, bool stat_mode = false);

#endif // EGRET_H
