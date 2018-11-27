//===--- export_nesting.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_NESTING_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_NESTING_H_

// We export a header which already re-exports things to verify that nested
// exports are acceptable.

#include "tests/cxx/export_nesting-d1.h" // IWYU pragma: export

Nested_Enum f;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_NESTING_H_

/**** IWYU_SUMMARY

(tests/cxx/export_nesting.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
