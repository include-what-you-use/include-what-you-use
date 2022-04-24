//===--- iwyu_regex.cc - iwyu regex implementation ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_regex.h"

#include <regex>

namespace include_what_you_use {

bool RegexMatch(const std::string& str, const std::string& pattern) {
  std::regex r(pattern);
  return std::regex_match(str, r);
}

}  // namespace include_what_you_use
