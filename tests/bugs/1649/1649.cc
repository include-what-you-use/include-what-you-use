//===--- 1649.cc - test input file for iwyu -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include <map>

void f() {
  std::map<const char*, int> m;

  const std::map<const char*, int>::const_iterator entry = m.find(nullptr);
  (void)entry->second;
}

/**** IWYU_SUMMARY

(tests/bugs/1649/1649.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
