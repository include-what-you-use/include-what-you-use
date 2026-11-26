//===--- protected_include_as_written-d1.h - test input file for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "protected_include_as_written-i1.h"  // IWYU pragma: export
#include "protected_include_as_written-i2.h"

/**** IWYU_SUMMARY

(tests/cxx/protected_include_as_written-d1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
