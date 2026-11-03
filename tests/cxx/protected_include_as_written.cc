//===--- protected_include_as_written.cc - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . \
//            -Xiwyu --check_also=tests/cxx/protected_include_as_written-d1.h

// Tests that the #includes protected by an export pragma or by an
// always_keep pragma are kept as they are written.  -d1.h #includes its
// headers relative to its own directory, while IWYU would spell them
// "tests/cxx/...": it must not ask -d1.h to add them again.

#include "tests/cxx/protected_include_as_written-d1.h"

ProtectedExported exported;

/**** IWYU_SUMMARY

(tests/cxx/protected_include_as_written.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
