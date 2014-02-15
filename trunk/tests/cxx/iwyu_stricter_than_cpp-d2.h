//===--- iwyu_stricter_than_cpp-d2.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class IndirectStruct2;

// These functions are also declared in -autocast2.h, but that
// declaration isn't visible from here.
void TwiceDeclaredFunction(IndirectStruct2 ic2);
void TwiceDeclaredRefFunction(const IndirectStruct2& ic2);

void CallTwiceDeclaredFunction() {
  // We need the full type for IndirectStruct2 because the only
  // declaration that we can see, does not provide the full type for
  // us.
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TwiceDeclaredFunction(1);

  // This *should* be exactly the same, but doesn't seem to be:
  // clang leaves out the constructor-conversion AST node.
  // TODO(csilvers): IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TwiceDeclaredRefFunction(1);
}

/**** IWYU_SUMMARY

tests/cxx/iwyu_stricter_than_cpp-d2.h should add these lines:
#include "tests/cxx/iwyu_stricter_than_cpp-i2.h"

tests/cxx/iwyu_stricter_than_cpp-d2.h should remove these lines:
- class IndirectStruct2;  // lines XX-XX

The full include-list for tests/cxx/iwyu_stricter_than_cpp-d2.h:
#include "tests/cxx/iwyu_stricter_than_cpp-i2.h"  // for IndirectStruct2

***** IWYU_SUMMARY */
