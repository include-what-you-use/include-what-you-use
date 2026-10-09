//===--- visibility_conflict.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/visibility_conflict.imp \
//            -I .

// Test that IWYU does not crash when the same header gets conflicting
// visibility from different mapping entries.  The mapping file marks
// visibility_conflict-d1.h as both private (from-side) and public
// (to-side).  Before the fix, this triggered a CHECK_ failure in
// MarkVisibility.  With the fix, IWYU warns and the last mapping wins.

// IWYU~: header "tests/cxx/visibility_conflict-d1.h" has conflicting visibilities \("public" vs "private"\); using "private"
// IWYU~: header "tests/cxx/visibility_conflict-d1.h" has conflicting visibilities \("private" vs "public"\); using "public"

#include "tests/cxx/visibility_conflict-d1.h"

IndirectClass ic;

/**** IWYU_SUMMARY

(tests/cxx/visibility_conflict.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
