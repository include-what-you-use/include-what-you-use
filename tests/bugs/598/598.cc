//===--- 598.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "container.h"

int foo(Container& c) {
    c.push_back(5);
    auto it = c.begin();
    return *it;
}

/**** IWYU_SUMMARY

(tests/bugs/598/598.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
