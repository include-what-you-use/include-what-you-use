//===--- template_function_arg.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the weird (to me) function-proto template arguments.
// These are used by gmock, and we want to make sure we read
// through the function types.

#include "tests/direct.h"

// IWYU: IndirectClass needs a declaration
char Foo(IndirectClass ic);

template <typename F> struct Function;
template <typename R, typename A1> struct Function<R(A1)> {
  R result;
  char Argument1[sizeof(A1)];
};

int main() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  Function<char(IndirectClass&)> f;
  (void)f;
}

/**** IWYU_SUMMARY

tests/template_function_arg.cc should add these lines:
#include "tests/indirect.h"

tests/template_function_arg.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/template_function_arg.cc:
#include "tests/indirect.h"  // for IndirectClass
template <typename F> struct Function;  // lines XX-XX

***** IWYU_SUMMARY */
