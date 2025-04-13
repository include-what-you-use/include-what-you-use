//===--- var_cleanup_attr.c - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// The cleanup attribute runs a function when the variable goes out of scope.
// This test makes sure that we track that function.

#include "tests/c/var_cleanup_attr-d1.h"

void cleanup(void) {
  // IWYU: freep is...*var_cleanup_attr-i1.h
  __attribute__((__cleanup__(freep))) char* x = 0;
}

/**** IWYU_SUMMARY

tests/c/var_cleanup_attr.c should add these lines:
#include "tests/c/var_cleanup_attr-i1.h"

tests/c/var_cleanup_attr.c should remove these lines:
- #include "tests/c/var_cleanup_attr-d1.h"  // lines XX-XX

The full include-list for tests/c/var_cleanup_attr.c:
#include "tests/c/var_cleanup_attr-i1.h"  // for freep

***** IWYU_SUMMARY */
