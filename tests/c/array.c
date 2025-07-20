//===--- array.c - test input file for iwyu -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/c/direct.h"

// IWYU: Indirect is...*indirect.h
// IWYU: Indirect needs a declaration
void array(const struct Indirect array[]);

// IWYU: Indirect is...*indirect.h
typedef struct Indirect (*ArrayPtr)[5];

// IWYU: Indirect is...*indirect.h
extern const struct Indirect extern_array[];

/**** IWYU_SUMMARY

tests/c/array.c should add these lines:
#include "tests/c/indirect.h"

tests/c/array.c should remove these lines:
- #include "tests/c/direct.h"  // lines XX-XX

The full include-list for tests/c/array.c:
#include "tests/c/indirect.h"  // for Indirect

***** IWYU_SUMMARY */
