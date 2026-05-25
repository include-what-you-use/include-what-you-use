//===--- funcptrs.c - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c17

// Tests handling pointers to functions. The C language standard has been nailed
// down because the old K&R non-prototype function types have been removed
// in C23.

#include "tests/c/direct.h"

// Parameter and return types don't need to be complete at function pointer
// declarations.
struct Indirect (*no_prototype)();
struct Indirect (*prototype1)(void);
// A forward-declaration is desirable due to constrained visibility of names
// from the function prototype scope.
// IWYU: Indirect needs a declaration
void (*prototype2)(struct Indirect);

/**** IWYU_SUMMARY

tests/c/funcptrs.c should add these lines:
struct Indirect;

tests/c/funcptrs.c should remove these lines:
- #include "tests/c/direct.h"  // lines XX-XX

The full include-list for tests/c/funcptrs.c:
struct Indirect;

***** IWYU_SUMMARY */
