//===--- tolerate_transitive_iwyu.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test case proves that when both -direct.h and -indirect.h are included
// and only symbols from -indirect are used, we remove -direct even under
// --tolerate_transitive. This is IWYU, after all, and -direct.h is unused.

// Requires -Xiwyu --tolerate_transitive.

#include "tests/cxx/tolerate_transitive-direct.h"
#include "tests/cxx/tolerate_transitive-indirect.h"

IndirectType full_use;

/**** IWYU_SUMMARY

tests/cxx/tolerate_transitive_iwyu.cc should add these lines:

tests/cxx/tolerate_transitive_iwyu.cc should remove these lines:
- #include "tests/cxx/tolerate_transitive-direct.h"  // lines XX-XX

The full include-list for tests/cxx/tolerate_transitive_iwyu.cc:
#include "tests/cxx/tolerate_transitive-indirect.h"  // for IndirectType

***** IWYU_SUMMARY */
