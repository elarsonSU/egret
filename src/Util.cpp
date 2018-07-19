/*  Util.cpp: Singleton utility class

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

#include <string>
#include <sstream>
#include "Util.h"
using namespace std;

// global static pointer for singleton class
Util* Util::inst = NULL;

Util *
Util::get() 
{
  if (inst == NULL) inst = new Util;
  return inst;
}

void
Util::init(string r, bool c, bool w, string s)
{
  regex = r;
  check_mode = c;
  web_mode = w;
  base_substring = s;
  alerts.clear();
  prev_alerts.clear();
}

void
Util::add_alert(Alert alert)
{
  // Create type, location pair
  pair <string, int> alert_pair = make_pair(alert.type, alert.loc1.first);

  // Line break
  string lb = web_mode ? "<br>" : "\n";
  string start = web_mode ? "<mark>" : "\033[33;44;1m";
  string end = web_mode ? "</mark>" : "\033[0m";

  if (prev_alerts.find(alert_pair) == prev_alerts.end()) {
    // New error - add to list of previous alerts
    prev_alerts.insert(alert_pair);
  }
  else {
    // Duplicate - do not add the error again
    return;
  }

  // Ignore warnings in check mode (warnings only relevant in test generation mode)
  if (alert.warning && check_mode) return;

  // Produce alert message
  stringstream s;
  if (alert.warning)
    s << "WARNING (";
  else
    s << "VIOLATION (";
  s << alert.type << "): " << alert.message << lb;
  
  if (alert.loc1.first != -1) {
    s << "...Regex: ";

    for (int i = 0; i < (int) regex.size(); i++) {
      if (i == alert.loc1.first || i == alert.loc2.first) {
        s << start;
      }
      s << regex[i];
      if (i == alert.loc1.second || i == alert.loc2.second) {
        s << end;
      }
    }
    s << lb;
  }

  if (alert.has_suggest) {
    s << "...Suggested fix: " << alert.suggest << lb;
  }

  if (alert.has_example) {
    s << "...Example accepted string: " << alert.example << lb;
  }
  alerts.push_back(s.str());
}
