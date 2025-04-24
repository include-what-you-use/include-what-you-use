//===--- 1541.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include <list>

struct S;

class C {
  // IWYU requires the complete type for S, while compilers do not.
  std::list<S> l_s;
};

/**** IWYU_SUMMARY

(tests/bugs/1541/1541.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
