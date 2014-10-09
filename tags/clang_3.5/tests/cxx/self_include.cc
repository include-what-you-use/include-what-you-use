//===--- self_include.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we don't crash when a file includes itself.

#ifndef SEEN
#define SEEN

#include "tests/cxx/self_include.cc"

#endif


/**** IWYU_SUMMARY

(tests/cxx/self_include.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
