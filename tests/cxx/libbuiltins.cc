//===--- libbuiltins.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Library builtins are, like normal builtins, compiled down to an intrinsic,
// but a header still needs to be included for the program to be valid. The math
// library (std::pow, std::round, etc) is a typical example.

// IWYU_ARGS: -I .

#include "tests/cxx/libbuiltins-direct.h"

float kapow(float x) {
  // IWYU: std::pow(float, float) is...*cmath
  return std::pow(x, 2.0F);
}

/**** IWYU_SUMMARY

tests/cxx/libbuiltins.cc should add these lines:
#include <cmath>

tests/cxx/libbuiltins.cc should remove these lines:
- #include "tests/cxx/libbuiltins-direct.h"  // lines XX-XX

The full include-list for tests/cxx/libbuiltins.cc:
#include <cmath>  // for pow

***** IWYU_SUMMARY */
