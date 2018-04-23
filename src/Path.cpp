/*  Path.cpp: Represents a path through the NFA

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

#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include "Path.h"
#include "Edge.h"
#include "TestString.h"
#include "error.h"
using namespace std;

// PATH CONSTRUCTION FUNCTIONS

void
Path::append(Edge *edge, unsigned int state)
{
  edges.push_back(edge);
  states.push_back(state);
}

void
Path::remove_last()
{
  edges.pop_back();
  states.pop_back();
}

void
Path::mark_path_visited(bool *visited)
{
  vector <unsigned int>::iterator it;
  for (it = states.begin(); it != states.end(); it++) {
    visited[*it] = true;
  }
}

// PATH PROCESSING FUNCTION

void
Path::process_path(TestString base_substring)
{
  // Clear the string to start
  test_string.clear();

  for (unsigned int i = 0; i < edges.size(); i++) {
    // An edge must be processed first before being added, the function returns
    // whether the edge is evil and more tests should be added later.
    bool evil_edge = edges[i]->process_edge(test_string, base_substring);
    if (evil_edge) {
      evil_edges.push_back(i);
    }

    // Add the substring to the initial string.
    test_string.append(edges[i]->get_substring());
  }
}

// CHECKER FUNCTIONS

bool
Path::has_leading_caret()
{
  for (unsigned int i = 0; i < edges.size(); i++) {
    switch (edges[i]->get_type()) {
      case CARET_EDGE:
	return true;
      case BEGIN_LOOP_EDGE:
      case END_LOOP_EDGE:
      case EPSILON_EDGE:
      case BACKREFERENCE_EDGE:
      case BEGIN_GROUP_EDGE:
      case END_GROUP_EDGE:
	break; // Skip over
      default:
	return false;
    }
  }
  return false;
}

bool
Path::has_trailing_dollar()
{
  for (unsigned int i = edges.size() - 1; i > 0; i--) {
    switch (edges[i]->get_type()) {
      case DOLLAR_EDGE:
	return true;
      case BEGIN_LOOP_EDGE:
      case END_LOOP_EDGE:
      case EPSILON_EDGE:
      case BACKREFERENCE_EDGE:
      case BEGIN_GROUP_EDGE:
      case END_GROUP_EDGE:
	break; // Skip over
      default:
	return false;
    }
  }
  return false;
}

bool
Path::check_anchor_in_middle()
{
  bool seen_non_caret = false;
  bool seen_dollar = false; 

  for (unsigned int i = 0; i < edges.size(); i++) {
    switch (edges[i]->get_type()) {
      case CARET_EDGE:
	if (seen_non_caret) {
	  addViolation("anchor middle", "Generated string has ^ anchor in middle: " +
	    test_string.get_string());
	  return true;
	}
	break;
      case DOLLAR_EDGE:
	seen_dollar = true;
	break;
      case BEGIN_LOOP_EDGE:
      case END_LOOP_EDGE:
      case EPSILON_EDGE:
      case BACKREFERENCE_EDGE:
      case BEGIN_GROUP_EDGE:
      case END_GROUP_EDGE:
	break;
      default:
	seen_non_caret = true;
	if (seen_dollar) {
          addViolation("anchor middle", "Generated string has $ anchor in middle: " +
	    test_string.get_string());
	  return true;
	}
    }
  }

  return false;
}

bool
Path::check_charsets()
{
  set <string> charsets; // keeps track of charsets, looking for duplicates
  
  for (unsigned int i = 0; i < edges.size(); i++) {

    if (edges[i]->get_type() == CHAR_SET_EDGE) {
      CharSet *charset_ptr = edges[i]->get_charset();

      // check the character set
      charset_ptr->check();

      // look for duplicate charsets, only consider charsets that have only characters and are
      // not complemented
      if (charset_ptr->only_has_characters() && !charset_ptr->is_complement()) {
	string charset_str = charset_ptr->get_charset_as_string();
        if (charset_str.length() > 1) {
          if (charsets.find(charset_str) == charsets.end()) {
	    // not a duplicate - add to list
	    charsets.insert(charset_str);
          }
          else {
	    addViolation("duplicate charset", "Found duplicate character set [" + charset_str + "]");
	    return true;
          }
	}
      }
    }
  }

  return false;
}

// TEST STRING GENERATION FUNCTIONS

TestString
Path::gen_min_iter_string()
{
  TestString min_iter_string;
  for (unsigned int i = 0; i < edges.size(); i++) {
    edges[i]->gen_min_iter_string(min_iter_string);
  }
  return min_iter_string;
}

vector <TestString>
Path::gen_evil_strings(const set <char> &punct_marks)
{
  vector <TestString> evil_strings;

  // add strings for interesting edges (char sets, strings, and loops)
  for (unsigned int i = 0; i < evil_edges.size(); i++) {
    int index = evil_edges[i];
    vector <TestString> new_strings = edges[index]->gen_evil_strings(test_string, punct_marks);
    vector <TestString>::iterator tsi;
    for (tsi = new_strings.begin(); tsi != new_strings.end(); tsi++) {
      evil_strings.push_back(*tsi);
    }
  }
  return evil_strings;
}

// PRINT FUNCTION

void
Path::print()
{
  for (unsigned int i = 0; i < edges.size(); i++)
    edges[i]->print();
}
