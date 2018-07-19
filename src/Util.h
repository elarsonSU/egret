/*  Util.h: Singleton utility class

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

#ifndef ERROR_H
#define ERROR_H

#include <set>
#include <string>
#include <vector>
using namespace std;

// Location
typedef pair <int, int> Location;

struct Alert {

  bool warning;
  string type;
  string message;
  bool has_suggest;
  string suggest;
  bool has_example;
  string example;
  Location loc1;
  Location loc2;

  Alert(string t, string m) { 
    warning = false; type = t; message = m; has_suggest = false; has_example = false;
    loc1 = make_pair(-1, -1); loc2 = make_pair(-1, -1);
  }
  Alert(string t, string m, Location l1) { 
    warning = false; type = t; message = m; has_suggest = false; has_example = false;
    loc1 = l1; loc2 = make_pair(-1, -1);
  }
  Alert(string t, string m, Location l1, Location l2) { 
    warning = false; type = t; message = m; has_suggest = false; has_example = false;
    loc1 = l1; loc2 = l2;
  }
  Alert(string t, string m, string s) { 
    warning = false; type = t; message = m; has_suggest = true; has_example = false;
    suggest = s; loc1 = make_pair(-1, -1); loc2 = make_pair(-1, -1);
  }
  Alert(string t, string m, string s, Location l1) { 
    warning = false; type = t; message = m; has_suggest = true; has_example = false;
    suggest = s; loc1 = l1; loc2 = make_pair(-1, -1);
    
  }
  Alert(string t, string m, string s, Location l1, Location l2) { 
    warning = false; type = t; message = m; has_suggest = true; has_example = false;
    suggest = s; loc1 = l1; loc2 = l2;
  }
};

class Util {

public:
  static Util* get();

  void init(string r, bool c, bool w, string s);

  bool is_check_mode() { return check_mode; }
  bool is_web_mode() { return web_mode; }
  string get_base_substring() { return base_substring; }
  string get_regex() { return regex; }
  vector<string> get_alerts() { return alerts; }

  // Alerts 
  void add_alert(Alert alert);

// TODO: Possibly create a new regex class where the "fixing" functions reside?
private:
  Util() {};            // singleton class, private constructor
  static Util *inst;

  // Global options
  bool check_mode;
  bool web_mode;
  string base_substring; 

  string regex;                                 // original regular expression

  // Alerts
  vector <string> alerts;                       // vector of alert strings
  set <pair <string, int>> prev_alerts;         // all previous alerts

};
     
// TODO: Can this exception be folded into util class above?
// TODO: One idea is to add an add_error function that throws an exception that is caught at
// the top level.
// Egret Exception
class EgretException {

public:
  EgretException(string msg) { error_msg = msg; }
  string get_error() const { return error_msg; }

private:
  string error_msg;
};

#endif // ERROR_H

