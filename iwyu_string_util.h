//===--- iwyu_string_util.h - string utilities for include-what-you-use ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// String utilities for the IWYU checker.
//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_STRING_UTIL_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_STRING_UTIL_H_

#include <ctype.h>
#include <string>
#include <vector>
#include "port.h"

namespace include_what_you_use {

using std::string;
using std::vector;


// Returns true if str starts with prefix.
inline bool StartsWith(const string& str, const string& prefix) {
  return str.substr(0, prefix.length()) == prefix;
}

// Returns true if str ends with suffix.
inline bool EndsWith(const string& str, const string& suffix) {
  if (suffix.length() > str.length())
    return false;
  return str.substr(str.length() - suffix.length()) == suffix;
}

// If *str starts with prefix, removes the prefix and returns true.
inline bool StripLeft(string* str, const string& prefix) {
  if (StartsWith(*str, prefix)) {
    *str = str->substr(prefix.length());
    return true;
  }

  return false;
}

// If *str ends with suffix, removes the suffix and returns true.
inline bool StripRight(string* str, const string& suffix) {
  if (str->length() >= suffix.length() &&
      str->substr(str->length() - suffix.length()) == suffix) {
    *str = str->substr(0, str->length() - suffix.length());
    return true;
  }

  return false;
}

// Finds the first occurrence of substr in *str and removes from *str
// everything before the occurrence and the occurrence itself.  For
// example, string s = "What a hat!"; StripPast(&s, "hat"); will make s
// " a hat!".
inline bool StripPast(string* str, const string& substr) {
  const size_t pos = str->find(substr);
  if (pos == string::npos)
    return false;

  *str = str->substr(pos + substr.length());
  return true;
}

// Removes leading whitespace.
inline void StripWhiteSpaceLeft(string* str) {
  for (string::size_type i = 0; i < str->size(); ++i) {
    if (!isspace((*str)[i])) {
      *str = str->substr(i);
      return;
    }
  }
  // Everything is whitespace. Return with an empty string.
  str->clear();
}

// Removes trailing whitespace.
inline void StripWhiteSpaceRight(string* str) {
  for (string::size_type end_of_string = str->size();
       end_of_string > 0; --end_of_string) {
    if (!isspace((*str)[end_of_string - 1])) {
      *str = str->substr(0, end_of_string);
      return;
    }
  }
  // Everything is whitespace. Return with an empty string.
  str->clear();
}

// Removes both leading and trailing whitespace.
inline void StripWhiteSpace(string* str) {
  StripWhiteSpaceLeft(str);
  StripWhiteSpaceRight(str);
}

// This is the same as split() in Python.  If max_segs is 0, there's
// no limit on the number of the generated segments.
inline vector<string> Split(
    string str, const string& divider, size_t max_segs) {
  CHECK_(!divider.empty());
  vector<string> retval;
  size_t pos;
  // If max_segs is 0, the first part of the condition will always be true.
  while (retval.size() + 1 != max_segs &&
         (pos = str.find(divider)) != string::npos) {
    retval.push_back(str.substr(0, pos));
    str = str.substr(pos + divider.length());
  }
  retval.push_back(str);
  return retval;
}

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_STRING_UTIL_H_
