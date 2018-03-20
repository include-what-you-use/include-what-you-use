//===--- tolerate_transitive_iwyu_none.cc - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test case proves that when both -direct.h and -indirect.h are included
// but no symbols are used, --tolerate_transitive does not ruin the normal IWYU
// behavior.

// Requires -Xiwyu --tolerate_transitive.

#include "tests/cxx/tolerate_transitive-direct.h"
#include "tests/cxx/tolerate_transitive-indirect.h"

/**** IWYU_SUMMARY

tests/cxx/tolerate_transitive_iwyu_none.cc should add these lines:

tests/cxx/tolerate_transitive_iwyu_none.cc should remove these lines:
- #include "tests/cxx/tolerate_transitive-direct.h"  // lines XX-XX
- #include "tests/cxx/tolerate_transitive-indirect.h"  // lines XX-XX

The full include-list for tests/cxx/tolerate_transitive_iwyu_none.cc:

***** IWYU_SUMMARY */
