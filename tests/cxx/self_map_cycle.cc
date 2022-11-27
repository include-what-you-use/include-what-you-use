//===--- self_map_cycle.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Test that self-includes do not create a mapping cycle, which would later be
// detected and abort with assertion failure (as described in issue #424).

// Note that "" #include was required to trigger the assertion here.
#include "tests/cxx/self_map_cycle-d1.h"

X x;

/**** IWYU_SUMMARY

(tests/cxx/self_map_cycle.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
