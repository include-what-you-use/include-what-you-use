//===--- libbuiltins.c - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Library builtins are, like normal builtins, compiled down to an intrinsic,
// but a header still needs to be included for the program to be valid. The math
// library (pow, round, etc) is a typical example.

// IWYU_ARGS: -I .

#include "tests/c/libbuiltins-direct.h"

float Kapow(float x) {
  // IWYU: pow is...*math.h
  return pow(x, 2.0F);
}

/**** IWYU_SUMMARY

tests/c/libbuiltins.c should add these lines:
#include <math.h>

tests/c/libbuiltins.c should remove these lines:
- #include "tests/c/libbuiltins-direct.h"  // lines XX-XX

The full include-list for tests/c/libbuiltins.c:
#include <math.h>  // for pow

***** IWYU_SUMMARY */
