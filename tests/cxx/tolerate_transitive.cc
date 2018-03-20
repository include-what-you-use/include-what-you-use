//===--- tolerate_transitive.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test case proves that when only -direct.h is included and symbols from
// both -direct.h and -indirect.h are used, -indirect.h is not suggested. This
// is correct under --tolerate_transitive, because -direct.h re-exports
// -indirect.h.

// Requires -Xiwyu --tolerate_transitive.

#include "tests/cxx/tolerate_transitive-direct.h"

// A diagnostic for IndirectType will be produced, but the use will be
// reattributed later in IWYU analysis.

// IWYU: IndirectType is...*tolerate_transitive-indirect.h
IndirectType full_use;

void f() {
  DirectFunction(full_use);
}

/**** IWYU_SUMMARY

(tests/cxx/tolerate_transitive.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
