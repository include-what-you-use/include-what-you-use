//===--- block_template_internals.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/block_template_internals.imp -I .

// Test that IWYU does not report template implementation details, especially
// for templates from external libraries.

#include "tests/cxx/block_template_internals-d1.h"

// IWYU should not report InternalFn here. It is assumed to be provided
// by the public header for TplFn.
int x = TplFn(1);

/**** IWYU_SUMMARY

(tests/cxx/block_template_internals.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
