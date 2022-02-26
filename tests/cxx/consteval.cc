//===--- consteval.cc - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++20

// These tests are not particularly interesting in themselves, but they cover an
// upstream bug in Clang (https://github.com/llvm/llvm-project/issues/53044) for
// which we have a workaround.

#include "tests/cxx/direct.h"

struct X {
  // IWYU: IndirectClass needs a declaration
  consteval X(const IndirectClass& v) {
  }

  // IWYU: IndirectClass needs a declaration
  consteval operator IndirectClass*() const {
    return nullptr;
  }
};

void t() {
  // Pass value through Consteval conversion constructor.
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass a;
  X x = a;

  // Try an implicit consteval user-defined conversion too.
  // IWYU: IndirectClass needs a declaration
  IndirectClass* b = x;
}

/**** IWYU_SUMMARY

tests/cxx/consteval.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/consteval.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/consteval.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
