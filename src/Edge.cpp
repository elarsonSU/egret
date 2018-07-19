/*  Edge.cpp: an edge in an NFA and a path 

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

#include <cassert>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "Edge.h"
#include "Scanner.h"
#include "Path.h"
using namespace std;

bool
Edge::process_edge(string test_string, Path *path)
{
  if (type == BEGIN_LOOP_EDGE) {
    regex_loop->set_curr_prefix(test_string);
  }
  if (type == END_LOOP_EDGE) {
    regex_loop->set_curr_substring(test_string);
  }
  if (type == BACKREFERENCE_EDGE) {
    backref->set_curr_prefix(test_string);
    backref->set_curr_substring(path->gen_backref_string(backref->get_group_loc()));
  }

  // no further work needed if edge already processed from prior path
  if (processed) return false;
  processed = true;

  // set the path prefix and substring for nodes that need it
  // and return true for evil edges
  switch (type) {
    case CHAR_SET_EDGE:
      char_set->set_prefix(test_string);
      return true;
    case STRING_EDGE:
      regex_str->set_prefix(test_string);
      regex_str->set_substring(Util::get()->get_base_substring());
      return true;
    case BEGIN_LOOP_EDGE:
      regex_loop->set_prefix(test_string);
      return false;
    case END_LOOP_EDGE:
      regex_loop->set_substring_from_curr();
      return true;
    case BACKREFERENCE_EDGE:
      backref->set_prefix_from_curr();
      backref->set_substring_from_curr();
      return true;
    default:
      return false;
  }
}

string
Edge::get_substring()
{
  string s;
  
  switch (type) {
    case CHARACTER_EDGE:
      s += character;
      return s;
    case CHAR_SET_EDGE:
      // TODO: Does the character field contain a valid character for char set?
      s += char_set->get_valid_character();
      return s;
    case STRING_EDGE:
      return regex_str->get_substring();
    case END_LOOP_EDGE:
      return regex_loop->get_substring();
    case BACKREFERENCE_EDGE:
      return backref->get_substring();
    default:
      return s;
  }
}

bool
Edge::is_opt_repeat_begin()
{
  return (type == BEGIN_LOOP_EDGE && regex_loop->is_opt_repeat());
}

bool
Edge::is_opt_repeat_end()
{
  return (type == END_LOOP_EDGE && regex_loop->is_opt_repeat());
}

bool
Edge::is_wild_candidate()
{
  if (type == CHAR_SET_EDGE && char_set->is_wildcard()) return true;
  if (type == CHAR_SET_EDGE && char_set->is_complement()) return true;
  if (type == STRING_EDGE && regex_str->is_wild_candidate()) return true;
  return false;
}

bool
Edge::is_valid_character(char c)
{
  if (type == CHARACTER_EDGE && character == c) return true;
  if (type == CHAR_SET_EDGE && char_set->is_wildcard()) return true;
  if (type == CHAR_SET_EDGE && char_set->is_valid_character(c)) return true;
  if (type == STRING_EDGE && regex_str->is_valid_character(c)) return true;
  return false;
}

bool
Edge::is_repeat_begin()
{
  if (type != BEGIN_LOOP_EDGE) return false;
  int upper = regex_loop->get_repeat_upper();
  if (upper == -1 || upper >= 2) return true;
  return false;
}

bool
Edge::is_repeat_end()
{
  if (type != END_LOOP_EDGE) return false;
  int upper = regex_loop->get_repeat_upper();
  if (upper == -1 || upper >= 2) return true;
  return false;
}

bool
Edge::is_repeat_punc_candidate()
{
  if (type == CHARACTER_EDGE && ispunct(character)) return true;
  if (type == CHAR_SET_EDGE && char_set->is_repeat_punc_candidate()) return true;
  return false;
}

bool
Edge::is_str_repeat_punc_candidate()
{
  return (type == STRING_EDGE && regex_str->is_repeat_punc_candidate());
}

char 
Edge::get_repeat_punc_char()
{
  if (type == CHARACTER_EDGE) return character;
  if (type == STRING_EDGE) return regex_str->get_repeat_punc_char();
  return char_set->get_repeat_punc_char();
}

int 
Edge::get_repeat_lower_limit()
{
  if (type == STRING_EDGE) return regex_str->get_repeat_lower();
  return regex_loop->get_repeat_lower();
}

int 
Edge::get_repeat_upper_limit()
{
  if (type == STRING_EDGE) return regex_str->get_repeat_upper();
  return regex_loop->get_repeat_upper();
}

bool
Edge::is_zero_repeat_begin()
{
  return (type == BEGIN_LOOP_EDGE && regex_loop->get_repeat_lower() == 0);
}

bool
Edge::is_zero_repeat_end()
{
  return (type == END_LOOP_EDGE && regex_loop->get_repeat_lower() == 0);
}

bool
Edge::is_digit_too_optional_candidate()
{
  return (type == CHAR_SET_EDGE && char_set->is_digit_too_optional_candidate());
}

string
Edge::fix_wild_punctuation(char c)
{
  string regex = Util::get()->get_regex();
  string curr_regex = regex.substr(loc.first, loc.second - loc.first + 1);

  string char_str = string(1, c);
  switch (c) {
    case '\\':
    case '[':
    case ']':
    case '^':
    case '-':
      char_str = "\\" + char_str;
  } 

  string fixed_char_set;
  if (curr_regex[0] == '.') {
    fixed_char_set = "[^" + char_str + "]";
  }
  else {
    assert(curr_regex[0] == '[');
    size_t pos = curr_regex.find_last_of(']');
    assert(pos != string::npos);
    fixed_char_set = curr_regex.substr(0, pos + 1);
    fixed_char_set.insert(pos, char_str);
  }

  return fixed_char_set;
}

void
Edge::gen_min_iter_string(string &min_iter_string)
{
  switch (type) {
    case STRING_EDGE:
      regex_str->gen_min_iter_string(min_iter_string);
      break;
    case BEGIN_LOOP_EDGE:
      regex_loop->set_curr_prefix(min_iter_string);
      break;
    case END_LOOP_EDGE:
      regex_loop->gen_min_iter_string(min_iter_string);
      break;
    default:
      min_iter_string += get_substring();
      break;
  }
}

vector <string>
Edge::gen_evil_strings(string path_string, const set <char> &punct_marks)
{
  switch (type) {
    case CHAR_SET_EDGE:
      return char_set->gen_evil_strings(path_string, punct_marks);
    case STRING_EDGE:
      return regex_str->gen_evil_strings(path_string, punct_marks);
    case END_LOOP_EDGE:
      return regex_loop->gen_evil_strings(path_string);
    case BACKREFERENCE_EDGE:
      return backref->gen_evil_strings(path_string);
    default:
    {
      vector <string> empty;
      return empty;
    }
  }
}

void
Edge::print()
{
  switch (type) {
    case CHARACTER_EDGE:
      cout << "CHARACTER " << character;
      break;
    case CHAR_SET_EDGE:
      cout << "CHAR_SET ";
      char_set->print();
      break;
    case STRING_EDGE:
      cout << "STRING ";
      regex_str->print();
      break;
    case BEGIN_LOOP_EDGE:
      cout << "BEGIN_LOOP ";
      regex_loop->print();
      break;
    case END_LOOP_EDGE:
      cout << "END_LOOP ";
      regex_loop->print();
      break;
    case CARET_EDGE:
      cout << "CARET";
      break;
    case DOLLAR_EDGE:
      cout << "DOLLAR";
      break;
    case BACKREFERENCE_EDGE:
      cout << "BACKREFERENCE ";
      backref->print();
      break;
    case EPSILON_EDGE:
      cout << "EPSILON";
      break;
  } 
  cout << " @ (" << loc.first << "," << loc.second << ")" << endl;
}
