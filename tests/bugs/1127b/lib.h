//===--- lib.h - iwyu test ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <set>
#include <string>

std::set<std::string> make_set() {
  return std::set<std::string>{"one", "two", "three"};
}
