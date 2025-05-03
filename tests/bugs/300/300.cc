//===--- 300.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -stdlib=libc++
// IWYU_XFAIL

#include <string>
#include <unordered_set>

static const std::unordered_set<std::string> us;

bool f() {
  return (us.find("") != us.end());
}

/**** IWYU_SUMMARY

(tests/bugs/300/300.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
