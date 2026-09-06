//===--- check_also_sibling.inc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The unused #include below gives IWYU something to report for this file.

#include "tests/cxx/indirect.h"

inline void SiblingFn() {
}

/**** IWYU_SUMMARY

tests/cxx/check_also_sibling.inc should add these lines:

tests/cxx/check_also_sibling.inc should remove these lines:
- #include "tests/cxx/indirect.h"  // lines XX-XX

The full include-list for tests/cxx/check_also_sibling.inc:

***** IWYU_SUMMARY */
