//===--- namespace_macro.cc - test input file for IWYU --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/namespace_macro-direct.h"

// IWYU_ARGS: -I . -DMYNS=myns

int main() {
  // IWYU: myns::foo is...*namespace_macro-indirect.h
  MYNS::foo();

  return 0;
}

/**** IWYU_SUMMARY

tests/cxx/namespace_macro.cc should add these lines:
#include "tests/cxx/namespace_macro-indirect.h"

tests/cxx/namespace_macro.cc should remove these lines:
- #include "tests/cxx/namespace_macro-direct.h"  // lines XX-XX

The full include-list for tests/cxx/namespace_macro.cc:
#include "tests/cxx/namespace_macro-indirect.h"  // for foo

***** IWYU_SUMMARY */
