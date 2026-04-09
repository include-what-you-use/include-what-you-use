//===--- unnamed.cc - iwyu test -------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/unnamed-direct.h"

// IWYU_ARGS: -I .

int main() {
  // Notably, we do NOT want -i1.h because the type in there has no name.
  // IWYU: deduced_var is...*unnamed-i2.h
  return deduced_var.i;
}

/**** IWYU_SUMMARY

tests/cxx/unnamed.cc should add these lines:
#include "tests/cxx/unnamed-i2.h"

tests/cxx/unnamed.cc should remove these lines:
- #include "tests/cxx/unnamed-direct.h"  // lines XX-XX

The full include-list for tests/cxx/unnamed.cc:
#include "tests/cxx/unnamed-i2.h"  // for deduced_var

***** IWYU_SUMMARY */
