//===--- exitcode_warn.c - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// By default, if IWYU finds policy violations it exits with code 0.

#include "tests/driver/direct.h"

// IWYU: Indirect is...*indirect.h
struct Indirect x;

/**** IWYU_SUMMARY(0)

tests/driver/exitcode_warn.c should add these lines:
#include "tests/driver/indirect.h"

tests/driver/exitcode_warn.c should remove these lines:
- #include "tests/driver/direct.h"  // lines XX-XX

The full include-list for tests/driver/exitcode_warn.c:
#include "tests/driver/indirect.h"  // for Indirect

***** IWYU_SUMMARY */
