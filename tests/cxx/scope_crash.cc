//===--- scope_crash.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a weak testcase, but it's the smallest example we've found to
// reproduce an IWYU crash where Sema::TUScope was unexpectedly null.
//
// For some reason libstdc++9's std::map with a value with explicit default
// constructor triggers some path in Sema's constructor lookup that needs a
// non-null TUScope.
//
// Clang or libstdc++ might change so that this can no longer trigger the
// original bug, or so that the bug manifests some other way. But testers can't
// be choosers.

#include <string>
#include <map>

struct A {};

bool operator<(const A& lhs, const A& rhs) {
  return false;
}

struct B {
  // Used to crash with libstdc++ 9, worked without 'explicit'
  explicit B() = default;
  std::string data;
};

void foo(const A& a) {
  std::map<A, B> m;
  m.erase(a);
}

/**** IWYU_SUMMARY

(tests/cxx/scope_crash.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
