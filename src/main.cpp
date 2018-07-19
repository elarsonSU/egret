/*  main.cpp: test driver for EGRET engine (used for debugging)

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

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "egret.h"
using namespace std;

static char *get_arg(int &idx, int argc, char **argv);

int
main(int argc, char *argv[])
{
  cout << "RUNNING PROGRAM" << endl;
  int idx = 1;
  string regex = "";
  string base_substring = "evil";
  bool check_mode = false;
  bool web_mode = false;
  bool debug_mode = false;
  bool stat_mode = false;

  // Process arguments
  while (idx < argc) {

    char *arg = get_arg(idx, argc, argv);

    // -r: regular expression
    if (strcmp(arg, "-r") == 0) {
      if (regex != "") {
	cerr << "USAGE: Can only have one regular expression to process" << endl;
	return -1;
      }
      regex = get_arg(idx, argc, argv);
    }

    // -f: file that contains a single regular expression
    else if (strcmp(arg, "-f") == 0) {
      ifstream regexFile;
      char *file_name = get_arg(idx, argc, argv);
      regexFile.open(file_name);

      if (!regexFile.is_open()) {
        cerr << "USAGE: Unable to open file " << file_name << endl;
        return -1;
      }
      
      if (regex != "") {
	cerr << "USAGE: Can only have one regular expression to process" << endl;
	return -1;
      }

      getline(regexFile, regex);
      regexFile.close();
    }

    // -b: base substring for regex strings
    else if (strcmp(arg, "-b") == 0) {
      base_substring = get_arg(idx, argc, argv);
    }

    // -c: run check mode
    else if (strcmp(arg, "-c") == 0) {
      check_mode = true;
    }

    // -d: print debug information based on the given mode 
    else if (strcmp(arg, "-d") == 0) {
      debug_mode = true;
    }

    // -s: print stats
    else if (strcmp(arg, "-s") == 0) {
      stat_mode = true;
    }

    // -w: run web mode
    else if (strcmp(arg, "-w") == 0) {
      web_mode = true;
    }


    // everything else is invalid
    else {
      cerr << "USAGE: Invalid command line option: " << arg << endl;
      return -1;
    }
  }

  if (regex == "") {
    cerr << "USAGE: Did not find a regular expression to process" << endl;
    return -1;
  }

  vector <string> test_strings =
    run_engine(regex, base_substring, check_mode, web_mode, debug_mode, stat_mode);

  vector <string>::iterator it;
  for (it = test_strings.begin(); it != test_strings.end(); it++) {
    cout << *it << endl;
  }

  return 0;
}

static char *
get_arg(int &idx, int argc, char **argv)
{
  char *arg;

  if (idx >= argc) {
    cerr << "USAGE: Invalid command line" << endl << endl;
  }

  arg = argv[idx];
  idx++;

  return arg;
}
