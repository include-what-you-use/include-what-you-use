//===--- non_transitive_include.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/*-d*.h" \
//            -Xiwyu --transitive_includes_only \
//            -I .

// Tests that when we run in --transitive_includes_only mode, we
// do not suggest that d2.h #include d1.h, even though it needs
// a symbol from there, because d1.h is not a file that d2.h can
// see transitively.

#include "tests/cxx/non_transitive_include-d1.h"
#include "tests/cxx/non_transitive_include-d2.h"

NonTransitiveIncludeD1 nti = nti_d2;


/**** IWYU_SUMMARY

(tests/cxx/non_transitive_include.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
