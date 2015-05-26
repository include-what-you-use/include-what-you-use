//===--- no_h_includes_cc.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we never suggest that a .h file #include a .cc file,
// even in cases where one .cc file #includes another one.

#include "tests/cxx/no_h_includes_cc-inc.c"
#include "tests/cxx/no_h_includes_cc.h"

#ifndef CC_INC_HAS_INT
const int x = kCcIncInt + 2;
#endif

#define INCLUDED_FROM_MAIN 1
#include "tests/cxx/no_h_includes_cc-inc2.c"


/**** IWYU_SUMMARY

(tests/cxx/no_h_includes_cc.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
