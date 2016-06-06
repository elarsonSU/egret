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
#include "Path.h"
#include "Edge.h"
using namespace std;

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

string
Path::gen_initial_string(string base_substring)
{
  path_string = "";
  for (unsigned int i = 0; i < edges.size(); i++) {
    bool process_edge = edges[i]->process_edge_in_path(path_string, base_substring);
    if (process_edge) {
      evil_edges.push_back(i);
    }
    path_string += edges[i]->get_substring();
  }
  return path_string;
}

bool
Path::has_leading_caret()
{
  for (unsigned int i = 0; i < edges.size(); i++) {
    if (edges[i]->getType() == CARET_EDGE) {
      return true;
    }
    else if (edges[i]->getType() != EPSILON_EDGE) {
      return false;
    }
  }
  return false;
}

bool
Path::has_trailing_dollar()
{
  for (unsigned int i = edges.size() - 1; i > 0; i--) {
    if (edges[i]->getType() == DOLLAR_EDGE) {
      return true;
    }
    else if (edges[i]->getType() != EPSILON_EDGE) {
      return false;
    }
  }
  return false;
}

string
Path::check_anchor_middle()
{
  bool seen_non_caret = false;
  bool seen_dollar = false; 
  bool caret_in_middle = false;
  bool dollar_in_middle = false;
  unsigned int caret_index;
  unsigned int dollar_index;

  // traverse the path
  for (unsigned int i = 0; i < edges.size(); i++) {
    switch (edges[i]->getType()) {
      case CARET_EDGE:
	if (seen_non_caret && !caret_in_middle) {
	  caret_in_middle = true;
	  caret_index = i;
	}
	break;
      case DOLLAR_EDGE:
	if (!dollar_in_middle) {
	  seen_dollar = true;
	  dollar_index = i;
	}
	break;
      case BEGIN_LOOP_EDGE:
      case END_LOOP_EDGE:
      case EPSILON_EDGE:
	break;
      default:	// everything else (not epsilon or anchor)
	seen_non_caret = true;
	if (seen_dollar) dollar_in_middle = true;
    }
  }

  // return if no violations
  if (!caret_in_middle && !dollar_in_middle) return "";

  // create before and after substring
  unsigned int split_index = caret_in_middle ? caret_index : dollar_index;
  string before = "";
  for (unsigned int i = 0; i < split_index; i++) {
    before += edges[i]->get_substring();
  }
  string after = "";
  for (unsigned int i = split_index + 1; i < edges.size(); i++) {
    after += edges[i]->get_substring();
  }
 
  // generate error message
  if (caret_in_middle) {
    stringstream s;
    s << "Generated string has ^ anchor in middle\n";
    s << "...Characters before ^ anchor: " << before << "\n";
    s << "...Characters after ^ anchor:  " << after;
    return s.str();
  }
  else {
    stringstream s;
    s << "Generated string has $ anchor in middle\n";
    s << "...Characters before $ anchor: " << before << "\n";
    s << "...Characters after $ anchor:  " << after;
    return s.str();
  }
}

set <string>
Path::gen_evil_strings(const set <char> &punct_marks)
{
  set <string> evil_strings;

  vector <unsigned int>::iterator it;
  for (unsigned int i = 0; i < evil_edges.size(); i++) {
    int index = evil_edges[i];
    set <string> new_strings = edges[index]->gen_evil_strings(path_string, punct_marks);
    evil_strings.insert(new_strings.begin(), new_strings.end());
  }
  return evil_strings;
}
