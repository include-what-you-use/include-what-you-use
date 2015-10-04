//===--- using_unused.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we import a name via a using statement, that IWYU still tells
// us we need to include the original thing we're referencing.

#include <vector>
using std::vector;

/**** IWYU_SUMMARY

(tests/cxx/using_unused.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
