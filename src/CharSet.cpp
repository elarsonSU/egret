/*  CharSet.cpp: Character set

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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "CharSet.h"
#include "Path.h"
#include "Util.h"
using namespace std;

// CONSTRUCTION FUNCTIONS
void
CharSet::add_item(CharSetItem item)
{
  items.push_back(item);
}

// PROPERTY FUNCTIONS

bool
CharSet::is_single_char()
{
  return (items.size() == 1 && items.begin()->type == CHARACTER_ITEM);
}

bool
CharSet::is_wildcard()
{
  if (items.size() != 1) return false;
  vector<CharSetItem>::iterator item_ptr = items.begin();
  return (item_ptr->type == CHAR_CLASS_ITEM && item_ptr->character == '.');
}

bool
CharSet::is_string_candidate()
{
  // In for order a char set to be a string candidate, one of the
  // following must be true:
  // - the char set is complemented
  // - the char set contains char class \w, \D, \S, or .
  // - the char set contains char range A-Z or a-z
  // In addition, it cannot contain a character class except 0-9.

  if (complement) return true;

  bool candidate = false;

  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
        break; 	// Ignore
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'w':	candidate = true; break;
          case 'D':	candidate = true; break;
          case 'S':	candidate = true; break;
          case '.':	candidate = true; break;
          default:	;
	}
        break;
      case CHAR_RANGE_ITEM:
        if (it->range_start == 'a' && it->range_end == 'z')  {
          candidate = true;
          break;
        }
        if (it->range_start == 'A' && it->range_end == 'Z') {
          candidate = true;
          break;
        }
        if (it->range_start == '0' && it->range_end == '9') {
          break;
        }
        return false;
    }
  }

  return candidate;
}

bool
CharSet::allows_punctuation()
{
  if (complement) return true;

  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
	if (ispunct(it->character)) return true;
        break;
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'E':
          case 'D':
          case 'S':
          case '.':
	    return true;
          default:	;
	}
        break;
      case CHAR_RANGE_ITEM:
	break;	// ignore
    }
  }

  return false;
}

bool
CharSet::only_has_punc_and_spaces()
{
  return only_has_punc(true);
}

bool
CharSet::only_has_punc(bool allow_spaces)
{
  vector <CharSetItem>::iterator it;
  if (complement) return false;

  bool found_punc = false;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
      {
        char c = it->character;
        if (allow_spaces) {
          if (!isspace(c) && !ispunct(c)) return false;
        }
        else {
          if (!ispunct(c)) return false;
        }
        if (ispunct(c)) found_punc = true;
        break;
      }
      case CHAR_RANGE_ITEM:
      {
        char start = it->range_start;
        char end = it->range_end;
        for (char c = start; c <= end; c++) {
          if (allow_spaces) {
            if (!isspace(c) && !ispunct(c)) return false;
          }
          else {
            if (!ispunct(c)) return false;
          }
          if (ispunct(c)) found_punc = true;
        }
        break;
      }
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 's':
            if (!allow_spaces) return false;
            break;
          case 'w':
	  case 'd':
	  case 'D':
	  case 'S':
	  case '.':
            return false;
        }
    }
  }
  
  return found_punc;
}

bool
CharSet::is_valid_character(char character)
{
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
	if (character == it->character) return !complement;
	break;
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'w':
	    if (character >= 'a' && character <= 'z') return !complement; 
	    if (character >= 'A' && character <= 'Z') return !complement; 
	    if (character >= '0' && character <= '9') return !complement; 
	    if (character == '_') return !complement; 
	    break;
	  case 'd':
	    if (character >= '0' && character <= '9') return !complement; 
	    break;
	  case 's':
	    if (character == ' ') return !complement;
	    break;
	  case 'W':
	  {
	    bool matches_w = false;
	    if (character >= 'a' && character <= 'z') matches_w = true;
	    if (character >= 'A' && character <= 'Z') matches_w = true;
	    if (character >= '0' && character <= '9') matches_w = true;
	    if (character == '_') matches_w = true;
	    if (!matches_w) return !complement;
	    break;
	  }
	  case 'D':
	    if (!(character >= '0' && character <= '9')) return !complement; 
	    break;
	  case 'S':
	    if (character != ' ') return !complement;
	    break;
	  case '.':		
            return !complement;
	  default:
	  {
	    stringstream s;
	    s << "ERROR (internal): Invalid character class in character set: "
	      << it->character;
	    throw EgretException(s.str());
	  }
  	}
        break;

      case CHAR_RANGE_ITEM:
        if (character >= it->range_start && character <= it->range_end) return !complement;
        break;
    }
  }
  return complement;
}

bool
CharSet::has_character_item(char character)
{
  if (complement) return false;

  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHARACTER_ITEM && character == it->character)
      return true;
  }

  return false;
}

string
CharSet::get_charset_as_string()
{
  vector <CharSetItem>::iterator it;
  string ret = "";
  vector <CharSetItem> sorted_items = items;

  struct sortClass {
    bool operator() (CharSetItem x, CharSetItem y) {
      return (x.character <= y.character);
    }
  } sortObject;
  
  sort(sorted_items.begin(), sorted_items.end(), sortObject);

  for (it = sorted_items.begin(); it != sorted_items.end(); it++) {
    ret.push_back(it->character);
  }
  
  return ret;
}

char
CharSet::get_valid_character(char except)
{
  vector <CharSetItem>::iterator it;
  const char PUNC_ARRAY[32] = { '!', '\"', '#', '$', '%', '&', '\'', '*', '+', '/',
        ':', ';', '<', '=', '>', '?', '@', '\\', '^', '_', '`', '~', '-', '.',
        '{', '[', '(', '}', ']', ')', ',', '|'};

  if (Util::get()->is_check_mode()) {
    // TODO: The first of the function for test generation could be skipped over or test
    // generation could use this function.
    for (char c = 'a'; c <= 'z'; c++) {
      if (except == c) continue;
      if (is_valid_character(c)) return c;
    }
    for (char c = 'A'; c <= 'Z'; c++) {
      if (except == c) continue;
      if (is_valid_character(c)) return c;
    }
    for (char c = '0'; c <= '9'; c++) {
      if (except == c) continue;
      if (is_valid_character(c)) return c;
    }
    for (unsigned int i = 0; i < 32; i++) {
      char c = PUNC_ARRAY[i];
      if (except == c) continue;
      if (is_valid_character(c)) return c;
    }
    if ((except != ' ') && is_valid_character(' ')) return ' ';
    // TODO: Refactor this function, can't find a character, go to the test generation algorithm
  }

  // test generation mode only at this point
  if (!complement) {
    // If set is not complemented, choose the first explicit character.
    for (it = items.begin(); it != items.end(); it++) {
      if (it->type == CHARACTER_ITEM) {
        if (it->character != except) {
	  return it->character;
        }
      }
    }
    // If set is not complemented and there are no explicit character,
    // choose the first character that meets the criteria.
    for (it = items.begin(); it != items.end(); it++) {
      switch (it->type) {
	case CHARACTER_ITEM:
	  break; 	// Ignore - alreay processed in earlier loop.
        case CHAR_CLASS_ITEM:
          switch (it->character) {
            case 'w':	return (except != 'a') ? 'a' : 'b';
            case 'd':	return (except != '0') ? '0' : '1';
            case 's':	return (except != ' ') ? ' ' : '\t';
            case 'W':	return (except != ';') ? ';' : '&';
            case 'D':	return (except != 'a') ? 'a' : 'b';
            case 'S':	return (except != 'a') ? 'a' : 'b';
            case '.':	return (except != 'a') ? 'a' : 'b';
	    default:
	    {
	      stringstream s;
	      s << "ERROR (internal): Invalid character class in character set: "
	           << it->character;
	      throw EgretException(s.str());
  	    }
	  }
          break;
        case CHAR_RANGE_ITEM:
        {
          char c = it->range_start;
  	  return (except != c) ? c : c + 1;
	  break;
        }
      }
    }

    if (is_valid_character(except)) return except;

    throw EgretException(
	"ERROR (internal): Could not find good char in regular char set");
  }

  // At this point, the character set is complemented. Find the first valid character.
  for (char c = 'a'; c <= 'z'; c++) {
    if (except == c) continue;
    if (is_valid_character(c)) return c;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    if (except == c) continue;
    if (is_valid_character(c)) return c;
  }
  for (char c = '0'; c <= '9'; c++) {
    if (except == c) continue;
    if (is_valid_character(c)) return c;
  }
  if ((except != ' ') && is_valid_character(' ')) return ' ';

  for (char c = 33; c <= 47; c++) {
    if (except == c) continue;
    if (is_valid_character(c)) return c;
  }
  for (char c = 58; c <= 64; c++) {
    if (except == c) continue;
    if (is_valid_character(c)) return c;
  }
  for (char c = 91; c <= 96; c++) {
    if (except == c) continue;
    if (is_valid_character(c)) return c;
  }
  for (char c = 123; c <= 126; c++) {
    if (except == c) continue;
    if (is_valid_character(c)) return c;
  }

  throw EgretException("ERROR (internal): Could not valid character in char set");
}

// CHECKER FUNCTIONS

void
CharSet::check(Path *path, Location loc)
{
  if (checked) return;
  checked = true;

  set <char> ind_chars;
  set <char> duplicates;
  bool bar_found = false;
  bool not_bar_punc_found = false;
  bool bar_violation = false;
  bool comma_found = false;
  bool not_comma_punc_found = false;
  bool comma_violation = false;

  // Ignore characer sets that only contain one character
  if (items.size() == 1 && items.begin()->type == CHARACTER_ITEM) return;

  // Check for three item character sets
  if (items.size() == 3 && !complement) {
    vector <CharSetItem>::iterator first, second, third;
    first = items.begin();
    second = first + 1;
    third = second + 1;
    if (second->type == CHARACTER_ITEM) {
      if (second->character == '|') {
        string suggest = fix_comma_bar_charset(loc, '|'); 
        Alert a("charset sep", "Likely use of | in character set for alternation", suggest, loc);
        a.has_example = true;
        a.example = path->gen_example_string(loc, '|');
        Util::get()->add_alert(a);
        bar_violation = true;
      }
      if (second->character == ',') {
        string suggest = fix_comma_bar_charset(loc, ','); 
        Alert a("charset sep", "Likely use of , in character set to separate cases", suggest, loc);
        a.has_example = true;
        a.example = path->gen_example_string(loc, ',');
        Util::get()->add_alert(a);
        comma_violation = true;
      }
    }
  }
    
  // Process individual characters first
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHARACTER_ITEM) {
      char c = it->character;

      // Check for duplicates
      if (ind_chars.find(c) != ind_chars.end()) {
	duplicates.insert(c);
      }
      else {
        ind_chars.insert(c);
      }

      if (ispunct(c)) {
        if (c == '|') {
          if (it != items.begin() && it + 1 != items.end()) {
            bar_found = true;
          }
        }
        else {
          not_bar_punc_found = true;
        }
        if (c == ',') {
          if (it != items.begin() && it + 1 != items.end()) {
            comma_found = true;
          }
        }
        else {
          not_comma_punc_found = true;
        }
      }
    }

    if (it->type == CHAR_RANGE_ITEM) {
      char start = it->range_start;
      char end = it->range_end;

      // Check for bad ranges
      bool good_range = is_good_range(start, end);

      // Ignore cases: |-| ,-
      if (start == '|' && end == '|') {
        good_range = true;
      }
      if (start == ',' && end == ',') {
        good_range = true;
      }

      if (!good_range) {
        stringstream s;
        s << "The fragment " << start << "-" << end << " is interpreted as a range";
        Alert a("bad range", s.str(), fix_bad_range(loc), loc);
        Util::get()->add_alert(a);
      }
      else {
        for (char c = start; c <= end; c++) {
          if (ind_chars.find(c) != ind_chars.end()) {
	    duplicates.insert(c);
          }
          else {
            ind_chars.insert(c);
          }
        }
      }
    }
  }

  // Check if | or , are only duplicates:
  bool dup_bar = false;
  bool dup_comma = false;
  bool dup_other = false;
  if (!duplicates.empty()) {

    set <char>::iterator si;
    for (si = duplicates.begin(); si != duplicates.end(); si++) {
      switch (*si) {
        case '|':
          dup_bar = true;
          break;
        case ',':
          dup_comma = true;
          break;
        default:
          dup_other = true;
      }
    } 

    // If both duplicate bar and commas, treat as normal duplicate character case.
    if (dup_bar && dup_comma) {
      dup_bar = false;
      dup_comma = false;
      dup_other = true;
    }
  }

  // Report duplicate violations
  if (dup_bar || (bar_found && !not_bar_punc_found && !complement)) {
    if (!bar_violation) {
      string suggest;
      if (has_range(loc)) {
        suggest = fix_comma_bar_charset(loc, '|'); 
      }
      else {
        suggest = replace_charset_with_parens(loc);
      }
      Alert a("charset sep", "Likely use of | in character set for alternation", suggest, loc);
      a.has_example = true;
      a.example = path->gen_example_string(loc, '|');
      Util::get()->add_alert(a);
    }
  }
  else if (dup_comma || (comma_found && !not_comma_punc_found && !complement)) {
    if (!comma_violation) {
      string suggest = fix_comma_bar_charset(loc, ','); 
      Alert a("charset sep", "Likely use of , in character set to separate cases", suggest, loc);
      a.has_example = true;
      a.example = path->gen_example_string(loc, ',');
      Util::get()->add_alert(a);
    }
  }
  else if (dup_other || dup_bar || dup_comma) {
    stringstream s;
    s << "Duplicate characters in character set:";
    set <char>::iterator si;
    for (si = duplicates.begin(); si != duplicates.end(); si++) {
      s << " " << *si;
    }
    if (s.str() != "&") {
      Alert a("duplicate char", s.str(), loc);
      Util::get()->add_alert(a);
    }
  }

  // Report brace violations
  if (is_valid_character('(') && !is_valid_character(')')) {
    Alert a("charset brace", "Found ( in charset but not ), could lead to unbalanced ()", loc);
    a.has_example = true;
    a.example = path->gen_example_string(loc, '(', ')');
    Util::get()->add_alert(a);
  }
  if (is_valid_character('{') && !is_valid_character('}')) {
    Alert a("charset brace", "Found { in charset but not {, could lead to unbalanced {}", loc);
    a.has_example = true;
    a.example = path->gen_example_string(loc, '{', '}');
    Util::get()->add_alert(a);
  }
  if (is_valid_character('[') && !is_valid_character(']')) {
    Alert a("charset brace", "Found [ in charset but not ], could lead to unbalanced []", loc);
    a.has_example = true;
    a.example = path->gen_example_string(loc, '[', ']');
    Util::get()->add_alert(a);
  }
  if (!is_valid_character('(') && is_valid_character(')')) {
    Alert a("charset brace", "Found ) in charset but not (, could lead to unbalanced ()", loc);
    a.has_example = true;
    a.example = path->gen_example_string(loc, ')', '(');
    Util::get()->add_alert(a);
  }
  if (!is_valid_character('{') && is_valid_character('}')) {
    Alert a("charset brace", "Found } in charset but not {, could lead to unbalanced {}", loc);
    a.has_example = true;
    a.example = path->gen_example_string(loc, '}', '{');
    Util::get()->add_alert(a);
  }
  if (!is_valid_character('[') && is_valid_character(']')) {
    Alert a("charset brace", "Found ] in charset but not [, could lead to unbalanced []", loc);
    a.has_example = true;
    a.example = path->gen_example_string(loc, ']', '[');
    Util::get()->add_alert(a);
  }
}

bool
CharSet::is_repeat_punc_candidate()
{
  if (only_has_punc()) return true;
  if (!complement && has_character_item('.')) return true;
  if (!complement && has_character_item(',')) return true;
  return false;
}

char
CharSet::get_repeat_punc_char()
{
  if (only_has_punc()) return get_valid_character();
  if (has_character_item('.')) return '.';
  if (has_character_item(',')) return ',';
  return 'X';
}

bool
CharSet::is_digit_too_optional_candidate()
{
  if (items.size() != 1) return false;
  vector <CharSetItem>::iterator vi = items.begin();

  if (vi->type == CHAR_CLASS_ITEM && vi->character == 'd') return true;
  if (vi->type == CHAR_RANGE_ITEM && vi->range_start == '0' && vi->range_end == '9') return true;
  if (vi->type == CHAR_RANGE_ITEM && vi->range_start == '1' && vi->range_end == '9') return true;
  return false;
}

bool
CharSet::is_good_range(char start, char end)
{
  if (start >= 'a' && start < 'z' && end > 'a' && end <= 'z') return true;
  if (start >= 'A' && start < 'Z' && end > 'A' && end <= 'Z') return true;
  if (start >= '0' && start < '9' && end > '0' && end <= '9') return true;

  if (complement && end <= 0x1F) return true;

  return false;
}

bool
CharSet::has_upper_range()
{
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
	break;
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'w':
	  case 'D':
	  case 'S':
	  case '.':		
            return true;
  	}
        break;
      case CHAR_RANGE_ITEM:
        if (it->range_start == 'A' && it->range_end == 'Z') return true;
        break;
    }
  }
  return false;
}

bool
CharSet::has_lower_range()
{
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
	break;
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'w':
	  case 'D':
	  case 'S':
	  case '.':		
            return true;
  	}
        break;
      case CHAR_RANGE_ITEM:
        if (it->range_start == 'a' && it->range_end == 'z') return true;
        break;
    }
  }
  return false;
}

bool
CharSet::has_digit_range()
{
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
	break;
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'w':
	  case 'd':
	  case 'S':
	  case '.':		
            return true;
  	}
        break;
      case CHAR_RANGE_ITEM:
        if (it->range_start == '0' && it->range_end == '9') return true;
        if (it->range_start == '1' && it->range_end == '9') return true;
        break;
    }
  }
  return false;
}

string
CharSet::fix_bad_range(Location loc)
{
  string new_charset = "[";
  string regex = Util::get()->get_regex();
  int begin = loc.first + 1;

  bool has_upper = has_upper_range();
  bool has_lower = has_lower_range();
  bool has_digit = has_digit_range();

  new_charset += regex[begin];
  if (regex[begin] == '^') {
    begin++;
    new_charset += regex[begin];
  }
  begin++;

  bool punc_range_found = false;
  for (int i = begin; i < loc.second; i++) {
    if (i != loc.second - 1 && regex[i] == '-' && (regex[i-1] != '\\' || regex[i-2] == '\\')) {
      char start = regex[i-1];
      char end = regex[i+1];
      if (is_good_range(start, end)) {
        new_charset += '-';
      }
      else if (ispunct(start) || ispunct(end)) {
        punc_range_found = true;
      }
      else if (start == 'A' && end == 'z') {
        if (has_upper) {
          new_charset[new_charset.size()-1] = 'a';
          new_charset += '-';
        }
        else if (has_lower) {
          new_charset += '-';
          new_charset += 'Z';
          i++;
        }
        else {
          new_charset += "-Za-";
        }
      }
      else if (start == 'A' && end == '9') {
        if (has_upper) {
          new_charset[new_charset.size()-1] = '0';
          new_charset += '-';
        }
        else if (has_digit) {
          new_charset += '-';
          new_charset += 'Z';
          i++;
        }
        else {
          new_charset += "-Z0-";
        }
      }
      else if (start == 'a' && end == 'Z') {
        if (has_lower) {
          new_charset[new_charset.size()-1] = 'A';
          new_charset += '-';
        }
        else if (has_upper) {
          new_charset += '-';
          new_charset += 'z';
          i++;
        }
        else {
          new_charset += "-zA-";
        }
      }
      else if (start == 'a' && end == '9') {
        if (has_lower) {
          new_charset[new_charset.size()-1] = '0';
          new_charset += '-';
        }
        else if (has_digit) {
          new_charset += '-';
          new_charset += 'z';
          i++;
        }
        else {
          new_charset += "-z0-";
        }
      }
      else if ((start == '0' || start == '1') && end == 'Z') {
        if (has_digit) {
          new_charset[new_charset.size()-1] = 'A';
          new_charset += '-';
        }
        else if (has_upper) {
          new_charset += '-';
          new_charset += '9';
          i++;
        }
        else {
          new_charset += "-9A-";
        }
      }
      else if ((start == '0' || start == '1') && end == 'z') {
        if (has_digit) {
          new_charset[new_charset.size()-1] = 'a';
          new_charset += '-';
        }
        else if (has_lower) {
          new_charset += '-';
          new_charset += '9';
          i++;
        }
        else {
          new_charset += "-9a-";
        }
      }
    }
    else {
      new_charset += regex[i];
    }
  }
  if (punc_range_found && regex[loc.second - 1] != '-') {
    new_charset += '-';
  }
  new_charset += ']';

  return new_charset;
}

bool
CharSet::has_range(Location loc)
{
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHAR_RANGE_ITEM) return true;
  }

  string regex = Util::get()->get_regex();
  string charset = regex.substr(loc.first, loc.second - loc.first + 1);
  if (regex.find("0|9") != string::npos) return true;
  if (regex.find("0,9") != string::npos) return true;
  if (regex.find("A|Z") != string::npos) return true;
  if (regex.find("A,Z") != string::npos) return true;
  if (regex.find("a|z") != string::npos) return true;
  if (regex.find("a,z") != string::npos) return true;

  return false;
}

string 
CharSet::fix_comma_bar_charset(Location loc, char elim)
{
  string regex = Util::get()->get_regex();
  string new_regex;

  string charset = regex.substr(loc.first, loc.second - loc.first + 1);
  replace(charset, "0|9", "0-9");
  replace(charset, "0,9", "0-9");
  replace(charset, "A|Z", "A-Z");
  replace(charset, "A,Z", "A-Z");
  replace(charset, "a|z", "a-z");
  replace(charset, "a,z", "a-z");

  for (unsigned int i = 0; i < charset.size(); i++) {
    if (charset[i] != elim) {
      new_regex += charset[i];
    }
  }

  // TODO: Can this code which eliminates bad ranges from the result use a modified fix_bad_range function?
  string new_charset = "[";
  regex = new_regex;
  int begin = 1;

  new_charset += regex[begin];
  if (regex[begin] == '^') {
    begin++;
    new_charset += regex[begin];
  }
  begin++;

  bool punc_range_found = false;
  for (unsigned int i = begin; i < regex.size() - 2; i++) {
    if (regex[i] == '-' && (regex[i-1] != '\\' || regex[i-2] == '\\')) {
      char start = regex[i-1];
      char end = regex[i+1];
      if (is_good_range(start, end)) {
        new_charset += '-';
      }
      else if (ispunct(start) || ispunct(end)) {
        punc_range_found = true;
      }
    }
    else {
      new_charset += regex[i];
    }
  }
  new_charset += regex[regex.size() - 2];
  if (punc_range_found && regex[regex.size() - 2] != '-') {
    new_charset += '-';
  }
  new_charset += ']';

  return new_charset;
}
 
void
CharSet::replace(string &str, string from, string to)
{
  size_t start_pos = str.find(from);
  if (start_pos == string::npos) return;
  str.replace(start_pos, from.size(), to);
}

string
CharSet::replace_charset_with_parens(Location loc)
{
  string regex = Util::get()->get_regex();
  string charset = regex.substr(loc.first, loc.second - loc.first + 1);
  charset[0] = '(';
  charset[charset.size() - 1] = ')';
  return charset;
}

// TEST GENERATION FUNCTIONS

vector <string>
CharSet::gen_evil_strings(string test_string, const set <char> &punct_marks)
{
  set <char> test_chars  = create_test_chars(punct_marks);
  string suffix = test_string.substr(prefix.size() + 1);
  vector <string> evil_strings;

  set <char>::iterator cs;
  for (cs = test_chars.begin(); cs != test_chars.end(); cs++) {
    string new_string = prefix;
    new_string += *cs;
    new_string += suffix;
    evil_strings.push_back(new_string);
  }
  return evil_strings;
}

set <char>
CharSet::create_test_chars(const set<char> &punct_marks)
{
  set <char> test_chars;
  bool lowercase_flag = false;
  bool uppercase_flag = false;
  bool digit_flag = false;
  bool punct_flag = false;

  bool lowercase[26];
  bool uppercase[26];
  bool digits[10];

  for (int i = 0; i < 26; i++) {

    lowercase[i] = false;
    uppercase[i] = false;
  }
  for (int i = 0; i < 10; i++) {
    digits[i] = false;
  }

  // Process individual characters first
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHARACTER_ITEM) {
      char c = it->character;
      test_chars.insert(c);

      // Set flags properly
      if (islower(c)) {
	lowercase_flag = true;
	lowercase[c - 'a'] = true;
      }
      else if (isupper(c)) {
	uppercase_flag = true;
	uppercase[c - 'A'] = true;
      }
      else if (isdigit(c)) {
	digit_flag = true;
	digits[c - '0'] = true;
      }
    }
  }

  // Process ranges second
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHAR_RANGE_ITEM) {
      char start = it->range_start;
      char end = it->range_end;

      // Lowercase range
      if (start >= 'a' && end <= 'z') {
	lowercase_flag = true;
	bool found_letter = false;
	for (char c = start; c <= end; c++) {
	  if (found_letter == false && lowercase[c - 'a'] == false) {
	    test_chars.insert(c);
	    found_letter = true;
	  }
	  lowercase[c - 'a'] = true;
	}
      }

      // Uppercase range
      else if (start >= 'A' && end <= 'Z') {
	uppercase_flag = true;
	bool found_letter = false;
	for (char c = start; c <= end; c++) {
	  if (found_letter == false && uppercase[c - 'A'] == false) {
	    test_chars.insert(c);
	    found_letter = true;
	  }
	  uppercase[c - 'A'] = true;
	}
      }

      // Digit range
      else if (start >= '0' && end <= '9') {
	digit_flag = true;
	bool found_letter = false;
	for (char c = start; c <= end; c++) {
	  if (found_letter == false && digits[c - '0'] == false) {
	    test_chars.insert(c);
	    found_letter = true;
	  }
	  digits[c - '0'] = true;
	}
      }
      else {
	stringstream s;
        // TODO: Fix this for "bad" ranges that no longer abort earlier
        s << "ERROR (bad range): Invalid range: " << start << "-" << end;
        throw EgretException(s.str());
      }
    }
  }
  
  // Process character classes third
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHAR_CLASS_ITEM) {
      switch (it->character) {
        // \w - add digit, lowercase, uppercase, and '_'
        case 'w':
	  uppercase_flag = true;
	  lowercase_flag = true;
	  digit_flag = true;
	  test_chars.insert('_');
	  break;

	// \d - add digit
	case 'd':
	  digit_flag = true;
	  break;

        // \s - add space
        case 's':
	  test_chars.insert(' ');
	  break;

        // \W, \D, \S - add digit, lowercase, uppercase, '_', space, and punctuation
	case 'W':
	case 'D':
	case 'S':
	  uppercase_flag = true;
	  lowercase_flag = true;
	  digit_flag = true;
	  punct_flag = true;
	  test_chars.insert('_');
	  test_chars.insert(' ');
	  break;

	// . (wildcard) - add digit, lowercase, space, and punctuation
        case '.':
	  uppercase_flag = true;
	  lowercase_flag = true;
	  digit_flag = true;
	  punct_flag = true;
	  test_chars.insert(' ');
	  break;

	default:
	{
	  stringstream s;
	  s << "ERROR (internal): Invalid character class in character set: "
	    << it->character;
	  throw EgretException(s.str());
	}
      }
    }
  }

  // Process complemented sets
  if (complement) {
    uppercase_flag = true;
    lowercase_flag = true;
    digit_flag = true;
    punct_flag = true;
    test_chars.insert(' ');
  }

  // If lowercase is flagged, then add one more lower case letter.
  if (lowercase_flag) {
    for (char c = 'a'; c <= 'z'; c++)  {
      if (lowercase[c - 'a'] == false)  {
	test_chars.insert(c);
	break;
      }
    }
  }

  // If uppercase is flagged, then add one more upper case letter.
  if (uppercase_flag) {
    for (char c = 'A'; c <= 'Z'; c++)  {
      if (uppercase[c - 'A'] == false)  {
	test_chars.insert(c);
	break;
      }
    }
  }

  // If digit is flagged, then add one more digit.
  if (digit_flag) {
    for (char c = '0'; c <= '9'; c++)  {
      if (digits[c - '0'] == false)  {
	test_chars.insert(c);
	break;
      }
    }
  }

  // If punctuation is flagged, then add all punctuation marks that appear
  // in the regular expression.
  if (punct_flag) {
    set<char>::iterator si;
    for (si = punct_marks.begin(); si != punct_marks.end(); si++) {
      test_chars.insert(*si);
    }

    // if no punctuation marks, add underscore
    if (punct_marks.empty()) {
      test_chars.insert('_');
    }
  }

  return test_chars;
}

// PRINT FUNCTION

void
CharSet::print()
{
  if (complement) cout << "^";

  vector <CharSetItem>::reverse_iterator it;
  for (it = items.rbegin(); it != items.rend(); it++) {
    switch (it->type) {
    case CHARACTER_ITEM:
      cout << it->character;
      break;
    case CHAR_CLASS_ITEM:
      cout << "\\" << it->character;
      break;
    case CHAR_RANGE_ITEM:
      cout << it->range_start << "-" << it->range_end;
      break;
    }
  }
}

