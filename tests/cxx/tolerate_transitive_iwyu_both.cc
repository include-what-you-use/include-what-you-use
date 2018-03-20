//===--- tolerate_transitive_iwyu_both.cc - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test case proves that when both -direct.h and -indirect.h are included
// and symbols from both are used, both includes are retained even under
// --tolerate_transitive. The rationale is that if the file already plays by
// IWYU rules, we don't need to make it worse.

// Requires -Xiwyu --tolerate_transitive.

#include "tests/cxx/tolerate_transitive-direct.h"
#include "tests/cxx/tolerate_transitive-indirect.h"

IndirectType full_use;

void f() {
  DirectFunction(full_use);
}

/**** IWYU_SUMMARY

(tests/cxx/tolerate_transitive_iwyu_both.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
