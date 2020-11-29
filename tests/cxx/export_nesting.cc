//===--- export_nesting.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/export_nesting.h"

Nested_Enum x;

/**** IWYU_SUMMARY

(tests/cxx/export_nesting.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
