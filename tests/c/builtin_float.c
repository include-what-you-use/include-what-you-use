//===--- builtin_float.c - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The float macros live in the compiler builtin header <float.h>. This test
// case proves we have internal mappings to sort out any private/public
// challenges. Uses C23, since NAN is defined to live in <float.h> from that
// point (<math.h>, not a compiler built-in, before that).
//
// Note that <float.h> lives in the compiler resource dir, but may delegate to a
// system <float.h> using #include_next, so this test could fail on more exotic
// platforms. If it does, let's remove it. We typically don't test include
// mappings exactly because they aren't very portable.

#include "tests/c/builtin_float-d1.h"

// IWYU_ARGS: -std=c23 -I .

static double epsilon(void) {
  // IWYU: DBL_EPSILON is...*float.h
  return DBL_EPSILON;
}

static int nonumber(void) {
  // IWYU: NAN is...*float.h
  return NAN;
}

/**** IWYU_SUMMARY

tests/c/builtin_float.c should add these lines:
#include <float.h>

tests/c/builtin_float.c should remove these lines:
- #include "tests/c/builtin_float-d1.h"  // lines XX-XX

The full include-list for tests/c/builtin_float.c:
#include <float.h>  // for DBL_EPSILON, NAN

***** IWYU_SUMMARY */
