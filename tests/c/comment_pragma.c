//===--- comment_pragma.c - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests IWYU pragma comment handling.

// IWYU_ARGS: -I .

// Forward-declarations are placed both before and after the included full
// definition to check that IWYU finds the latter among all the redeclarations.

struct Indirect;

#include "tests/c/direct.h"

// IWYU pragma: no_forward_declare Indirect

struct Indirect;

// IWYU: Indirect is...*indirect.h
void Fn(struct Indirect);

/**** IWYU_SUMMARY

tests/c/comment_pragma.c should add these lines:
#include "tests/c/indirect.h"

tests/c/comment_pragma.c should remove these lines:
- #include "tests/c/direct.h"  // lines XX-XX
- struct Indirect;  // lines XX-XX
- struct Indirect;  // lines XX-XX

The full include-list for tests/c/comment_pragma.c:
#include "tests/c/indirect.h"  // for Indirect

***** IWYU_SUMMARY */
