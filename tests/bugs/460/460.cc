//===--- 460.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "foo.h"

int main() {
  bar::MyVec c;
  bar::MyVec::iterator cit = c.begin();
  while (cit != c.end()) {
    cit++;
  }

  bar2::MyList d;
  bar2::MyList::iterator dit = d.begin();
  while (dit != d.end()) {
    dit++;
  }
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/460/460.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
